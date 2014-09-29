#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <syslog.h>
#include <sys/time.h>
#include <unistd.h>

#include <native/pipe.h>
#include "libwebsockets.h"

#include "../include/telemetry.h"


// Максимальный размер блока данных, передаваемый между потоками
#define MAX_TRANSFER_BLOCK 256

ar_remote_shmems_t remote_shmems;


// Массив указателей на блоки памяти, готовые к передаче
// Первое два байта в блоке памяти - длина последующего блока, предназначенного для передачи (длина лежащего там bson объекта)
void** ar_bufs = NULL;
size_t ar_bufs_len=0;

RT_TASK task_read_shmem;
int priority_task_read_shmem = 50;

/**
 * @brief
 * \~english input error queue
 * \~russian Входная очередь сообщений-ошибок(предупреждений) от модулей
 */
RT_QUEUE err_queue;


/**
 * @brief \~russian Буфер для приема блока данных из очереди с ошибками
 */
char err_buf[MAX_ERR_SIZE];

int pipe_fd;

enum demo_protocols {
    /* always first */
    PROTOCOL_HTTP = 0,

    PROTOCOL_TELEMETRY
};

static struct option options[] = {
{ "help",	no_argument,		NULL, 'h' },
{ "debug",	required_argument,	NULL, 'd' },
{ "port",	required_argument,	NULL, 'p' },
{ "ssl",	no_argument,		NULL, 's' },
{ "allow-non-ssl",	no_argument,		NULL, 'a' },
{ "interface",  required_argument,	NULL, 'i' },
{ "closetest",  no_argument,		NULL, 'c' },
{ "libev",  no_argument,		NULL, 'e' },
#ifndef LWS_NO_DAEMONIZE
{ "daemonize", 	no_argument,		NULL, 'D' },
#endif
{ NULL, 0, 0, 0 }
};


static volatile int force_exit = 0;
static struct libwebsocket_context *context;

void debug_print_bson(char* where, bson_t* bson) {
    fprintf(stdout, "%s\n", where);
    char* str = bson_as_json(bson, NULL);
    fprintf(stdout, "%s\n", str);
    bson_free(str);
    fprintf(stdout, "\n");
}

/* this protocol server (always the first one) just knows how to do HTTP */
static int callback_http(struct libwebsocket_context *context, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
{
    if(reason==LWS_CALLBACK_HTTP && len < 1)
    {
        libwebsockets_return_http_status(context, wsi, HTTP_STATUS_BAD_REQUEST, NULL);
        return -1;
    }
    return 0;
}


struct per_session_data__telemetry {
    int number;
};

typedef struct{
    bson_t* bson;
    void* buf;
} buf_and_bson_t;



bson_t* send_command(const char* instance_name, void *cmd_data, size_t cmd_len)
{
    char task_name[XNOBJECT_NAME_LEN] = "";
    strcat(task_name, instance_name);
    strcat(task_name, SUFFIX_TASK);

    RT_TASK recipient_task;
    int err = rt_task_bind(&recipient_task, task_name, TM_NONBLOCK);
    if(err!=0)
    {
        fprintf(stderr, "send_command, ERROR \"%i\" rt_task_bind, task instance:\"%s\"\n", err, task_name);
        return BCON_NEW ("status", BCON_UTF8 ("error"), "instance", BCON_UTF8(instance_name), "message", BCON_UTF8("rt_task_bind"));
    }

    char request_buf[MAX_TRANSFER_BLOCK];
    // В первые два байта сохраняем длину блока
    *((unsigned short*) request_buf) = cmd_len;
    // в буфер (со смещением в два байта) копируем блок данных
    memcpy(request_buf + sizeof(unsigned short), cmd_data, cmd_len);

    RT_TASK_MCB request_block;
    request_block.data = request_buf;
    request_block.size = cmd_len + sizeof(unsigned short);
    request_block.opcode = cmd_command;

    RT_TASK_MCB response_block;
    char response_buf[MAX_TRANSFER_BLOCK];
    response_block.data = response_buf;
    response_block.size = MAX_TRANSFER_BLOCK;

//fprintf(stderr, "send_command, BEFORE SEND, task instance:\"%s\"\n", task_name);
    ssize_t received = rt_task_send(&recipient_task, &request_block, &response_block, TM_INFINITE);
//fprintf(stderr, "send_command, AFTER SEND, task instance:\"%s\", received=%i\n", task_name, received);
    rt_task_unbind(&recipient_task);
    if(received<=0)
    {
        bson_t* resp = bson_new();
        bson_append_utf8(resp, "type", -1, "status", -1);
        bson_append_utf8(resp, "value", -1, "ERROR in rt_task_send", -1);
        bson_append_utf8(resp, "instance", -1, instance_name, -1);
        return resp;
    }
//fprintf(stderr, "send_command, ERROR \"%i\" rt_task_send, instance:\"%s\"\n", received, instance_name);
    return bson_new_from_data (response_block.data, received);
}


void* buf_err = NULL;
buf_and_bson_t* ar_bson2send = NULL;
int len_bson2send = 0;

static int callback_telemetry(struct libwebsocket_context *context, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
{
    int n, m, i, k;
    unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 + LWS_SEND_BUFFER_POST_PADDING];
    unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
    struct per_session_data__telemetry *pss = (struct per_session_data__telemetry *)user;

    ssize_t read_len;

    bson_t* bson_request;
    const char* module_instance_name;
    const char* module_out_name;
    const char* cmd_name;
    int res_read;

    switch (reason)
    {

    case LWS_CALLBACK_ESTABLISHED:
        lwsl_info("callback_telemetry: " "LWS_CALLBACK_ESTABLISHED\n");
        pss->number = 0;
        break;

    case LWS_CALLBACK_SERVER_WRITEABLE:

        // Вычитываем мессагу из очереди сообщений с ошибками модулей
        res_read = rt_queue_read(&err_queue, err_buf, MAX_ERR_SIZE, TM_NONBLOCK);
        if (res_read > 0)
        {
            //buf_err = calloc(1, res_read);

            buf_err = (unsigned char*) malloc(LWS_SEND_BUFFER_PRE_PADDING + res_read + LWS_SEND_BUFFER_POST_PADDING);

            memcpy(&buf_err[LWS_SEND_BUFFER_PRE_PADDING], err_buf, res_read);

            // и отправляем в веб-сокет
            libwebsocket_write(wsi, &buf_err[LWS_SEND_BUFFER_PRE_PADDING], res_read, LWS_WRITE_BINARY);
            //TODO: Разобраться с этим сраным libwebsocket_write. Сам он освобождает память переданного буфера или нет.
            // Если нет, то когда его можно освобождать? Ибо передача данных из буфера в вебсокет похоже происходит асинхронно.
             free(buf_err);
            break;
        }


        // Вычистим из памяти ранеее отправленные данные
        for(i=0; i < len_bson2send; i++)
        {
            buf_and_bson_t buf_and_bson = ar_bson2send[i];
            if(buf_and_bson.bson)
            {
                bson_destroy(buf_and_bson.bson);
                buf_and_bson.bson = NULL;
            }
            if(buf_and_bson.buf)
            {
                free(buf_and_bson.buf);
                buf_and_bson.buf = NULL;
            }
        }

        // Выделим память под массив
        if(len_bson2send!=remote_shmems.remote_shmems_len)
        {
            len_bson2send = remote_shmems.remote_shmems_len;
            int ar_count_bytes = len_bson2send * sizeof(buf_and_bson_t);
            if(ar_bson2send){
                ar_bson2send = realloc(ar_bson2send, ar_count_bytes);
                memset(ar_bson2send, 0, ar_count_bytes);
            }
            else
                ar_bson2send = calloc(1, ar_count_bytes);
        }

        // Вычитываем данные из разделяемой памяти и напихиваем их в очередь (ведущую в не риалтаймовому потоку)
        for(k=0; k < remote_shmems.remote_shmems_len; k++)
        {
            shmem_in_set_t* remote_shmem = remote_shmems.remote_shmems[k];
            if(!remote_shmem->f_shmem_connected)
                continue;

            buf_and_bson_t buf_and_bson = ar_bson2send[k];
            buf_and_bson.buf = malloc(500);
            unsigned short retlen = 0;
            read_shmem(remote_shmem, buf_and_bson.buf, &retlen);
            if (retlen < 1)
                continue;

            buf_and_bson.bson = bson_new_from_data(buf_and_bson.buf, retlen);
            bson_append_utf8 (buf_and_bson.bson, "_from", -1, remote_shmem->name_instance, -1);

            unsigned char * bson_data = (unsigned char *)bson_get_data(buf_and_bson.bson);
            m = libwebsocket_write(wsi, bson_data, buf_and_bson.bson->len, LWS_WRITE_BINARY);
            if (m < buf_and_bson.bson->len) {
                //lwsl_err("ERROR %d writing to di socket\n", n);
            }
        }

        break;


    case LWS_CALLBACK_RECEIVE:
        bson_request = bson_new_from_data (in, len);
        //debug_print_bson("received", bson_request);

        // Get Instance Name
        bson_iter_t iter_instance_name;
        if (!bson_iter_init_find(&iter_instance_name, bson_request, "instance")) {
            fprintf(stderr, "Not found property \"instance\" in module_instance");
            return -1;
        }
        if (!BSON_ITER_HOLDS_UTF8(&iter_instance_name)) {
            fprintf(stderr, "Property \"instance\" in module_instance not UTF8 type");
            return -1;
        }
        module_instance_name = bson_iter_utf8(&iter_instance_name, NULL);

        // Get Command Name
        bson_iter_t iter_cmd;
        if (!bson_iter_init_find(&iter_cmd, bson_request, "cmd")) {
            fprintf(stderr, "Not found property \"cmd\" in module_out");
            return -1;
        }
        if (!BSON_ITER_HOLDS_UTF8(&iter_cmd)) {
            fprintf(stderr, "Property \"cmd\" in module_out not UTF8 type");
            return -1;
        }
        cmd_name = bson_iter_utf8(&iter_cmd, NULL);

        //fprintf(stderr, "module_instance_name: %s\tmodule_out_name: %s\n", module_instance_name, module_out_name);

        if(strcmp(cmd_name, "subscribe")==0 || strcmp(cmd_name, "unsubscribe")==0)
        {
            /*
             * JSON
             * {
             *      cmd: 'subscribe'/'unsubscribe',
             *      instance: instanceName,
             *      out: outputName
             * }
             */

            // Get Out Name
            bson_iter_t iter_out_name;
            if (!bson_iter_init_find(&iter_out_name, bson_request, "out")) {
                fprintf(stderr, "Not found property \"out\" in module_out");
                return -1;
            }
            if (!BSON_ITER_HOLDS_UTF8(&iter_out_name)) {
                fprintf(stderr, "Property \"out\" in module_out not UTF8 type");
                return -1;
            }
            module_out_name = bson_iter_utf8(&iter_out_name, NULL);

            if(strcmp(cmd_name, "subscribe")==0)
            {
                register_remote_shmem(&remote_shmems, module_instance_name, module_out_name);
            }
            else if(strcmp(cmd_name, "unsubscribe")==0)
            {
                unregister_remote_shmem(&remote_shmems, module_instance_name, module_out_name);
            }
        }
        else if(strcmp(cmd_name, "command")==0)
        {
            /*
             * JSON
             * {
             *      cmd: 'command',         // Тип операции 'command' (выполняемой сервисом телеметрии)
             *                              // подразумевает передачу команды инстансу
             *      name: COMMAND_NAME      // Имя команды передаваемой в инстанс
             *      params: []
             *      instance: instanceName
             * }
             */

//fprintf(stderr, "callback_telemetry BEFORE send_command\n");
            bson_t* resp_bson = send_command(module_instance_name, in, len);
//fprintf(stderr, "callback_telemetry AFTER send_command\n");
//debug_print_bson("resp_bson", resp_bson);
            if(resp_bson)
            {
                // ответим в вебсокет
                libwebsocket_write(wsi, (unsigned char *)bson_get_data(resp_bson), resp_bson->len, LWS_WRITE_BINARY);
                bson_destroy(resp_bson);
            }

        }
        else if(strcmp(cmd_name, "getParams")==0)
        {
            /*
             * JSON
             * {
             *      cmd: getParams,
             *      instance: instanceName
             * }
             */
        }
        else if(strcmp(cmd_name, "setParams")==0)
        {
            /*
             * JSON
             * {
             *      cmd: getParams,
             *      instance: instanceName
             *      params: []
             * }
             */
        }
        else
        {
            fprintf(stderr, "Function: callback_telemetry, unknown command \"%s\" module_instance_name: %s\n", cmd_name, module_instance_name);
        }

        bson_destroy(bson_request);
        break;
    }
    return 0;
}



/* list of supported protocols and callbacks */
static struct libwebsocket_protocols protocols[] = {
    /* first protocol must always be HTTP handler */
{
    "http-only",
    callback_http,
    0,
    0,
},
{
    "telemetry-protocol",
    callback_telemetry,
    sizeof(struct per_session_data__telemetry),
    100,
},
{ NULL, NULL, 0, 0 } /* terminator */
};



void run_task_read_shmem (void *module)
{
    int n = 0;
    int count=0;
    while (n >= 0 && !force_exit)
    {
        // Вызов данной функции проверит появилась ли возможность записать очередную партию данных в сокет,
        // и если да, то вызовет калбэк (для указанного протокола) с причиной  LWS_CALLBACK_SERVER_WRITEABLE
        libwebsocket_callback_on_writable_all_protocol(&protocols[PROTOCOL_TELEMETRY]);

        //fprintf(stdout, "%i %i libwebsocket_callback_on_writable_all_protocol\n", count++, n);

        // Если не все конекты с расщиряемой памяти установлены, то попытаемся это сделать
        if(!remote_shmems.f_connected_in_links)
        {
            char* local_instance_name = "telemetry";
            connect_in_links(&remote_shmems, local_instance_name);
        }

        // Функция проверят, есть ли какие то данные прием из сокета. Если есть то вызывает калбэк с указанной причиной LWS_CALLBACK_RECEIVE
        // Потом спит указанное количество миллисекунд
        n = libwebsocket_service(context, 0);

        rt_task_sleep(rt_timer_ns2ticks(200000000));
    }

}



int init_rt_task()
{
    int err = rt_task_create(&task_read_shmem, "telemetry_mt", TASK_STKSZ, priority_task_read_shmem, TASK_MODE);
    if (err != 0)
    {
        fprintf(stdout, "Error create task_read_shmem \n");
        return err;
    }

    err = rt_task_start(&task_read_shmem, &run_task_read_shmem, NULL);
    if (err != 0){
        fprintf(stderr, "Error start main task\n");
        return err;
    }


    err = rt_queue_create(&err_queue, NAME_ERR_QUEUE, MAX_ERR_SIZE, 10, Q_FIFO);
    if (err != 0)
    {
        fprintf(stdout, "Error create queue \"%s\"\n", NAME_ERR_QUEUE);
        return err;
    }

    return 0;
}



int main(int argc, char **argv)
{
    mlockall(MCL_CURRENT|MCL_FUTURE);

    int n = 0;
    int use_ssl = 0;
    int opts = 0;
    char interface_name[128] = "";
    const char *iface = NULL;
    unsigned int oldus = 0;
    struct lws_context_creation_info info;

    int debug_level = 7;
#ifndef LWS_NO_DAEMONIZE
    int daemonize = 0;
#endif

    memset(&info, 0, sizeof info);
    info.port = 7681;

    while (n >= 0) {
        n = getopt_long(argc, argv, "eci:hsap:d:Dr:", options, NULL);
        if (n < 0)
            continue;
        switch (n) {
        case 'e':
            opts |= LWS_SERVER_OPTION_LIBEV;
            break;
#ifndef LWS_NO_DAEMONIZE
        case 'D':
            daemonize = 1;
            break;
#endif
        case 'd':
            debug_level = atoi(optarg);
            break;
        case 's':
            use_ssl = 1;
            break;
        case 'a':
            opts |= LWS_SERVER_OPTION_ALLOW_NON_SSL_ON_SSL_PORT;
            break;
        case 'p':
            info.port = atoi(optarg);
            break;
        case 'i':
            strncpy(interface_name, optarg, sizeof interface_name);
            interface_name[(sizeof interface_name) - 1] = '\0';
            iface = interface_name;
            break;
        case 'h':
            fprintf(stderr, "Usage: test-server "
                    "[--port=<p>] [--ssl] "
                    "[-d <log bitfield>] ");
            exit(1);
        }
    }

#if !defined(LWS_NO_DAEMONIZE) && !defined(WIN32)
    /*
     * normally lock path would be /var/lock/lwsts or similar, to
     * simplify getting started without having to take care about
     * permissions or running as root, set to /tmp/.lwsts-lock
     */
    if (daemonize && lws_daemonize("/tmp/.lwsts-lock")) {
        fprintf(stderr, "Failed to daemonize\n");
        return 1;
    }
#endif

    //signal(SIGINT, sighandler);

    /* tell the library what debug level to emit and to send it to syslog */
    lws_set_log_level(debug_level, lwsl_emit_syslog);

    info.iface = iface;
    info.protocols = protocols;

    info.gid = -1;
    info.uid = -1;
    info.options = opts;

    context = libwebsocket_create_context(&info);
    if (context == NULL) {
        lwsl_err("libwebsocket init failed\n");
        return -1;
    }

    memset(&remote_shmems, 0, sizeof(ar_remote_shmems_t));

    init_rt_task();

    fprintf(stderr, "\nPress ENTER for exit\n\n");
    getchar();

    libwebsocket_context_destroy(context);

    lwsl_notice("libwebsockets-test-server exited cleanly\n");

    return 0;
}


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

#include "../include/websocket_service.h"


ar_remote_shmems_t remote_shmems;

// Массив указателей на блоки памяти, готовые к передаче
// Первое два байта в блоке памяти - длина последующего блока, предназначенного для передачи (длина лежащего там bson объекта)
void** ar_bufs = NULL;
size_t ar_bufs_len=0;

RT_TASK task_read_shmem;
int priority_task_read_shmem = 50;

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


static int callback_telemetry(struct libwebsocket_context *context, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
{
    int n, m;
    unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 + LWS_SEND_BUFFER_POST_PADDING];
    unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
    struct per_session_data__telemetry *pss = (struct per_session_data__telemetry *)user;

    //bson_t bson;
    bson_oid_t oid;
    bson_t *doc;

    char pipe_buf[500]; // TODO:
    ssize_t read_len;

    int i;

    switch (reason) {

    case LWS_CALLBACK_ESTABLISHED:
        lwsl_info("callback_telemetry: " "LWS_CALLBACK_ESTABLISHED\n");
        pss->number = 0;
        break;

    case LWS_CALLBACK_SERVER_WRITEABLE:


        read_len= read(pipe_fd, pipe_buf, sizeof(pipe_buf));
        if(read_len==-1)
        {
            printf("Error read from pipe\n");
            return -1;
        }

        if (read_len > 0) {
            // со смещением в два байта читаем следующий блок данных
            m = libwebsocket_write(wsi, (unsigned char *)pipe_buf, read_len, LWS_WRITE_BINARY);
            if (m < read_len) {
                lwsl_err("ERROR %d writing to di socket\n", n);
                return -1;
            }
        }
        break;


    case LWS_CALLBACK_RECEIVE:
//		fprintf(stderr, "rx %d\n", (int)len);
        if (len < 6)
            break;
        if (strcmp((const char *)in, "reset\n") == 0)
            pss->number = 0;
        break;
    /*
     * this just demonstrates how to use the protocol filter. If you won't
     * study and reject connections based on header content, you don't need
     * to handle this callback
     */

    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
        //dump_handshake_info(wsi);
        /* you could return non-zero here and kill the connection */
        break;

    default:
        break;
    }

    return 0;
}

void debug_print_bson(char* where, bson_t* bson) {
    printf("%s\n", where);
    char* str = bson_as_json(bson, NULL);
    fprintf(stdout, "%s\n", str);
    bson_free(str);
    printf("\n");
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
    RT_PIPE pipe_between_rt;
    int err;
    err = rt_pipe_create(&pipe_between_rt, "telemetry", 0, 500);
    if (err)
    {
        printf("Error create pipe\n");
        return;
    }

    while(1)
    {
        int i;
        for(i=0; i < remote_shmems.remote_shmems_len; i++)
        {
            shmem_in_set_t* remote_shmem = remote_shmems.remote_shmems[i];

            if(remote_shmem->f_shmem_connected)
            {
                //TODO: Определить размер буфера где нибудь в настройках
                // и вынести в структуру
                char buf[500];
                unsigned short retlen;
                retlen=0;
                read_shmem(&remote_shmem->remote_shmem, buf, &retlen);


                if (retlen > 0) {
                    bson_t* bson = bson_new_from_data (buf, retlen);

                    bson_append_utf8 (bson, "_from", -1, remote_shmem->name_instance, -1);
debug_print_bson("bson_append_utf8", bson);

                    ssize_t send = rt_pipe_write(&pipe_between_rt, bson_get_data(bson), bson->len, P_NORMAL);
                    if(P_NORMAL<0)
                    {
                        printf("Error rt_pipe_write %i\n", send);
                        continue;
                    }
                }
            }
        }
        rt_task_sleep(rt_timer_ns2ticks(100000000));
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
    if (err != 0)
        printf("Error start main task\n");

    return 0;
}

int main(int argc, char **argv)
{
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


    // Зарегистрируем модули
    memset(&remote_shmems, 0, sizeof(ar_remote_shmems_t));
    register_remote_shmem(&remote_shmems, "test-sender-1", "Output1");


    init_rt_task();



    pipe_fd = open("/proc/xenomai/registry/native/pipes/telemetry", O_RDWR);
    if (pipe_fd < 0)
    {
        printf("Error open pipe /proc/xenomai/registry/native/pipes/telemetry\n");
        return -1;
    }


    n = 0;
    while (n >= 0 && !force_exit) {
        struct timeval tv;

        gettimeofday(&tv, NULL);

        /*
         * This provokes the LWS_CALLBACK_SERVER_WRITEABLE for every
         * live websocket connection using the TELEMETRY protocol,
         * as soon as it can take more packets (usually immediately)
         */

        if (((unsigned int)tv.tv_usec - oldus) > 50000) {
            libwebsocket_callback_on_writable_all_protocol(&protocols[PROTOCOL_TELEMETRY]);
            oldus = tv.tv_usec;
        }



        if(!remote_shmems.f_connected_in_links)
        {
            printf("попытка in связи %i\n", remote_shmems.f_connected_in_links);

            char* local_instance_name = "telemetry";
            connect_in_links(&remote_shmems, local_instance_name);
        }



        /*
         * If libwebsockets sockets are all we care about,
         * you can use this api which takes care of the poll()
         * and looping through finding who needed service.
         *
         * If no socket needs service, it'll return anyway after
         * the number of ms in the second argument.
         */
        n = libwebsocket_service(context, 20);
    }


    libwebsocket_context_destroy(context);

    lwsl_notice("libwebsockets-test-server exited cleanly\n");

    return 0;
}

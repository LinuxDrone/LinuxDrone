/**
  * @file
  * @brief
  * \~russian Файл содержит функции, необходимые для реализации модуля.
  * \~english This file contains the functions necessary to implement the module.
  */

#include <stdio.h>
#include <locale.h>

#ifdef WIN32
#include <apr_getopt.h>
#else
// Mac OS X
#include <apr-1/apr_getopt.h>
#endif


#define INSTANCE_SEPARATOR "#"
#define PIN_SEPARATOR "@"
#define PORT_SEPARATOR ":"

#define DEF_POLLSET_NUM		32

#include "../include/apr-module-functions.h"


#define SHMEM_WRITER_MASK	0x7FFFFFFF
#define SHMEM_HEAP_SIZE		300
#define SHMEM_BLOCK1_SIZE	200

#define CLIENT_TCP_SOCKET_TIMEOUT 100000 // Таймаут операций клиенского TCP сокета в микросекундах


/* listen port number */
int listen_port = 0;

/**
 * @brief
 * \~russian    Заполняет указатель адресом на структуру,
 *              данные из которой можно считывать для передачи в разделяемую память
 * \~english Fills the pointer location on the structure, the data from which can be read to transfer into the shared memory
 * @param obj
 * @param was_queue \~russian Если возаращается объект переданный через очередь
 * @return
 * \~russian 0 в случае успеха
 */
#ifdef WIN32
__declspec(dllexport) int checkout4transmiter(module_t* module, out_object_t* set, void** obj, bool was_queue)
#else
int checkout4transmiter(module_t* module, out_object_t* set, void** obj, bool was_queue)
#endif
{
	(*obj)=set->obj1;

    return 0;
}


/**
 * @brief
 * \~russian Возвращает объект системе (объект будет помечен как свободный для записи основным потоком)
 * \~english Returns to the system (to be marked as free to record the main thread)
 * @param obj
 * @param was_queue \~russian Если возаращается объект переданный через очередь \~english If the returned object is passed through the queue
 * @return
 */
int checkin4transmiter(module_t* module, out_object_t* set, void** obj, bool was_queue)
{
    (*obj)=NULL;

	return 0;
}



/**
 * @brief Функция добавляет линк в список линков, связывающих данный модуль с удаленным модулем подписчиком (обмен через очередь)
 * Подготавливает структуры необходимые для работы с данными линками (мапинг свойств) в риалтайме
 * @return
 */
int register_out_link(out_object_t* out_object, const char* subscriber_instance_name, unsigned short offset_field, TypeFieldObj type_field_obj, const char* remote_field_name, remote_queue_t* remote_queue)
{
    // Найдем данные ассоциированные с инстансом подписчика
    // Если таковых не зарегистрировано, то создадим (выделим память и заполним) необходимые структуры
    out_queue_set_t* out_queue_set=NULL;
    if(out_object->out_queue_sets!=NULL)
    {
        int i;
        for(i=0;i<out_object->out_queue_sets_len;i++)
        {
            out_queue_set = out_object->out_queue_sets[i];
            if(strcmp(out_queue_set->out_queue->name_instance, subscriber_instance_name)==0)
                break;
            else
                out_queue_set = NULL;
        }
    }

    if(out_queue_set==NULL)
    {
        // Если инстанс вообще не зарегестрирован в списке потребителей нашего модуля, сделаем это
        out_object->out_queue_sets_len += 1;
        out_object->out_queue_sets = realloc(out_object->out_queue_sets, sizeof(out_queue_set_t*)*out_object->out_queue_sets_len);
        out_queue_set = calloc(1, sizeof(out_queue_set_t));
        out_queue_set->out_queue = remote_queue;
        out_object->out_queue_sets[out_object->out_queue_sets_len-1] = out_queue_set;
    }

    // Зарегистрируем линк
    out_queue_set->len += 1;
    out_queue_set->remote_out_obj_fields = realloc(out_queue_set->remote_out_obj_fields, sizeof(remote_out_obj_field_t*)*out_queue_set->len);
    remote_out_obj_field_t* remote_obj_field = calloc(1, sizeof(remote_out_obj_field_t));
    remote_obj_field->offset_field_obj = offset_field;
    remote_obj_field->remote_field_name = malloc(strlen(remote_field_name)+1);
    strcpy(remote_obj_field->remote_field_name, remote_field_name);
    remote_obj_field->type_field_obj = type_field_obj;
    out_queue_set->remote_out_obj_fields[out_queue_set->len-1] = remote_obj_field;

    return 0;
}


/**
 * @brief Функция добавляет линк в список линков, связывающих данный модуль с удаленным модулем поставщиком (обмен через разделяемую память)
 * Подготавливает структуры необходимые для работы с данными линками (мапинг свойств) в риалтайме
 * @return
 */
int register_in_link(shmem_in_set_t* shmem, TypeFieldObj type_field_obj, const char* remote_field_name, unsigned short  offset_field_obj)
{
    // Зарегистрируем линк
    shmem->len_remote_in_obj_fields += 1;
    shmem->remote_in_obj_fields = realloc(shmem->remote_in_obj_fields, sizeof(remote_in_obj_field_t*)*shmem->len_remote_in_obj_fields);
    remote_in_obj_field_t* remote_obj_field = calloc(1, sizeof(remote_in_obj_field_t));
    remote_obj_field->remote_field_name = malloc(strlen(remote_field_name)+1);
    strcpy(remote_obj_field->remote_field_name, remote_field_name);
    remote_obj_field->offset_field_obj = offset_field_obj;
    remote_obj_field->type_field_obj = type_field_obj;
    shmem->remote_in_obj_fields[shmem->len_remote_in_obj_fields-1] = remote_obj_field;

    return 0;
}


/**
* @brief Функция регистрирует удаленный инстанс, к которому будем бегать выпрашивать его информацию.
* @return
*/

#ifdef WIN32
__declspec(dllexport) shmem_in_set_t* register_remote_shmem(apr_pool_t *mp, ar_remote_shmems_t* ar_remote_shmems, const char* name_remote_instance, const char* name_remote_outgroup)
#else
shmem_in_set_t* register_remote_shmem(apr_pool_t *mp, ar_remote_shmems_t* ar_remote_shmems, const char* name_remote_instance, const char* name_remote_outgroup)
#endif
{
    if(ar_remote_shmems==NULL)
    {
        fprintf(stderr, "Function \"register_remote_shmem\" null parameter ar_remote_shmems\n");
        return NULL;
    }

    int i=0;
    for(i=0;i<ar_remote_shmems->remote_shmems_len;i++)
    {
        shmem_in_set_t* info_remote_shmem = ar_remote_shmems->remote_shmems[i];
        if(strcmp(info_remote_shmem->name_instance, name_remote_instance)==0 && strcmp(info_remote_shmem->name_outgroup, name_remote_outgroup)==0)
        {
            // Разделяемая память уже зарегистрирована
            return info_remote_shmem;
        }
    }

    // Разделяеимая память не зарегистрирована и set следует создать и сохранить на него ссылку в массиве.
    ar_remote_shmems->remote_shmems_len +=1;
    ar_remote_shmems->remote_shmems = realloc(ar_remote_shmems->remote_shmems, sizeof(shmem_in_set_t*)*ar_remote_shmems->remote_shmems_len);
    shmem_in_set_t* new_remote_shmem = calloc(1, sizeof(shmem_in_set_t));

    new_remote_shmem->name_instance = malloc(strlen(name_remote_instance)+1);
    strcpy(new_remote_shmem->name_instance, name_remote_instance);

    new_remote_shmem->name_outgroup = malloc(strlen(name_remote_outgroup)+1);
    strcpy(new_remote_shmem->name_outgroup, name_remote_outgroup);
    
    
    // создадим клинский сокет для пулинга по данной входящей связи
    char* remote_addr = malloc(strlen(name_remote_instance) + 1);
    strcpy(remote_addr, name_remote_instance);
    char* name_remote_host = strtok(remote_addr, PORT_SEPARATOR);
    char* name_remote_port = strtok('\0', "");
    if (name_remote_port == NULL)
    {
        fprintf(stderr, "Need port for remote instance for link %s:%s\n", name_remote_instance, name_remote_outgroup);
        exit(EXIT_FAILURE);
    }
    apr_port_t remote_port = atoi(name_remote_port);
    
    apr_status_t rv;
    rv = apr_sockaddr_info_get(&new_remote_shmem->sockaddr, name_remote_host, APR_INET, remote_port, 0, mp);
    if (rv != APR_SUCCESS) {
        fprintf(stderr, "error: apr_sockaddr_info_get. new_remote_shmem->sockaddr\n");
        return NULL;
    }
    rv = apr_socket_create(&new_remote_shmem->socket, new_remote_shmem->sockaddr->family, SOCK_STREAM, APR_PROTO_TCP, mp);
    if (rv != APR_SUCCESS) {
        fprintf(stderr, "error: apr_socket_create. new_remote_shmem->socket\n");
        return NULL;
    }
    /* it is a good idea to specify socket options explicitly.
     * in this case, we make a blocking socket with timeout. */
    apr_socket_opt_set(new_remote_shmem->socket, APR_SO_NONBLOCK, 0);
	apr_socket_timeout_set(new_remote_shmem->socket, CLIENT_TCP_SOCKET_TIMEOUT);


    ar_remote_shmems->remote_shmems[ar_remote_shmems->remote_shmems_len-1] = new_remote_shmem;

    ar_remote_shmems->f_connected_in_links=false;

    return new_remote_shmem;
}

#ifdef WIN32
__declspec(dllexport) int unregister_remote_shmem(ar_remote_shmems_t* ar_remote_shmems, const char* name_remote_instance, const char* name_remote_outgroup)
#else
int unregister_remote_shmem(ar_remote_shmems_t* ar_remote_shmems, const char* name_remote_instance, const char* name_remote_outgroup)
#endif
{
    /*
    if(ar_remote_shmems==NULL)
    {
        fprintf(stderr, "Function \"unregister_remote_shmem\" null parameter ar_remote_shmems\n");
        return 0;
    }

    int i=0;
    for(i=0;i<ar_remote_shmems->remote_shmems_len;i++)
    {
        shmem_in_set_t* info_remote_shmem = ar_remote_shmems->remote_shmems[i];
        if(strcmp(info_remote_shmem->name_instance, name_remote_instance)==0 && strcmp(info_remote_shmem->name_outgroup, name_remote_outgroup)==0)
        {
            // Разделяемая память уже зарегистрирована
            // Отконнектить объекты ксеномая и освободить память
            if(disconnect_in_links(info_remote_shmem)!=0)
            {
                fprintf(stderr, "Function \"unregister_remote_shmem\" Error disconnect_in_links()\n");
                return -1;
            }

            int f;
            for(f=0;f<info_remote_shmem->len_remote_in_obj_fields;f++)
            {
                remote_in_obj_field_t* remote_in_obj_field = info_remote_shmem->remote_in_obj_fields[f];
                free(remote_in_obj_field->remote_field_name);
                free(remote_in_obj_field);
            }
            free(info_remote_shmem->remote_in_obj_fields);

            free(info_remote_shmem->name_instance);
            free(info_remote_shmem->name_outgroup);
            free(info_remote_shmem);


            remove_element(ar_remote_shmems->remote_shmems, i, ar_remote_shmems->remote_shmems_len);  
     // First shift the elements, then reallocate
            shmem_in_set_t** tmp = realloc(ar_remote_shmems->remote_shmems, (ar_remote_shmems->remote_shmems_len - 1) * sizeof(shmem_in_set_t*) );
            if (tmp == NULL && ar_remote_shmems->remote_shmems_len > 1) {
                // No memory available
                fprintf(stderr, "Function \"unregister_remote_shmem\" No memory available\n");
                exit(EXIT_FAILURE);
            }
            ar_remote_shmems->remote_shmems_len--;
            ar_remote_shmems->remote_shmems = tmp;

            return 0;
        }
    }
*/
    return -1;
     
}



/**
 * @brief Фунция проверяет, зарегистрирована ли ссылка на очередь инстанса потребителя (входная очередь модуля потребителя)
 * Если таковой нет, то создается очередь и ссылка на нее сохраняется в массиве
 * @param module Массив хранящий ссылки на входные очереди модулей подписчиков
 * @param name_remote_queue Имя входной очереди модуля подписчика
 */
remote_queue_t* register_remote_queue(module_t* module, const char* name_remote_instance)
{
    if(module==NULL)
    {
        fprintf(stderr, "Function \"register_remote_queue\" null parameter module\n");
        return NULL;
    }

    int i=0;
    for(i=0;i<module->remote_queues_len;i++)
    {
        remote_queue_t* info_remote_queue = module->remote_queues[i];
        if(strcmp(info_remote_queue->name_instance, name_remote_instance)==0)
        {
            // Очередь уже зарегистрирована
            return info_remote_queue;
        }
    }

    // Очередь не зарегистрирована и ее следует создать и сохранить на нее ссылку в массиве.
    module->remote_queues_len +=1;
    module->remote_queues = realloc(module->remote_queues, sizeof(remote_queue_t*)*module->remote_queues_len);
    remote_queue_t* new_remote_queue = calloc(1, sizeof(remote_queue_t));
    new_remote_queue->name_instance = malloc(strlen(name_remote_instance)+1);
    strcpy(new_remote_queue->name_instance, name_remote_instance);
    module->remote_queues[module->remote_queues_len-1] = new_remote_queue;

    return new_remote_queue;
}

int usage(module_t* module, char *argv[])
{
    fprintf(stderr, "\nusage: %s [OPTION]...\n\n", argv[0]);
    
    fprintf(stderr, "--help\n\tdisplay this help and exit\n\n");
    
    fprintf(stderr, "--module-definition\n\tprint JSON module definition\n\n");
    
    fprintf(stderr, "--print-params\n\tprint params for module instance\n\n");
    
    fprintf(stderr, "--name=NAME\n");
    fprintf(stderr, "\trequired argument\n");
    fprintf(stderr, "\tInstance name\n\n");
    
    fprintf(stderr, "--port=PORT\n");
    fprintf(stderr, "\trequired argument\n");
    fprintf(stderr, "\tListen IP port\n\n");
    
    fprintf(stderr, "--main-task-period=PERIOD\n");
    fprintf(stderr, "\toptional\n");
    fprintf(stderr, "\tBusiness function execution period in microseconds (default: 20000)\n\n");
    
    fprintf(stderr, "--in-link=INSTANCE_TRANSMITTER.OBJ_NAME.OUT_NAME->IN_NAME\n");
    fprintf(stderr, "\toptional\n");
    fprintf(stderr, "\tInput link (provides data from another instance to this one through shared memory)\n\n");
    
    fprintf(stderr, "--out-link=OUT_NAME#INSTANCE_RECEIVER@IN_NAME\n");
    fprintf(stderr, "\toptional\n");
    fprintf(stderr, "\tOutput link (provides data from this instance to another one through pipe)\n\n");
    
    module->print_help();
    
    exit(EXIT_SUCCESS);
}


/**
 * @brief Возвращает название типа данных порта (входного или выходного), по имени порта
 * @param port_name
 * @return 0 в случае успеха
 */
int get_porttype_by_portname(module_t* module, const char* port_name, char* port_type_name)
{
    char m_format[64] = "\"";
    strcat(m_format, port_name);
    strcat(m_format, "\":{");
    
    char* f = strstr(module->json_module_definition, m_format);
    if(f==NULL)
    {
        fprintf(stderr, "Function: get_porttype_by_portname. Not found substring: \"%s\" in string: %s\n", m_format, module->json_module_definition);
    }
    
    char* t_find = "\"type\":\"";
    char* s = strstr(f, t_find);
    if(f==NULL)
    {
        fprintf(stderr, "Function: get_porttype_by_portname. Not found substring: \"%s\" in string: %s\n", t_find, f);
    }
    
    char* s_begin = s + strlen(t_find);
    //printf("s_begin:=%s\n", s_begin);
    
    
    char* q_find = "\"";
    char* s_end = strstr(s_begin, q_find);
    if(f==NULL)
    {
        fprintf(stderr, "Function: get_porttype_by_portname. Not found substring: \"%s\" in string: %s\n", q_find, s_begin);
    }
    
    long str_type_len = s_end - s_begin;
    
    memcpy(port_type_name, s_begin, str_type_len);
    port_type_name[str_type_len]=0;
    
    //printf("str_port_type=%s\n", port_type_name);
    
    return 0;
}

// Convert argv to structure common_params_t
int argv2common_params(void* in_module, int argc, char *argv[])
{
    module_t* module = in_module;
    if (!module)
    {
        printf("Error: func bson2common_params, NULL parameter\n");
        return -1;
    }
    
    
    apr_status_t rv;
    apr_pool_t *mp;
    /* API is data structure driven */
    static const apr_getopt_option_t opt_option[] = {
        // long-option, short-option, has-arg flag, description
        { "main-task-period", 'm', TRUE, "main task period" },
        { NULL, 0, 0, NULL }, /* end (a.k.a. sentinel) */
    };
    apr_getopt_t *opt;
    int optch;
    const char *optarg;
    
    apr_pool_create(&mp, NULL);
    
    /* initialize apr_getopt_t */
    apr_getopt_init(&opt, mp, argc, (const char *const *)argv);
    
    module->common_params.main_task_period = 20000; // Дефолтное значение в микросекундах для периода выполнения бизнес-функции
    
    opt->errfn = NULL;
    opt->interleave = true;
    
    while ((rv = apr_getopt_long(opt, opt_option, &optch, &optarg)) != APR_EOF) {
        switch (optch) {
            case 'm':
                if (optarg != NULL)
                {
                    module->common_params.main_task_period = atoll(optarg);
                    if (module->common_params.main_task_period < 0)
                    {
                        printf("argument 'main-task-period' valid values >-1\n\n");
                        usage(module, argv);
                    }
                }
                break;
        }
    }
    return 0;
}


/**
 * @brief init Инициализация инстанса модуля
 * Принимает на вход массив параметров командной строки argv
 * @param module
 * @param argc
 * @param argv
 * @return
 */
int init(module_t* module, int argc, char *argv[])
{
	setlocale(LC_ALL, "ru-RU");

	apr_status_t rv;
	apr_pool_t *mp;
	/* API is data structure driven */
	static const apr_getopt_option_t opt_option[] = {
		// long-option, short-option, has-arg flag, description 
		{ "help", 'h', FALSE, "help" },
		{ "module-definition", 'd', FALSE, "module definition" },
		{ "print-params", 's', FALSE, "print params" },
		{ "name", 'n', TRUE, "instance name" },
		{ "out-link", 'o', TRUE, "out link" },
		{ "in-link", 'i', TRUE, "in link" },
		{ "port", 'p', TRUE, "IP port for TCP and UDP sockets" },
		{ NULL, 0, 0, NULL } /* end (a.k.a. sentinel) */
	};
	apr_getopt_t *opt;
	int optch;
	const char *optarg;

	apr_pool_create(&mp, NULL);

	/* initialize apr_getopt_t */
	apr_getopt_init(&opt, mp, argc, (const char * const*)argv);


	opt->errfn = NULL;
	opt->interleave = true;

	bool print_params = false;
	/* parse the all options based on opt_option[] */
	while ((rv = apr_getopt_long(opt, opt_option, &optch, &optarg)) != APR_EOF) {
		switch (optch) {
		case 'h':
			usage(module, argv);
			break;

		case 's':
			print_params = true;
			break;


			// Имя инстанса
		case 'n':
			if (optarg != NULL){
				// Проверяем, не превышает ли длина имени инстанса значения 32-5=27
				// Т.к. максимальная длина имен объектов ксеномая равна 32 а 5 символов могут быть использованы на суфикс,
				// при формировании имен тасков, очередй и пр., необходимых для объетов ксеномая используемых инстансом в работе
				if (strlen(optarg) > XNOBJECT_NAME_LEN - 5)
				{
					fprintf(stderr, "Instance name (\"%s\") length (%lu) exceeds the maximum length allowed (%i)\n", module->instance_name, strlen(module->instance_name), XNOBJECT_NAME_LEN - 5);
					return -1;
				}
				module->instance_name = optarg;
				//fprintf(stdout, "instance name=%s\n\n", module->instance_name);
			}
			else
			{
				printf("required value for argument --name\n\n");
				usage(module, argv);
			}
			break;


		case 'o': // Исходящие связи (`OUT_NAME#INSTANCE_RECEIVER@IN_NAME`)
			if (optarg != NULL){
				// Выделяем память под структуры, представляющие связи с модулями подписчиками
				// Связи через очередь (данный модуль поставщик, другие потребители данных)
				char* param_val = malloc(strlen(optarg) + 1);
				strcpy(param_val, optarg);

				char* name_out_pin = strtok(param_val, INSTANCE_SEPARATOR);
				//name_out_pin[strlen(name_out_pin) - 1] = 0;
				//printf("name_out_pin=%s\n", name_out_pin);

				char* name_remote_instance = strtok('\0', PIN_SEPARATOR);
				//printf("name_remote_instance=%s\n", name_remote_instance);

				char* name_remote_pin = strtok('\0', "");
				//printf("name_remote_pin=%s\n", name_remote_pin);

				if (strlen(name_remote_instance) > XNOBJECT_NAME_LEN - 5)
				{
					fprintf(stderr, "Remote Instance name (\"%s\") length (%lu) exceeds the maximum length allowed (%i)\n", name_remote_instance, strlen(name_remote_instance), XNOBJECT_NAME_LEN - 5);
					free(param_val);
					return -1;
				}

				// Добавим имя инстанса подписчика и ссылку на объект его очереди (если оно не было зафиксировано раньше, то будут созданы необходимые структуры для его хранения)
				remote_queue_t* remote_queue = register_remote_queue(module, name_remote_instance);

				// Получим название типа данных связи
				char portType_name[32];
				get_porttype_by_portname(module, name_out_pin, portType_name);
				TypeFieldObj port_type = convert_port_type_str2type(portType_name);
				if (port_type == field_unknown)
				{
					fprintf(stderr, "Error convert data type of port \"%s\" from string \"%s\" for instance \"%s\"\n", name_out_pin, portType_name, module->instance_name);
					free(param_val);
					return -1;
				}

				unsigned short offset_field;
				unsigned short index_port;
				out_object_t* out_object = (*module->get_outobj_by_outpin)(module, name_out_pin, &offset_field, &index_port);
				if (out_object)
				{
					register_out_link(out_object, name_remote_instance, offset_field, port_type, name_remote_pin, remote_queue);
				}
				else
				{
					fprintf(stderr, "Not found OUT PIN \"%s\" in instance \"%s\"\n", name_out_pin, module->instance_name);
				}
				free(param_val);
			}
			else
			{
				printf("require value for argument --out-link\n\n");
				usage(module, argv);
			}
			break;


		case 'i': // Входящие связи (`INSTANCE_TRANSMITTER@OBJ_NAME@OUT_NAME#IN_NAME`)
			// Выделяем память под структуры, представляющие связи с модулями поставщиками
			// Связи через разделяемую память (данный модуль потребитель, другие поставщики данных)
			// Список входящих связей, должен быть в массиве "in_links" в объекте конфигурации
			// но его может не быть, если модуль не имеет входа
			if (optarg != NULL)
			{
				char* param_val = malloc(strlen(optarg) + 1);
				strcpy(param_val, optarg);

				// имя инстанса модуля поставщика
				char* publisher_instance_name = strtok(param_val, PIN_SEPARATOR);

				// имя группы пинов инстанса поставщика
				char* publisher_nameOutGroup = strtok('\0', PIN_SEPARATOR);

				// название выходного пина инстанса поставщика
				char* remote_out_pin_name = strtok('\0', INSTANCE_SEPARATOR);
				//remote_out_pin_name[strlen(remote_out_pin_name) - 1] = 0;

				// название входного пина данного модуля
				char* input_pin_name = strtok('\0', "");

				//printf("publisher_instance_name=%s\n", publisher_instance_name);
				//printf("publisher_nameOutGroup=%s\n", publisher_nameOutGroup);
				//printf("remote_out_pin_name=%s\n", remote_out_pin_name);
				//printf("input_pin_name=%s\n", input_pin_name);

				if (strlen(publisher_instance_name) > XNOBJECT_NAME_LEN - 5)
				{
					fprintf(stderr, "Remote Instance name (\"%s\") length (%lu) exceeds the maximum length allowed (%i)\n", publisher_instance_name, strlen(publisher_instance_name), XNOBJECT_NAME_LEN - 5);
					free(param_val);
					return -1;
				}


				// Добавим имя инстанса подписчика и ссылку на объект его очереди (если оно не было зафиксировано раньше, то будут созданы необходимые структуры для его хранения)
				shmem_in_set_t* remote_shmem = register_remote_shmem(module->mp, &module->ar_remote_shmems, publisher_instance_name, publisher_nameOutGroup);


				// Получим название типа данных входной связи
				char portType_name[32];
				get_porttype_by_portname(module, input_pin_name, portType_name);
				//printf("portType_name=%s\n", portType_name);
				TypeFieldObj port_type = convert_port_type_str2type(portType_name);

				//int port_type = get_porttype_by_in_portname(module->json_module_definition, input_pin_name);
				if (port_type == field_unknown)
				{
					fprintf(stderr, "Error convert data type of port \"%s\" from string for instance \"%s\"\n", remote_out_pin_name, module->instance_name);
					free(param_val);
					return -1;
				}

				t_mask input_port_mask = (*module->get_inmask_by_inputname)(input_pin_name);
				if (input_port_mask)
				{
					remote_shmem->assigned_input_ports_mask |= input_port_mask;
					int offset_field = (*module->get_offset_in_input_by_inpinname)(module, input_pin_name);

					register_in_link(remote_shmem, port_type, remote_out_pin_name, offset_field);
				}
				else
				{
					fprintf(stderr, "Not found INPUT PIN \"%s\" in instance \"%s\"\n", input_pin_name, module->instance_name);
				}
				free(param_val);
			}
			else
			{
				printf("require value for argument --in-link\n\n");
				usage(module, argv);
			}
			break;

		case 'd':
			fprintf(stdout, "%s\n", module->json_module_definition);
			exit(EXIT_SUCCESS);
			break;

		case 'p':
			if (optarg != NULL)
			{
				listen_port = atoi(optarg);
				fprintf(stdout, "port: %i\n", listen_port);
			}
			break;

		}
	}


	if (module->instance_name == NULL)
	{
		printf("required argument --name\n\n");
		usage(module, argv);
		exit(EXIT_FAILURE);
	}


	// Запись в структуру общих параметров модуля (информация вынимается из параметров командной строки модуля)
	argv2common_params(module, argc, argv);

	// Умножаем на тысячу потому, что время в конфиге указывается в микросекундах, а функция должна принимать на вход наносекунды (а использоваться будут тики)
	//module->common_params.main_task_period = rt_timer_ns2ticks(module->common_params.main_task_period * 1000);

	// Запись в структуру специфичных параметров модуля
	(*module->argv2params)(module, argc, argv);

	if (print_params)
	{
		print_common_params(&module->common_params);
		(*module->print_params)(module->specific_params);
	}

	

    return 0;
}


#ifdef WIN32
__declspec(dllexport) void read_shmem(shmem_in_set_t* remote_shmem, void* data, apr_size_t* datalen)
#else
void read_shmem(shmem_in_set_t* remote_shmem, void* data, apr_size_t* datalen)
#endif
{
	if (!remote_shmem->f_socket_connected)
    {
        *datalen = 0;
        return;
    }

	apr_status_t rv;
	apr_size_t len = strlen(remote_shmem->name_outgroup);
	rv = apr_socket_send(remote_shmem->socket, remote_shmem->name_outgroup, &len); // Посылаем в качестве запроса имя выходного объекта

	rv = apr_socket_recv(remote_shmem->socket, data, datalen);
	if (rv == APR_EOF || *datalen == 0) {
		char buf_err[250];
		apr_strerror(rv, buf_err,	250);
		fprintf(stderr, "%s\n", buf_err);

		*datalen = 0;
		return;
	}
}




/**
 * @brief send2queues Рассылает объект по входным очередям инстансов подписчиков
 * @param out_object
 * @return
 */
int send2queues(out_object_t* out_object, void* data_obj, bson_t* bson_obj)
{
    uintptr_t pval;
    int i;
    for(i=0;i<out_object->out_queue_sets_len;i++)
    {
        bson_init (bson_obj);

        out_queue_set_t* out_queue_set = out_object->out_queue_sets[i];

		if (out_queue_set->out_queue->remote_queue == NULL)
		{
			continue;
		}

        int cl;
        for(cl=0;cl<out_queue_set->len;cl++)
        {
            remote_out_obj_field_t* remote_obj_field = out_queue_set->remote_out_obj_fields[cl];

			pval = (uintptr_t)data_obj + remote_obj_field->offset_field_obj;

            switch (remote_obj_field->type_field_obj)
            {
            case field_char:
                bson_append_int32 (bson_obj, remote_obj_field->remote_field_name, -1, *((char*)pval));
                break;

            case field_short:
                bson_append_int32 (bson_obj, remote_obj_field->remote_field_name, -1, *((short*)pval));
                break;

            case field_int:
                bson_append_int32 (bson_obj, remote_obj_field->remote_field_name, -1, *((int*)pval));
                break;

            case field_long:
                bson_append_int32 (bson_obj, remote_obj_field->remote_field_name, -1, *((int32_t*)pval));
                break;

            case field_long_long:
                bson_append_int64 (bson_obj, remote_obj_field->remote_field_name, -1, *((long long*)pval));
                break;

            case field_float:
                bson_append_double  (bson_obj, remote_obj_field->remote_field_name, -1, *((float*)pval));
                break;

            case field_double:
                bson_append_double  (bson_obj, remote_obj_field->remote_field_name, -1, *((double*)pval));
                break;

            case field_const_char:
                bson_append_utf8  (bson_obj, remote_obj_field->remote_field_name, -1, (const char*)pval, -1);
                break;

            case field_bool:
                bson_append_bool   (bson_obj, remote_obj_field->remote_field_name, -1, *((bool*)pval));
                break;

            default:
                fprintf(stderr, "Function \"send2queues\" Unknown type remote field: %i\n", remote_obj_field->type_field_obj);
                break;
            }
        }

//debug_print_bson("send2queues", bson_obj);

		apr_size_t len = bson_obj->len;
		apr_status_t rv = apr_socket_send(out_queue_set->out_queue->remote_queue, (const char *)bson_get_data(bson_obj), &len);
		if (rv != APR_SUCCESS) {
			fprintf(stderr, "Warning: %i rt_queue_write\n", rv);
			bson_destroy(bson_obj);
			return rv;
		}

        bson_destroy(bson_obj);
    }

    return 0;
}


/* default buffer size */
#define BUFSIZE			4096


RTIME time_last_publish_shmem;
RTIME time_attempt_link_modules;


/**
 * @brief connect_links Устанавливает исходящие соединения посредством очередей
 * С входными очередями инстансов подписчиков
 * @param p_module
 * @return
 */
int connect_out_links(void *p_module)
{
    module_t* module = p_module;
    
    int count_connected = 0, i;
    for(i=0; i < module->remote_queues_len;i++)
    {
        remote_queue_t* info_remote_queue = module->remote_queues[i];
        
        if(info_remote_queue->f_queue_connected)
        {
            count_connected++;
            continue;
        }
        
        
        char* remote_addr = malloc(strlen(info_remote_queue->name_instance) + 1);
        strcpy(remote_addr, info_remote_queue->name_instance);
        
        
        char* name_remote_host = strtok(remote_addr, PORT_SEPARATOR);
        //name_out_pin[strlen(name_out_pin) - 1] = 0;
        //printf("name_out_pin=%s\n", name_out_pin);
        
        char* name_remote_port = strtok('\0', "");
        if (name_remote_port == NULL)
        {
            fprintf(stderr, "Need port for remote instance\n");
            exit(EXIT_FAILURE);
        }
        apr_port_t remote_port = atoi(name_remote_port);
        //printf("name_remote_pin=%s\n", name_remote_pin);
        
        
        apr_status_t rv;
        apr_sockaddr_t *sa;
        apr_sockaddr_info_get(&sa, name_remote_host, APR_INET, remote_port, 0, module->mp);
        apr_socket_create(&info_remote_queue->remote_queue, sa->family, SOCK_DGRAM, APR_PROTO_UDP, module->mp);
        rv = apr_socket_connect(info_remote_queue->remote_queue, sa);
        free(remote_addr);
        
        if (rv != APR_SUCCESS)
        {
            //fprintf(stderr, "Error:%i rt_queue_bind instance=%s to queue %s\n", res, module->instance_name, name_queue);
            continue;
        }
        else
        {
            info_remote_queue->f_queue_connected=true;
            fprintf(stderr, "%sCONNECTED: %s to queue %s%s\n", ANSI_COLOR_YELLOW, module->instance_name, name_remote_host, ANSI_COLOR_RESET);
        }
        
        count_connected++;
    }
    
    if(count_connected==module->remote_queues_len)
    {
        module->f_connected_out_links=true;
        fprintf(stderr, "%s%s: ALL QUEUES CONNECTED%s\n", ANSI_COLOR_GREEN, module->instance_name, ANSI_COLOR_RESET);
    }
    return 0;
}


/**
 * @brief get_input_data Функция получения данных
 * Должна вызываться из бизнес функции модуля. Доставляет данные из входной очереди и из шаред мемори инстансов поставщиков
 * @param p_module
 */
#ifdef WIN32
__declspec(dllexport) void get_input_data(module_t *module)
#else
void get_input_data(module_t *module)
#endif
{
    //TODO: Определить размер буфера где нибудь в настройках
    // и вынести в структуру
    uint8_t buf[BUFSIZE];
    apr_size_t len = sizeof(buf) - 1;
    
    
	if (!module) {
        return;
    }

	// Передадим объекты, заполненные в предыдущем цикле
    transmit_object(module, &time_last_publish_shmem);
	if (!module->f_connected_out_links)
	{
		// Если не все связи модуля установлены, то будем пытаться их установить
		if (apr_time_now() - time_attempt_link_modules > 1000000)
		{
			//fprintf(stderr, "попытка out связи\n");
			connect_out_links(module);
			time_attempt_link_modules = apr_time_now();
		}
	}

    
    
    // Проверим не подцепился ли еще кто то к нашему TCP сокету с жаждой пулить с нас данные
    apr_socket_t *ns;/* accepted socket */
    apr_status_t rv;
    
    rv = apr_socket_accept(&ns, module->tcp_socket, module->mp);
    if (rv == APR_SUCCESS) {
        /* we watch @ns(connected socket) to know whether it is ready to read(APR_POLLIN) */
        apr_pollfd_t pfd = { module->mp, APR_POLL_SOCKET, APR_POLLIN, 0, { NULL }, NULL };
        pfd.desc.s = ns;
        apr_pollset_add(module->pollset, &pfd);
    }
    
    
    // Проверим не прилетел ли новый запрос данные наших выходов
    apr_int32_t num;
    const apr_pollfd_t *ret_pfd;

    rv = apr_pollset_poll(module->pollset, 0, &num, &ret_pfd);
    if (rv == APR_SUCCESS) {
        int i;
        //assert(num > 0);
        /* scan the active sockets */
        for (i = 0; i < num; i++)
        {
            apr_socket_t *socket = ret_pfd[i].desc.s;
            rv = apr_socket_recv(socket, (char*)buf, &len);

			if (APR_STATUS_IS_EAGAIN(rv)) {
				/* we have no data to read. we should keep polling the socket */
			}
			else if (APR_STATUS_IS_EOF(rv) || len == 0) {
				/* we lost TCP session.
				* XXX On Windows, rv would equal to APR_SUCCESS and len==0 in this case. So, we should check @len in addition to APR_EOF check */
				
				apr_pollfd_t pfd = { module->mp, APR_POLL_SOCKET, APR_POLLIN, 0, { NULL }, NULL };
				pfd.desc.s = socket;
				apr_pollset_remove(module->pollset, &pfd);

				apr_socket_close(socket);

				printf("zero data from socket\n");
				continue;
			}



                if(buf[len-1]=='\n'){
                    buf[len-1] = '\0';
                }else
                    buf[len] = '\0';
                fprintf(stdout, "len=%lu str=%s\n", len, buf);
                
                
                void* obj;
                bson_t bson_tr;
                int i_obj=0;
                out_object_t* out_object = module->out_objects[i_obj];
                while(out_object)
                {
                    if(strcmp(out_object->out_name, (const char*)buf)!=0) // Если имя выходного объекта совпадает с именем присланным в запросе
                    {
                        out_object = module->out_objects[++i_obj];
                        continue;
                    }

                     checkout4transmiter(module, out_object, &obj, false);
                     if(obj!=NULL)
                     {
                         bson_init (&bson_tr);
                         // Call user convert function
                         (*out_object->obj2bson)(obj, &bson_tr);
                         
                         apr_size_t len = bson_tr.len;
                         apr_status_t rv = apr_socket_send(socket, (const char*)bson_get_data(&bson_tr), &len);
fprintf(stderr, "was sended: %i bytes\n", len);
                         if (rv != APR_SUCCESS) {
                             fprintf(stderr, "error: %i apr_socket_send in get_input_data\n", rv);
                             //bson_destroy(bson_obj);
                             //return rv;
                         }
                         bson_destroy(&bson_tr);
                     
                         // Вернуть объект основному потоку на новое заполнение
                         checkin4transmiter(module, out_object, &obj, false);
                     }
                    out_object = module->out_objects[++i_obj];
                }
            
        }
    }

    

    if(module->input_data==NULL)
    {
        //здесь просто поспать потоку
        apr_sleep(module->common_params.main_task_period);

        //fprintf(stderr, "Module don't have input\n");
        return;
    }

    module->updated_input_properties = 0;




	rv = apr_socket_recv(module->in_socket, (char*)buf, &len);
	if (len > 0)
    {
        bson_t bson;
		bson_init_static(&bson, buf, (uint32_t)len);

		size_t err_offset;
		if (!bson_validate(&bson, BSON_VALIDATE_NONE, &err_offset)) {
			fprintf(stderr, "The document failed to validate at offset: %u\n", (unsigned)err_offset);
		}
		else{
debug_print_bson("get_input_data", &bson);
			if ((*module->input_bson2obj)(module, &bson) != 0)
			{
				fprintf(stderr, "Error: func get_input_data, input_bson2obj\n");
			}
			else
			{
				//fprintf(stderr, "%s%s:%s ",ANSI_COLOR_RED, module->instance_name, ANSI_COLOR_RESET);
				//(*module->print_input)(module->input_data);
			}
		}
        bson_destroy(&bson);
    }
    

	
    // Если установлены флаги того, что юзер обязательно хочет каких то данных,
    // то постараемся их вытащить из разделяемой памяти
    // Если конечно его запрос не удовлетворен уже (возможно) полученными данными
    // 1) получено:             0010011 updated_input_properties
    // 2) мне надо:             0110001 refresh_input_mask
    // 3) часть нужного
    // мне из полученного:      0010001 получается логическим И 1&2
    // 4) осталось получить:    0100000 получается исключающим ИЛИ 2^3
    //fprintf(stderr, "before logic oper mask=0x%08X\n", module->refresh_input_mask);
    //fprintf(stderr, "updated_input_properties=0x%08X\n", module->updated_input_properties);
    module->refresh_input_mask ^= (module->refresh_input_mask & module->updated_input_properties);
    //fprintf(stderr, "before refresh mask=0x%08X\n", module->refresh_input_mask);

    if(!module->ar_remote_shmems.f_connected_in_links)
    {
        // Если не все связи модуля установлены, то будем пытаться их установить
        if( apr_time_now() - module->time_attempt_link_modules > 1000000)
        {
            //fprintf(stderr, "попытка in связи\n");

            connect_in_links(&module->ar_remote_shmems, module->instance_name);

            module->time_attempt_link_modules= apr_time_now();
        }
    }
	

    refresh_input(module);
}




/**
 * @brief connect_in_links Устанавливает входящие соединения посредством разделяемой памяти
 * с инстансами поставщиками данных
 * @param p_module
 * @return
 */

#ifdef WIN32
__declspec(dllexport) int connect_in_links(ar_remote_shmems_t* ar_remote_shmems, const char* instance_name)
#else
int connect_in_links(ar_remote_shmems_t* ar_remote_shmems, const char* instance_name)
#endif
{
    
    int count_connected = 0, i;
    for(i=0; i < ar_remote_shmems->remote_shmems_len;i++)
    {
        shmem_in_set_t* remote_shmem = ar_remote_shmems->remote_shmems[i];

        if(!remote_shmem->f_socket_connected)
        {
            apr_status_t rv = apr_socket_connect(remote_shmem->socket, remote_shmem->sockaddr);
            if (rv == APR_SUCCESS) 
			{
				/* see the tutorial about the reason why we have to specify options again */
				apr_socket_opt_set(remote_shmem->socket, APR_SO_NONBLOCK, 0);
				apr_socket_timeout_set(remote_shmem->socket, CLIENT_TCP_SOCKET_TIMEOUT);

				fprintf(stderr, "%sCONNECTED: %s to socket %s%s\n", ANSI_COLOR_YELLOW, remote_shmem->name_instance, remote_shmem->name_outgroup, ANSI_COLOR_RESET);
				remote_shmem->f_socket_connected = true;
            }
			else
			{
				//fprintf(stderr, "error: apr_socket_connect addr:%s\n", remote_shmem->name_instance);
				char buf_err[250];
				apr_strerror(rv, buf_err, 250);
				fprintf(stderr, "%s", buf_err);
				
				continue;
			}
        }
        count_connected++;
    }

    if(ar_remote_shmems->remote_shmems_len>0 && count_connected==ar_remote_shmems->remote_shmems_len)
    {
        ar_remote_shmems->f_connected_in_links=true;
        fprintf(stderr, "%s%s: ALL SOCKETS CONNECTED%s\n", ANSI_COLOR_GREEN, instance_name, ANSI_COLOR_RESET);
    }

    return 0;
}


int disconnect_in_links(shmem_in_set_t* remote_shmem)
{
    /*
    if(remote_shmem->f_shmem_connected)
    {
        int res = rt_heap_unbind	(&remote_shmem->remote_shmem.h_shmem);
        if(res!=0)
        {
            fprintf(stderr, "Error:%i rt_heap_unbind for instance=%s\n", res, remote_shmem->name_instance);
            return -1;
        }

        fprintf(stderr, "%sDISCONNECTED: %s shmem%s\n", ANSI_COLOR_YELLOW, remote_shmem->name_instance, ANSI_COLOR_RESET);
        remote_shmem->f_shmem_connected = false;
    }


    if(remote_shmem->f_event_connected)
    {
        int res = rt_event_unbind(&remote_shmem->remote_shmem.eflags);
        if(res!=0)
        {
            fprintf(stderr, "Error:%i rt_event_unbind instance=%s\n", res, remote_shmem->name_instance);
            return -1;
        }
        fprintf(stderr, "%sDISCONNECTED: %s event service %s\n", ANSI_COLOR_YELLOW, remote_shmem->name_instance, ANSI_COLOR_RESET);
        remote_shmem->f_event_connected = false;
    }


    if(remote_shmem->f_mutex_connected)
    {
        int res = rt_mutex_unbind(&remote_shmem->remote_shmem.mutex_read_shmem);
        if(res!=0)
        {
            fprintf(stderr, "Error:%i rt_mutex_unbind instance=%s\n", res, remote_shmem->name_instance);
            return -1;
        }
        fprintf(stderr, "%sDISCONNECTED: %s to mutex service %s\n\n", ANSI_COLOR_YELLOW, remote_shmem->name_instance, ANSI_COLOR_RESET);
        remote_shmem->f_mutex_connected = false;
    }
*/
    return 0;
     
}


int transmit_object(module_t* module, RTIME* time_last_publish_shmem)
{
    void* obj;
    bson_t bson_tr;

    int i=0;
    out_object_t* out_object = module->out_objects[i];
    while(out_object)
    {
        //fprintf(stderr, "outside=%i bool=%i\n",i,time2publish2shmem);
        // Нашли обновившийся в основном потоке объект
        // Пуш в очереди подписчиков
        checkout4transmiter(module, out_object, &obj, true);
        if(obj!=NULL)
        {
            send2queues(out_object, obj, &bson_tr);
            //fprintf(stderr, "send2queues\t");
            //(*out_object->print_obj)(obj);
            checkin4transmiter(module, out_object, &obj, true);
        }
        out_object = module->out_objects[++i];
    }

    return 0;
}


int create_xenomai_services(module_t* module)
{
    if (listen_port == 0)
    {
        fprintf(stderr, "need --port param\n");
        exit(EXIT_FAILURE);
    }
    
    apr_status_t rv;
    apr_sockaddr_t *sa;
    
    rv = apr_sockaddr_info_get(&sa, NULL, APR_INET, listen_port, 0, module->mp);
    if (rv != APR_SUCCESS) {
        return rv;
    }
    
    
    if(module->input_data)
    {
        rv = apr_socket_create(&module->in_socket, sa->family, SOCK_DGRAM, APR_PROTO_UDP, module->mp);
        if (rv != APR_SUCCESS) {
            fprintf(stderr, "error: apr_socket_create. in_socket\n");
            return rv;
        }
        
        apr_socket_timeout_set(module->in_socket, module->common_params.main_task_period);
        
        rv = apr_socket_bind(module->in_socket, sa);
        if (rv != APR_SUCCESS) {
            fprintf(stderr, "error: apr_socket_bind. in_socket\n");
            return rv;
        }
    }
    
    
    if (module->out_objects)
    {
        // В принципе есть выходы у модуля
        apr_pollset_create(&module->pollset, DEF_POLLSET_NUM, module->mp, 0);
        
        rv = apr_socket_create(&module->tcp_socket, sa->family, SOCK_STREAM, APR_PROTO_TCP, module->mp);
        if (rv != APR_SUCCESS) {
            return rv;
        }
        
        /* non-blocking socket */
        apr_socket_opt_set(module->tcp_socket, APR_SO_NONBLOCK, 1);
        apr_socket_timeout_set(module->tcp_socket, 0);
        apr_socket_opt_set(module->tcp_socket, APR_SO_REUSEADDR, 1);/* this is useful for a server(socket listening) process */
        
        rv = apr_socket_bind(module->tcp_socket, sa);
        if (rv != APR_SUCCESS) {
            fprintf(stderr, "error: apr_socket_bind. tcp_socket\n");
            return rv;
        }
        rv = apr_socket_listen(module->tcp_socket, SOMAXCONN);
        if (rv != APR_SUCCESS) {
            fprintf(stderr, "error: apr_socket_listen. tcp_socket\n");
            return rv;
        }
    }
    
    return 0;
}



#ifdef WIN32
__declspec(dllexport) int start(void* p_module)
#else
int start(void* p_module)
#endif
{
    module_t* module = p_module;

    
    // * Create required xenomai services
     
    int err = create_xenomai_services(module);
    if (err != 0) {
        fprintf(stderr, "Error create xenomai services\n");
        return err;
    }


    if (module == NULL) {
        fprintf(stderr, "Function \"start\". Param \"module\" is null\n");
        return -1;
    }

    if (module->func == NULL) {
        fprintf(stderr, "module->func for main task required\n");
        return -1;
    }

	time_last_publish_shmem = apr_time_now();
	time_attempt_link_modules = apr_time_now();
	
	// Вызовем функцию. Возврата из нее нет.
	// Лучше бы вызвать ее из потока.
	module->func(p_module);

    return 0;
}


#ifdef WIN32
__declspec(dllexport) int stop(void* p_module)
#else
int stop(void* p_module)
#endif
{
    /*
    module_t* module = p_module;

    // TODO: Удалить и все сервисы ксеномая
    rt_task_delete(&module->task_transmit);
    rt_task_delete(&module->task_main);

    //int res = rt_heap_free(&module->h_shmem, module->shmem);

    free(module->module_type);

    free(module->out_objects);

    free(module);
*/
    return 0;
     
}




/**
 * @brief checkout4writer
 * \~russian    Заполняет указатель адресом на структуру ,
 *              которую можно заполнять данными для последующей передачи в разделяемую память
 * @param obj
 * @return
 * \~russian 0 в случае успеха
 */
#ifdef WIN32
__declspec(dllexport) int checkout4writer(module_t* module, out_object_t* set, void** obj)
#else
int checkout4writer(module_t* module, out_object_t* set, void** obj)
#endif
{
	(*obj)=set->obj1;
    return 0;
}


/**
 * @brief
 * \~russian Возвращает объект системе (данные будут переданы в разделяемую память)
 * \~english Returns to the system (the data will be transferred to the shared memory)
 * @param set \~russian Набор данных \~english Data set
 * @param obj \~russian Объект
 * @return
 */
#ifdef WIN32
__declspec(dllexport) int checkin4writer(module_t* module, out_object_t* set, void** obj)
#else
int checkin4writer(module_t* module, out_object_t* set, void** obj)
#endif
{
    (*obj)=NULL;
    return 0;
}

#define BUFSIZE 4096

/**
 * @brief refresh_input
 * \~russian Функция вычитывает данные из разделяемой памяти и мержит их во входной объект
 * @param p_module
 * @return
 */
int refresh_input(void* p_module)
{
    module_t* module = p_module;
	uintptr_t pval;

    // биты не установлены, обновления данных не требуется
    if(module->refresh_input_mask == 0)
        return 0;

    //TODO: Определить размер буфера где нибудь в настройках
    // и вынести в структуру
	char buf[BUFSIZE];

    // Бежим по всем входящим связям типа разделяемой памяти и если какая то из них
    // ассоциирована с требуемыми для обновления входами, произведем чтение объекта из разделяемой памяти и мапинг свойств
    int i=0;
    for(i=0;i<module->ar_remote_shmems.remote_shmems_len;i++)
    {
        shmem_in_set_t* remote_shmem = module->ar_remote_shmems.remote_shmems[i];
        if(remote_shmem->assigned_input_ports_mask & module->refresh_input_mask)
        {
			apr_size_t retlen;
			retlen = BUFSIZE;
            read_shmem(remote_shmem, buf, &retlen);
            //fprintf(stderr, "retlen=%i\n", retlen);
            bson_t bson;
            if (retlen > 0) 
			{
                bson_init_static(&bson, (const uint8_t*)buf, (uint32_t)retlen);
                //debug_print_bson("Receive from shared memory", &bson);

                int f;
                for(f=0;f<remote_shmem->len_remote_in_obj_fields;f++)
                {
                    remote_in_obj_field_t* remote_in_obj_field = remote_shmem->remote_in_obj_fields[f];

					pval = (uintptr_t)module->input_data + (uintptr_t)remote_in_obj_field->offset_field_obj;

                    bson_iter_t iter;
                    if (!bson_iter_init_find(&iter, &bson, remote_in_obj_field->remote_field_name)) {
                        fprintf(stderr, "Not found property \"%s\" in input bson readed from shared memory \"%s\" for instance %s\n", remote_in_obj_field->remote_field_name, remote_shmem->name_instance, module->instance_name);
                        return -1;
                    }

                    switch (remote_in_obj_field->type_field_obj)
                    {
                    case field_char:
                        if (!BSON_ITER_HOLDS_INT32(&iter)) {
                            fprintf(stderr, "Property \"%s\" not INT32 type in input bson readed from shared memory \"%s\" for instance %s\n", remote_in_obj_field->remote_field_name, remote_shmem->name_instance, module->instance_name);
                            return -1;
                        }
                        *((char*)pval) = bson_iter_int32(&iter);
                        break;

                    case field_short:
                        if (!BSON_ITER_HOLDS_INT32(&iter)) {
                            fprintf(stderr, "Property \"%s\" not INT32 type in input bson readed from shared memory \"%s\" for instance %s\n", remote_in_obj_field->remote_field_name, remote_shmem->name_instance, module->instance_name);
                            return -1;
                        }
                        *((short*)pval) = bson_iter_int32(&iter);
                        break;


                    case field_int:
                        if (!BSON_ITER_HOLDS_INT32(&iter)) {
                            fprintf(stderr, "Property \"%s\" not INT32 type in input bson readed from shared memory \"%s\" for instance %s\n", remote_in_obj_field->remote_field_name, remote_shmem->name_instance, module->instance_name);
                            return -1;
                        }
                        *((int*)pval) = bson_iter_int32(&iter);
                        break;

                    case field_long:
                        if (!BSON_ITER_HOLDS_INT32(&iter)) {
                            fprintf(stderr, "Property \"%s\" not INT32 type in input bson readed from shared memory \"%s\" for instance %s\n", remote_in_obj_field->remote_field_name, remote_shmem->name_instance, module->instance_name);
                            return -1;
                        }
                        *((long*)pval) = bson_iter_int32(&iter);
                        break;


                    case field_long_long:
                        if (!BSON_ITER_HOLDS_INT64(&iter)) {
                            fprintf(stderr, "Property \"%s\" not INT64 type in input bson readed from shared memory \"%s\" for instance %s\n", remote_in_obj_field->remote_field_name, remote_shmem->name_instance, module->instance_name);
                            return -1;
                        }
                        *((long long*)pval) = bson_iter_int64(&iter);
                        break;

                    case field_float:
                        if (!BSON_ITER_HOLDS_DOUBLE(&iter)) {
                            fprintf(stderr, "Property \"%s\" not DOUBLE type in input bson readed from shared memory \"%s\" for instance %s\n", remote_in_obj_field->remote_field_name, remote_shmem->name_instance, module->instance_name);
                            return -1;
                        }
                        *((float*)pval) = bson_iter_double(&iter);
                        break;

                    case field_double:
                        if (!BSON_ITER_HOLDS_DOUBLE(&iter)) {
                            fprintf(stderr, "Property \"%s\" not DOUBLE type in input bson readed from shared memory \"%s\" for instance %s\n", remote_in_obj_field->remote_field_name, remote_shmem->name_instance, module->instance_name);
                            return -1;
                        }
                        *((double*)pval) = bson_iter_double(&iter);
                        break;

                    case field_const_char:
                        if (!BSON_ITER_HOLDS_UTF8(&iter)) {
                            fprintf(stderr, "Property \"%s\" not UTF8 type in input bson readed from shared memory \"%s\" for instance %s\n", remote_in_obj_field->remote_field_name, remote_shmem->name_instance, module->instance_name);
                            return -1;
                        }
                        //pval = bson_iter_utf8(&iter, &length);
                        //TODO: malloc for string
                        break;

                    case field_bool:
                        if (!BSON_ITER_HOLDS_BOOL(&iter)) {
                            fprintf(stderr, "Property \"%s\" not BOOL type in input bson readed from shared memory \"%s\" for instance %s\n", remote_in_obj_field->remote_field_name, remote_shmem->name_instance, module->instance_name);
                            return -1;
                        }
                        *((bool*)pval) = bson_iter_bool(&iter);
                        break;

                    default:
                        fprintf(stderr, "Function \"refresh_input\" Unknown type remote field\n");
                        break;
                    }
                }
                //fprintf(stderr, "%s%s:%s ", ANSI_COLOR_BLUE, module->instance_name, ANSI_COLOR_RESET);
                //(*module->print_input)(module->input_data);
                bson_destroy(&bson);
            }
			else
			{
				// Разрыв соединения сокета.
				// Закроем сокет и вновь создадим его, т.к. ссылка на сокет после закрытия обнуляется
				apr_socket_close(remote_shmem->socket);

				apr_status_t rv = apr_socket_create(&remote_shmem->socket, remote_shmem->sockaddr->family, SOCK_STREAM, APR_PROTO_TCP, module->mp);
				if (rv != APR_SUCCESS) {
					fprintf(stderr, "error: apr_socket_create in refresh_input. remote_shmem->socket\n");
                    return rv;
				}
				/* it is a good idea to specify socket options explicitly.
				* in this case, we make a blocking socket with timeout. */
				apr_socket_opt_set(remote_shmem->socket, APR_SO_NONBLOCK, 0);
				apr_socket_timeout_set(remote_shmem->socket, CLIENT_TCP_SOCKET_TIMEOUT);

				// Пометим данное соединение как не установленное.
				remote_shmem->f_socket_connected = false;
				// И установим флаг, говорящий о том, что не все входящие связи установлены.
				module->ar_remote_shmems.f_connected_in_links = false;
			}
        }
    }

    // перед выходом обнулить биты. они отработаны
    module->refresh_input_mask = 0;

    return 0;
}



/**
 * \~russian Описание. Передача параметров в модуль
 * BSON объект, содержащий параметры, передается передающему потоку модуля посредством rt_task_send
 * В контексте передающего потока, парсится BSON и значения переносятся в структуру параметров.
 * При этом передающий поток, получает ссылку на структуру с параметрами, посредством вызова функции checkout_params
 * получать ссылку посредлством вызова функции, а не прямого доступа через module_t необходимо потому, что в функции лочится мьютекс,
 * не позволяющий (во время парсинга bson объекта заполнения структуры с параметрами) основному потоку модуля получить доступ к структуре.
 * Аналогично и основной поток модуля, когда ему нужно прочитать настроечные параметры, пользуется парой функций checkout_params и checkin_params
 *
 * В автосгенеренных хелпер файлах модуля, для поддержания данного функционала создаются
 *
 * - структура для хранения параметров
 * - пара функций получения и освобождения ссылки на структуру параметров (ссылка на функции должна быть доступна через струткуру модуля для потоков модуля)
 * - пара функций сериализации и десириализации bson объекта в структуру (ссылка на функции должна быть доступна через структуру модуля для потоков модуля)
 */

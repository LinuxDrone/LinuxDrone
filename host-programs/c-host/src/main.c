#include <sys/mman.h>
#include <dlfcn.h>
#include <stdio.h>
#include <bcon.h>
#include <mongoc.h>
#include "module-functions.h"

#define DB_HOST "mongodb://localhost:27017"
#define DB_NAME "test"
#define DB_CONF_COLLECTION "configuration"


// Хранилище указателей на загруженные пускачем инстансы модулей
bson_t instances;

char g_appPath[PATH_MAX];


bson_t* get_bson_from_db(const char* configuration_name, const char* configuration_version)
{
    bson_t *result = NULL;
    mongoc_collection_t *collection;
    mongoc_client_t *client;
    mongoc_cursor_t *cursor;
    const bson_t *item;
    bson_t *query;

    /* get a handle to our collection */
    client = mongoc_client_new (DB_HOST);
    collection = mongoc_client_get_collection (client, DB_NAME, DB_CONF_COLLECTION);

    /*
    // insert a document
    bson_oid_init (&oid, NULL);
    doc = BCON_NEW ("_id", BCON_OID (&oid),
                    "hello", BCON_UTF8 ("world!"));
    r = mongoc_collection_insert (collection, MONGOC_INSERT_NONE, doc, NULL, &error);
    if (!r) {
       fprintf (stderr, "%s\n", error.message);
       return EXIT_FAILURE;
    }
    */

    /* build a query to execute */
    query = BCON_NEW ("name", BCON_UTF8 (configuration_name), "version", BCON_UTF8(configuration_version));

    /* execute the query and iterate the results */
    cursor = mongoc_collection_find (collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    if (mongoc_cursor_next (cursor, &item)) {
       result = bson_copy (item);

       //str = bson_as_json (item, NULL);
       //printf ("%s\n", str);
       //bson_free (str);
    }

    /* release everything */
    mongoc_cursor_destroy (cursor);
    mongoc_collection_destroy (collection);
    mongoc_client_destroy (client);
    bson_destroy (query);
    //bson_destroy (doc);

    return result;
}


/**
 * @brief add_links2instance
 * extract info about links from configuration and add them to instance bson
 */
int add_links2instance(bson_t* bson_configuration, bson_t * module_instance, const char* module_instance_name)
{
    // Теперь надо вычитать линки и засунуть их как массивы в объект bson_module
    bson_iter_t iter_l;
    if (!bson_iter_init_find(&iter_l, bson_configuration, "links")) {
        fprintf(stderr, "Error: not found node \"links\" in configuration\n");
        return -1;
    }

    const uint8_t *link_buf = NULL;
    uint32_t link_buf_len = 0;
    bson_t bson_links;
    bson_iter_array(&iter_l, &link_buf_len, &link_buf);
    bson_init_static(&bson_links, link_buf, link_buf_len);

    bson_iter_t iter_links;
    if(!bson_iter_init (&iter_links, &bson_links))
    {
        fprintf(stderr, "Error: error create iterator for links\n");
        return -1;
    }



    // итерация по связям
    int lc = 0;
    bson_t bson_link;
    bson_t b_arr;

    // Добавление линков в список входящих связей
    bool was_in_links = false;
    while(bson_iter_next(&iter_links))
    {
        if(!BSON_ITER_HOLDS_DOCUMENT(&iter_links))
        {
            fprintf(stderr, "Error: iter_links not a link document\n");
            continue;
        }
        bson_iter_document(&iter_links, &link_buf_len, &link_buf);
        bson_init_static(&bson_link, link_buf, link_buf_len);

        bson_iter_t iter_inInst;
        if (!bson_iter_init_find(&iter_inInst, &bson_link, "inInst")) {
            fprintf(stderr, "Error: not found node \"inInst\" in link\n");
            debug_print_bson("Function \"add_links2instance\" main.c on error", &bson_link);
            return -1;
        }

        bson_iter_t iter_type;
        if (!bson_iter_init_find(&iter_type, &bson_link, "type")) {
            fprintf(stderr, "Error: not found node \"type\" in link\n");
            debug_print_bson("Function \"add_links2instance\" main.c on error", &bson_link);
            return -1;
        }

        // Имя инстанса с входящим линком
        const char* name_inInst = bson_iter_utf8(&iter_inInst, NULL);
        const char* name_type = bson_iter_utf8(&iter_type, NULL);
        // Если это имя равно имени данного инстанса, то добавим связь в список входящих связей данного модуля
        if(strncmp(name_inInst, module_instance_name, 100)==0 && strncmp(name_type, "memory", 100)==0)
        {
            if(!was_in_links)
            {
                bson_append_array_begin (module_instance, "in_links", -1, &b_arr);
                was_in_links = true;
            }
            char buffer [3];
            sprintf(buffer,"%i",lc);
            bson_append_document (&b_arr, buffer, -1, &bson_link);
            lc++;
        }
    }
    if(was_in_links)
    {
        bson_append_array_end (module_instance, &b_arr);
    }



    // Добавление линков в список исходящих связей
    if(!bson_iter_init (&iter_links, &bson_links))
    {
        fprintf(stderr, "Error: error create iterator for links\n");
        return -1;
    }

    bool was_out_links = false;
    while(bson_iter_next(&iter_links))
    {
        if(!BSON_ITER_HOLDS_DOCUMENT(&iter_links))
        {
            fprintf(stderr, "Error: iter_links not a link document\n");
            continue;
        }
        bson_iter_document(&iter_links, &link_buf_len, &link_buf);
        bson_init_static(&bson_link, link_buf, link_buf_len);

        bson_iter_t iter_outInst;
        if (!bson_iter_init_find(&iter_outInst, &bson_link, "outInst")) {
            fprintf(stderr, "Error: not found node \"outInst\" in link\n");
            debug_print_bson("Function \"add_links2instance\" main.c on error", &bson_link);
            return -1;
        }
        // Имя инстанса с исходящим линком
        const char* name_outInst = bson_iter_utf8(&iter_outInst, NULL);


        if (!bson_iter_init_find(&iter_outInst, &bson_link, "type")) {
            fprintf(stderr, "Error: not found node \"type\" in link\n");
            debug_print_bson("Function \"add_links2instance\" main.c on error", &bson_link);
            return -1;
        }
        // Имя инстанса с исходящим линком
        const char* type_outInst = bson_iter_utf8(&iter_outInst, NULL);


        // Если это имя равно имени данного инстанса, то добавим связь в список входящих связей данного модуля
        if(!strncmp(name_outInst, module_instance_name, 100) && !strncmp(type_outInst, "queue", 100))
        {
            if(!was_out_links)
            {
                bson_append_array_begin (module_instance, "out_links", -1, &b_arr);
                was_out_links = true;
            }
            char buffer [3];
            sprintf(buffer,"%i",lc);
            bson_append_document (&b_arr, buffer, -1, &bson_link);
            lc++;
        }
    }
    if(was_out_links)
    {
        bson_append_array_end (module_instance, &b_arr);
    }

    return 0;
}



int start_instance(bson_t* bson_configuration, bson_t* modules, const char* instance_name)
{
    bson_iter_t iter_modules;
    if(!bson_iter_init (&iter_modules, modules))
    {
        fprintf(stderr, "Error: error create iterator for modules\n");
        return -1;
    }

    bson_t * module_instance = NULL;
    const uint8_t *buf = NULL;
    uint32_t buf_len = 0;
    const char* module_instance_name = NULL;

    // Бежим по списку инстансов в конфигурации
    while(bson_iter_next(&iter_modules))
    {
        if(!BSON_ITER_HOLDS_DOCUMENT(&iter_modules))
        {
            fprintf(stderr, "Error: Not document\n");
            //continue;
            return -1;
        }
        bson_iter_document(&iter_modules, &buf_len, &buf);

        module_instance = bson_new_from_data (buf, buf_len);

        // Get Module Type
        bson_iter_t iter_instance_name;
        if (!bson_iter_init_find(&iter_instance_name, module_instance, "instance")) {
            printf("Not found property \"instance\" in module_instance");
            return -1;
        }
        if (!BSON_ITER_HOLDS_UTF8(&iter_instance_name)) {
            printf("Property \"instance\" in module_instance not UTF8 type");
            return -1;
        }

        module_instance_name = bson_iter_utf8(&iter_instance_name, NULL);

        if(!strncmp(module_instance_name, instance_name, 100))
        {
            // Мы нашли в конфигурации инстанс с нужным именем
            break;
        }
        module_instance = NULL;
    }

    if(module_instance==NULL)
    {
        printf("Not found instance \"%s\"\n", instance_name);
        return -1;
    }

    if(add_links2instance(bson_configuration, module_instance, module_instance_name)!=0)
    {
        printf("Error add_links2instance \"%s\"\n", instance_name);
        return -1;
    }

    //debug_print_bson("Function \"start_instance\" main.c", module_instance);


    // Get Module Type
    bson_iter_t iter_module_type;
    if (!bson_iter_init_find(&iter_module_type, module_instance, "name")) {
        printf("Not found property \"name\" in bson_module");
        return -1;
    }
    if (!BSON_ITER_HOLDS_UTF8(&iter_module_type)) {
        printf("Property \"name\" in bson_module not UTF8 type");
        return -1;
    }

    const char* module_type = bson_iter_utf8(&iter_module_type, NULL);
    char so_name[PATH_MAX] = "";
    strcat(so_name, g_appPath);
    strcat(so_name, "../modules/");
    strcat(so_name, module_type);
    strcat(so_name, "/lib");
    strcat(so_name, module_type);
    strcat(so_name, ".so");

    //fprintf(stdout, "so_name=%s\n", so_name);

    void *handle; // Указатель на загруженную dll
    char *error;
    handle = dlopen(so_name, RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    dlerror();    // Clear any existing error


    // Сварганим имя функции и вызовем ее
    char f_create_name[PATH_MAX] = "";
    strcat(f_create_name, replace(module_type, '-', "_")); // FREE NEED
    strcat(f_create_name, "_create");
    create_f create = dlsym(handle, f_create_name);
    if ((error = dlerror()) != NULL)  {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }
    dlerror();    // Clear any existing error

    // Call function Create instance (.so)
    module_t* module = (*create)(handle);

    module->module_type = malloc(strlen(module_type)+1);
    strcpy(module->module_type, module_type);


    char f_init_name[64] = "";
    strcat(f_init_name, replace(module_type, '-', "_")); // FREE NEED
    strcat(f_init_name, "_init");
    init_f init = dlsym(handle, f_init_name);
    if ((error = dlerror()) != NULL)  {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }
    dlerror();    // Clear any existing error

    // Call function init


    //void* bdata = malloc(module_instance->len);
    //memcpy(bdata, bson_get_data(module_instance), module_instance->len);

    bson_t * copy_module_instance;
    copy_module_instance= bson_copy (module_instance);

    //printf("main before debug_print_bson\n");
    //debug_print_bson("Function \"start_instance\" main.c", forprint);
    //printf("main after debug_print_bson\n");

    if ((*init)(module, bson_get_data(copy_module_instance), copy_module_instance->len) != 0)
        return -1;



    char f_start_name[64] = "";
    strcat(f_start_name, replace(module_type, '-', "_")); // FREE NEED
    strcat(f_start_name, "_start");
    start_f start = dlsym(handle, f_start_name);
    if ((error = dlerror()) != NULL)  {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }
    dlerror();    // Clear any existing error

    // Call function start
    if ((*start)(module) != 0)
        return -1;

//bson_destroy(copy_module_instance);

    bson_destroy(module_instance);

    bson_append_int32 (&instances, instance_name, -1, (int32_t)module);

    return 0;
}



int stop_instance(const char* instance_name)
{
    // Ищем по имени инстанса, ссылку на него
    bson_iter_t iter_l;
    if (!bson_iter_init_find(&iter_l, &instances, instance_name))
    {
        fprintf(stderr, "funcrion:stop_instance, call:bson_iter_init_find, Error: not found node \"%s\" in instances\n", instance_name);
        return -1;
    }

    module_t* module;
    module = (module_t*)bson_iter_int32(&iter_l);

    //printf("val poiner to module = 0x%08X\n", module);

    // Указатель на загруженную dll
    void *handle = module->dll_handle;
    char *error;

    char f_delete_name[64] = "";
    strcat(f_delete_name, replace(module->module_type, '-', "_")); // FREE NEED
    strcat(f_delete_name, "_delete");
    delete_f f_delete = dlsym(handle, f_delete_name);
    if ((error = dlerror()) != NULL)
    {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }
    dlerror();    // Clear any existing error

    // Call function delete
    (*f_delete)(module);

    dlclose(handle);

    return 0;
}


int main(int argc, char *argv[]) {

    mlockall(MCL_CURRENT|MCL_FUTURE);

    setvbuf(stdout, NULL, _IONBF, 0);

    if(argc < 3)
    {
        printf( "usage: %s configuration_name configuration_version [instance_name ...]\n", argv[0] );
        return -1;
    }

    if (realpath(argv[0], g_appPath) == 0) {
        fprintf (stderr, "realpath failed: %s\n", strerror (errno));
        exit(EXIT_FAILURE);
    }
    size_t len = strlen(g_appPath);
    if (len) {
        g_appPath[len-6] = 0;
    }


    const char* configuration_name = argv[1];
    const char* configuration_version = argv[2];
    //fprintf(stderr, "configuration_name=%s configuration_version=%s\n", configuration_name, configuration_version);


    // Инициализируем переменную (bson объект), в котором будем хранить ссылки на созданные инстансы
    bson_init(&instances);

    // Временное решение - грузить конфигурацию из файла, а не из базы
    //bson_t* bson_configuration = get_bson_from_file();

    bson_t* bson_configuration = get_bson_from_db(configuration_name, configuration_version);
    if(bson_configuration==NULL)
    {
        fprintf(stderr, "Not found configuration \"%s\" with version \"%s\"\n", configuration_name, configuration_version);
        return -1;
    }

	bson_iter_t iter_m;
    if (!bson_iter_init_find(&iter_m, bson_configuration, "modules")) {
		fprintf(stderr, "Error: not found node \"modules\" in configuration\n");
		return -1;
	}

    const uint8_t *module_buf = NULL;
    uint32_t module_buf_len = 0;
	bson_t bson_modules;
    bson_iter_array(&iter_m, &module_buf_len, &module_buf);
    bson_init_static(&bson_modules, module_buf, module_buf_len);


    if(argc<4)
    {
        // Конкретных инстансов в параметрах запуска хость процесса не указано
        // Будем запускать все инстансы найденные в конфигурации

        bson_iter_t iter_modules;
        if(!bson_iter_init (&iter_modules, &bson_modules))
        {
            fprintf(stderr, "Error: error create iterator for modules\n");
            return -1;
        }

        bson_t * module_instance = NULL;
        const uint8_t *buf = NULL;
        uint32_t buf_len = 0;
        const char* module_instance_name;

        // Бежим по списку инстансов в конфигурации
        while(bson_iter_next(&iter_modules))
        {
            if(!BSON_ITER_HOLDS_DOCUMENT(&iter_modules))
            {
                fprintf(stderr, "Error: Not document\n");
                //continue;
                return -1;
            }
            bson_iter_document(&iter_modules, &buf_len, &buf);

            module_instance = bson_new_from_data (buf, buf_len);

            // Get Instance Name
            bson_iter_t iter_instance_name;
            if (!bson_iter_init_find(&iter_instance_name, module_instance, "instance")) {
                printf("Not found property \"instance\" in module_instance");
                return -1;
            }
            if (!BSON_ITER_HOLDS_UTF8(&iter_instance_name)) {
                printf("Property \"instance\" in module_instance not UTF8 type");
                return -1;
            }

            module_instance_name = bson_iter_utf8(&iter_instance_name, NULL);

            bson_t * copy_bson_configuration = bson_copy (bson_configuration);
            bson_t * copy_bson_modules = bson_copy (&bson_modules);

            start_instance(copy_bson_configuration, copy_bson_modules, module_instance_name);
        }
    }
    else
    {
        // В апраметрах запуска хость процесса указаны конкретные имена инстансов
        // Будем запускать только их
        int im;
        for(im=3; im<argc; im++)
        {
            //printf("instance=%s\n", argv[im]);
            bson_t * copy_bson_configuration = bson_copy (bson_configuration);
            bson_t * copy_bson_modules = bson_copy (&bson_modules);

            start_instance(copy_bson_configuration, copy_bson_modules, argv[im]);

            //bson_destroy(copy_bson_configuration);
        }

        bson_destroy(bson_configuration);
    }


    printf("\nPress ENTER for exit\n\n");
	getchar();


    if(argc<4)
    {
        // Конкретных инстансов в параметрах запуска хость процесса не указано
        // Будем освобождать все инстансы найденные в конфигурации

        bson_iter_t iter_modules;
        if(!bson_iter_init (&iter_modules, &bson_modules))
        {
            fprintf(stderr, "Error: error create iterator for modules\n");
            return -1;
        }

        bson_t * module_instance = NULL;
        const uint8_t *buf = NULL;
        uint32_t buf_len = 0;
        const char* module_instance_name;

        // Бежим по списку инстансов в конфигурации
        while(bson_iter_next(&iter_modules))
        {
            if(!BSON_ITER_HOLDS_DOCUMENT(&iter_modules))
            {
                fprintf(stderr, "Error: Not document\n");
                //continue;
                return -1;
            }
            bson_iter_document(&iter_modules, &buf_len, &buf);

            module_instance = bson_new_from_data (buf, buf_len);

            // Get Instance Name
            bson_iter_t iter_instance_name;
            if (!bson_iter_init_find(&iter_instance_name, module_instance, "instance")) {
                printf("Not found property \"instance\" in module_instance");
                return -1;
            }
            if (!BSON_ITER_HOLDS_UTF8(&iter_instance_name)) {
                printf("Property \"instance\" in module_instance not UTF8 type");
                return -1;
            }

            module_instance_name = bson_iter_utf8(&iter_instance_name, NULL);

            stop_instance(module_instance_name);
        }
    }
    else
    {
        int im;
        for(im=3; im<argc; im++)
        {
            stop_instance(argv[im]);
        }
    }

    exit(EXIT_SUCCESS);
}

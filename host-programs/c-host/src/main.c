#include <dlfcn.h>
#include <stdio.h>
#include <bcon.h>
#include <bson.h>
#include <mongoc.h>
#include "module-functions.h"

// Хранилище указателей на загруженные пускачем инстансы модулей
bson_t instances;

bson_t* get_bson_from_file() {
	bson_json_reader_t *reader;
	bson_error_t error;
	const char *filename;
	bson_t *b_out;
	int i;
	int b;

	b_out = bson_new();

	filename = "/root/testConfig.json";


    if (!(reader = bson_json_reader_new_from_file(filename, &error))) {
        fprintf(stderr, "Failed to open \"%s\": %s\n", filename,
                error.message);
        return NULL ;
    }


	/*
	 * Convert incoming document to JSON.
	 */
	if ((b = bson_json_reader_read(reader, b_out, &error))) {
		if (b < 0) {
			fprintf(stderr, "Error in json parsing:\n%s\n", error.message);
			abort();
		}

		//fwrite(bson_get_data(b_out), 1, b_out->len, stdout);

		bson_json_reader_destroy(reader);

		return b_out;
	}
	return NULL ;
}

bson_t* get_bson_from_db()
{
    bson_t *result;
    mongoc_collection_t *collection;
    mongoc_client_t *client;
    mongoc_cursor_t *cursor;
    const bson_t *item;
    bson_error_t error;
    bson_oid_t oid;
    bson_t *query;
    //bson_t *doc;
    char *str;
    bool r;

    /* get a handle to our collection */
    client = mongoc_client_new ("mongodb://localhost:27017");
    collection = mongoc_client_get_collection (client, "test", "configuration");

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
    query = BCON_NEW ("name", BCON_UTF8 ("New Schema"), "version", BCON_UTF8("1"));

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
int add_links2instance(bson_t* bson_configuration, bson_t * module_instance)
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

    bson_append_array_begin (module_instance, "inputs", -1, &b_arr);
    while(bson_iter_next(&iter_links))
    {
        if(!BSON_ITER_HOLDS_DOCUMENT(&iter_links))
        {
            fprintf(stderr, "Error: Not link document\n");
            continue;
        }
        bson_iter_document(&iter_links, &link_buf_len, &link_buf);

        bson_init_static(&bson_link, link_buf, link_buf_len);
        //debug_print_bson(&bson_link);

        char buffer [3];
        sprintf(buffer,"%i",lc);

        bson_append_document (&b_arr, buffer, -1, &bson_link);

        lc++;
    }
    bson_append_array_end (module_instance, &b_arr);
    //debug_print_bson(module_instance);
    return 0;
}


int start_instance(bson_t* bson_configuration, bson_t* modules, char* instance_name)
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

        const char* module_instance_name = bson_iter_utf8(&iter_instance_name, NULL);

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

    if(add_links2instance(bson_configuration, module_instance)!=0)
    {
        printf("Error add_links2instance \"%s\"\n", instance_name);
        return -1;
    }

    debug_print_bson(module_instance);


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
    char so_name[64] = "";
    strcat(so_name, "lib");
    strcat(so_name, module_type);
    strcat(so_name, ".so");

    fprintf(stdout, "so_name=%s\n", so_name);

    void *handle; // Указатель на загруженную dll
    char *error;
    handle = dlopen(so_name, RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    dlerror();    // Clear any existing error


    // Сварганим имя функции и вызовем ее
    char f_create_name[64] = "";
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

    module->module_type = malloc(strlen(module_type));
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
    if ((*init)(module, buf, buf_len) != 0)
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


    bson_destroy(module_instance);

    bson_append_int32 (&instances, instance_name, -1, (int32_t)module);

    return 0;
}


int stop_instance(char* instance_name)
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

    return 0;
}



int main(int argc, char *argv[]) {
    if(argc < 2)
    {
        printf( "usage: %s instance_name1, instance_name2, ...\n", argv[0] );
        return -1;
    }

    // Инициализируем переменную (bson объект), в котором будем хранить ссылки на созданные инстансы
    bson_init(&instances);

    // Временное решение - грузить конфигурацию из файла, а не из базы
    //bson_t* bson_configuration = get_bson_from_file();

    bson_t* bson_configuration = get_bson_from_db();

    debug_print_bson(bson_configuration);

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


    int im;
    for(im=1; im<argc; im++)
    {
        printf("instance=%s\n", argv[im]);

        start_instance(bson_configuration, &bson_modules, argv[im]);
    }

    bson_destroy(bson_configuration);


    printf("\nPress ENTER for exit\n\n");
	getchar();

    for(im=1; im<argc; im++)
    {
        stop_instance(argv[im]);
    }

    exit(EXIT_SUCCESS);
}

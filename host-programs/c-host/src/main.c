#include <stdio.h>
#include <bson.h>
#include "c-gy87.h"

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

int start_module(bson_t* bson_configuration, bson_t* modules, char* instance_name)
{
    bson_iter_t iter_modules;
    if(!bson_iter_init (&iter_modules, modules))
    {
        fprintf(stderr, "Error: error create iterator for modules\n");
        return -1;
    }

    const uint8_t *buf = NULL;
    uint32_t buf_len = 0;

    // Вычитывается первый модуль
    if(bson_iter_next(&iter_modules))
    {
        if(!BSON_ITER_HOLDS_DOCUMENT(&iter_modules))
        {
            fprintf(stderr, "Error: Not document\n");
            //continue;
            return -1;
        }
        bson_iter_document(&iter_modules, &buf_len, &buf);
    }
    else{
        return -1;
    }


    bson_t * bson_module = bson_new_from_data (buf, buf_len);
    //debug_print_bson(&bson_module);

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

    bson_append_array_begin (bson_module, "inputs", -1, &b_arr);
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
    bson_append_array_end (bson_module, &b_arr);

    debug_print_bson(bson_module);


    module_GY87_t* module = c_gy87_create();

    if (c_gy87_init(module, buf, buf_len) != 0)
        return -1;

    if (c_gy87_start(module) != 0)
        return -1;


    bson_destroy(bson_module);
}


int main(int argc, char *argv[]) {
    if(argc < 2)
    {
        printf( "usage: %s instance_name1, instance_name2, ...\n", argv[0] );
        return -1;
    }

    bson_t* bson_configuration = get_bson_from_file();

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
        printf("module=%s\n", argv[im]);

        start_module(bson_configuration, &bson_modules, argv[im]);
    }

    bson_destroy(bson_configuration);


	printf("END\n");
	getchar();

	return 0;
}

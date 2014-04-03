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

	if (0 == strcmp(filename, "-")) {
		reader = bson_json_reader_new_from_fd(STDIN_FILENO, false);
	} else {
		if (!(reader = bson_json_reader_new_from_file(filename, &error))) {
			fprintf(stderr, "Failed to open \"%s\": %s\n", filename,
					error.message);
			return NULL ;
		}
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

int main() {
	bson_t* bson1 = get_bson_from_file();

	bson_iter_t iter_m;
	if (!bson_iter_init_find(&iter_m, bson1, "modules")) {
		fprintf(stderr, "Error: not found node \"modules\" in configuration\n");
		return -1;
	}

	const uint8_t *docbuf = NULL;
	uint32_t doclen = 0;
	bson_t bson_modules;
	bson_iter_array(&iter_m, &doclen, &docbuf);
	bson_init_static(&bson_modules, docbuf, doclen);

	bson_iter_t iter_modules;
	if(!bson_iter_init (&iter_modules, &bson_modules))
	{
		fprintf(stderr, "Error: error create iterator for modules\n");
		return -1;
	}

	if(bson_iter_next(&iter_modules))
	{
		if(!BSON_ITER_HOLDS_DOCUMENT(&iter_modules))
		{
			fprintf(stderr, "Error: Not document\n");
			//continue;
			return -1;
		}
		bson_iter_document(&iter_modules, &doclen, &docbuf);
	}
	else{
		return -1;
	}
	bson_destroy(bson1);


	if (c_gy87_init(docbuf, doclen) != 0)
		return -1;

	if (c_gy87_start() != 0)
		return -1;

	printf("END\n");
	getchar();

	return 0;
}

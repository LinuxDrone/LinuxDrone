#include <stdio.h>
#include "../../../include/module.h"

bson_t* get_bson_from_file() {
	bson_json_reader_t *reader;
	bson_error_t error;
	const char *filename;
	bson_t *b_out;
	int i;
	int b;

	b_out = bson_new();

	filename = "/usr/local/linuxdrone/modules/gy87/gy87.def.json";


	if (0 == strcmp(filename, "-")) {
		reader = bson_json_reader_new_from_fd(STDIN_FILENO, false);
	} else {
		if (!(reader = bson_json_reader_new_from_file(filename, &error))) {
			fprintf(stderr, "Failed to open \"%s\": %s\n", filename,
					error.message);
			return NULL;
		}
	}

	/*
	 * Convert each incoming document to JSON and print to stdout.
	 */
	if ((b = bson_json_reader_read(reader, b_out, &error))) {
		if (b < 0) {
			fprintf(stderr, "Error in json parsing:\n%s\n", error.message);
			abort();
		}

		fwrite(bson_get_data(b_out), 1, b_out->len, stdout);
		bson_json_reader_destroy(reader);

		//bson_destroy(b_out);

		return b_out;
	}
	return NULL ;
}

void work(Reason4callback reason) {
	int s = 10;
	for (;;) {
		int res = 0; //InvokeManagedCode (s++, 5);
		rt_task_sleep(1000000000);
		printf("step %i\n", res);
	}
}

int main() {
	register_business_callback(&work);

	bson_t bson, foo, bar, baz;

	bson_init(&bson);
	/*
	 BCON_APPEND(&bson,
	 "foo", "{",
	 "bar", "{",
	 "baz", "[", BCON_INT32(1), BCON_INT32(2), BCON_INT32(3), "]",
	 "}",
	 "}"
	 );*/

	if (!bson_append_utf8(&bson, "name", -1, "test_module", -1)) {
		printf("Don't succeed add property \"name\"\n");
	}

	bson_append_document_begin(&bson, "foo", -1, &foo);

	bson_append_document_begin(&foo, "bar", -1, &bar);
	bson_append_array_begin(&bar, "baz", -1, &baz);
	bson_append_int32(&baz, "0", -1, 1);
	bson_append_int32(&baz, "1", -1, 2);
	bson_append_int32(&baz, "2", -1, 3);
	bson_append_array_end(&bar, &baz);
	bson_append_document_end(&foo, &bar);
	bson_append_document_end(&bson, &foo);

	bson_t* bson1 = get_bson_from_file();

	init(bson_get_data(bson1), bson1->len);

	bson_destroy(&bson);
	bson_destroy(bson1);

	start();

	getchar();
}

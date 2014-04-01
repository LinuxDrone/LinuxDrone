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

	filename = "/root/testConfig.json";


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

	bson_t* bson1 = get_bson_from_file();




	init(bson_get_data(bson1), bson1->len);

	bson_destroy(bson1);

	if(start()!=0)
		return -1;

	getchar();

	return 0;
}

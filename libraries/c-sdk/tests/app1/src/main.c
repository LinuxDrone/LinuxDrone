#include <stdio.h>
#include "../../../include/module.h"

void work(Reason4callback reason)
{
	int s=10;
    for (;;) {
    	int res = 0;//InvokeManagedCode (s++, 5);
    	rt_task_sleep(1000000000);
    	printf("step %i\n", res);
    }
}

int main ()
{
	register_business_callback(&work);

	bson_t bson, foo, bar, baz;;
	bson_init(&bson);
/*
	BCON_APPEND(&bson,
	            "foo", "{",
	               "bar", "{",
	                  "baz", "[", BCON_INT32(1), BCON_INT32(2), BCON_INT32(3), "]",
	               "}",
	            "}"
	         );*/

	if(!bson_append_utf8 (&bson, "name", -1, "test_module", -1))
	{
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

	init(bson_get_data(&bson), bson.len);

	bson_destroy(&bson);

	start();

	getchar();
}

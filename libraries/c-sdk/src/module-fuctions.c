#include "../include/module-fuctions.h"
#include <native/queue.h>
#include <native/heap.h>
#include <native/event.h>
#include <stdio.h>

#define TASK_MODE  0  /* No flags */
#define TASK_STKSZ 0  /* Stack size (use default one) */

void debug_print_bson(bson_t* bson) {
	char* str = bson_as_json(bson, NULL);
	fprintf(stdout, "%s\n", str);
	bson_free(str);
}

int init(module_t* module, const uint8_t * data, uint32_t length) {
	bson_t bson;
	bson_init_static(&bson, data, length);
	debug_print_bson(&bson);

	// Instance name
	bson_iter_t iter;
	if (!bson_iter_init_find(&iter, &bson, "instance")) {
		printf("Not found property \"instance\"");
		return -1;
	}
	if (!BSON_ITER_HOLDS_UTF8(&iter)) {
		printf("Property \"instance\" not UTF8 type");
		return -1;
	}
	module->instance_name = bson_iter_utf8(&iter, NULL);
	fprintf(stdout, "instance name=%s\n", module->instance_name);

	// Task Priority
	if (!bson_iter_init_find(&iter, &bson, "Task Priority")) {
		printf("Not found property \"Task Priority\"");
		return -1;
	}
	if (!BSON_ITER_HOLDS_INT32(&iter)) {
		printf("Property \"Task Priority\" not INT32 type");
		return -1;
	}
	module->task_priority = bson_iter_int32(&iter);
	fprintf(stdout, "Task Priority=%i\n", module->task_priority);

	// Task Period
	if (!bson_iter_init_find(&iter, &bson, "Task Period")) {
		printf("Not found property \"Task Period\"");
		return -1;
	}
	if (!BSON_ITER_HOLDS_INT32(&iter)) {
		printf("Property \"Task Period\" not INT32 type");
		return -1;
	}
	module->queue_timeout = rt_timer_ns2ticks(bson_iter_int32(&iter) * 1000);
	fprintf(stdout, "queue_timeout=%i\n", module->queue_timeout);

	// Start required xenomai services
	int err = create_xenomai_services(module);
	if (err != 0) {
		printf("Error create xenomai services\n");
		return err;
	}

}

Reason4callback get_input_data(module_t* module) {
	char buf[256];

	int res_read = rt_queue_read(&module->in_queue, buf, 256, module->queue_timeout);
	if(res_read>0)
		printf("ofiget' ne vstat'\n");

	return obtained_data;
}

int start(module_t* module) {
	if (module == NULL) {
		printf("param \"module\" is null\n");
		return -1;
	}

	if (module->func == NULL) {
		printf("module->func required\n");
		return -1;
	}

	int err = rt_task_start(&module->task_main, module->func, NULL);
	if (err != 0)
		printf("Error start task\n");

	return err;
}

int create_xenomai_services(module_t* module) {
	// Create input queue
	char name_queue[64] = "";
	strcat(name_queue, module->instance_name);
	strcat(name_queue, "_queue");
	int queue_poolsize = 200;
	int err = rt_queue_create(&module->in_queue, name_queue, queue_poolsize, 10, Q_FIFO);
	if (err != 0) {
		fprintf(stdout, "Error create queue \"%s\"\n", name_queue);
		return err;
	}

	// Create shared memory
	char name_shmem[64] = "";
	strcat(name_shmem, module->instance_name);
	strcat(name_shmem, "_shmem");
	err = rt_heap_create(&module->shmem, name_shmem, 200, H_SINGLE | H_PRIO);
	if (err != 0) {
		fprintf(stdout, "Error create shared memory \"%s\"\n", name_shmem);
		return err;
	}

	// Create event service
	char name_eflags[64] = "";
	strcat(name_eflags, module->instance_name);
	strcat(name_eflags, "_flags");
	err = rt_event_create(&module->eflags, name_eflags, ULONG_MAX, EV_PRIO);
	if (err != 0) {
		fprintf(stdout, "Error create event service \"%s\"\n", name_eflags);
		return err;
	}

	// Create task
	char name_task_main[64] = "";
	strcat(name_task_main, module->instance_name);
	strcat(name_task_main, "_task");
	err = rt_task_create(&module->task_main, name_task_main, TASK_STKSZ, module->task_priority,
			TASK_MODE);
	if (err != 0) {
		fprintf(stdout, "Error create work task \"%s\"\n", name_task_main);
		return err;
	}
}


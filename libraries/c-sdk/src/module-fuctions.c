#include "../include/module-fuctions.h"
#include <native/queue.h>
#include <native/heap.h>
#include <native/event.h>
#include <stdio.h>

#define TASK_MODE  0  /* No flags */
#define TASK_STKSZ 0  /* Stack size (use default one) */

#define SHMEM_WRITER_MASK	0x7FFFFFFF

#define SHMEM_HEAP_SIZE		300
#define SHMEM_BLOCK1_SIZE	200

void debug_print_bson(bson_t* bson) {
	char* str = bson_as_json(bson, NULL);
	fprintf(stdout, "%s\n", str);
	bson_free(str);
}

void print_task_error(int err) {
	switch (err) {
	case -EINVAL:
		printf("is returned if task is not a task descriptor./n");
		break;

	case -EIDRM:
		printf("is returned if task is a deleted task descriptor.\n");
		break;

	case -EBUSY:
		printf("is returned if task is already started.\n");
		break;

	case -EPERM:
		printf(
				"is returned if this service was called from an asynchronous context.\n");
		break;

	default:
		printf("Unknown task error: %i.\n", err);
	}

}

int init(module_t* module, const uint8_t * data, uint32_t length) {
	bson_t bson;
	bson_init_static(&bson, data, length);
	debug_print_bson(&bson);

	/**
	 * Instance name
	 */
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

	/**
	 * Task Priority
	 */
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

	/**
	 * Main task Period
	 */
	if (!bson_iter_init_find(&iter, &bson, "Task Period")) {
		printf("Not found property \"Task Period\"");
		return -1;
	}
	if (!BSON_ITER_HOLDS_INT32(&iter)) {
		printf("Property \"Main task Period\" not INT32 type");
		return -1;
	}
	module->queue_timeout = rt_timer_ns2ticks(bson_iter_int32(&iter) * 1000);
	fprintf(stdout, "queue_timeout=%i\n", module->queue_timeout);

	/**
	 * Transfer task Period
	 */
	if (!bson_iter_init_find(&iter, &bson, "Transfer task period")) {
		printf("Not found property \"Transfer task period\"");
		return -1;
	}
	if (!BSON_ITER_HOLDS_INT32(&iter)) {
		printf("Property \"Main task Period\" not INT32 type");
		return -1;
	}
	module->transfer_task_period = rt_timer_ns2ticks(
			bson_iter_int32(&iter) * 1000);
	fprintf(stdout, "transfer_task_period=%i\n", module->transfer_task_period);

	/**
	 * Create required xenomai services
	 */
	int err = create_xenomai_services(module);
	if (err != 0) {
		printf("Error create xenomai services\n");
		return err;
	}

	module->obj1_data = data;
	module->obj1_length = length;

	return 0;
}

Reason4callback get_input_data(module_t* module) {
	char buf[256];

	int res_read = rt_queue_read(&module->in_queue, buf, 256,
			module->queue_timeout);
	if (res_read > 0)
		printf("ofiget' ne vstat'\n");

	return obtained_data;
}


void write_shmem(module_t* module, const char* data, int datalen) {
	unsigned long after_mask;
	int res = rt_event_clear(&module->eflags, ~SHMEM_WRITER_MASK, &after_mask);
	if (res != 0) {
		printf("error write_shmem: rt_event_clear1\n");
		return;
	}

	printf("was mask = 0x%08X\n", after_mask);

	/**
	 * \~russian Подождем, пока все читающие потоки выйдут из функции чтения и обнулят счетчик читающих потоков
	 */
	res = rt_event_wait(&module->eflags, SHMEM_WRITER_MASK, &after_mask,
	EV_ALL,
	TM_INFINITE);
	if (res != 0) {
		printf("error write_shmem: rt_event_wait\n");
		return;
	}

	memcpy(module->shmem, data, datalen);

	res = rt_event_signal(&module->eflags, ~SHMEM_WRITER_MASK);
	if (res != 0) {
		printf("error write_shmem: rt_event_signal\n");
		return;
	}
}


void task_transmit_body(void *cookie) {
	module_t* module = cookie;
	int i = 0;
	for (;;) {
		rt_task_sleep(module->transfer_task_period);

		write_shmem(module, module->obj1_data, module->obj1_length);

		printf("task_transmit %i\n", i++);
	}
}

int start(module_t* module) {
	if (module == NULL) {
		printf("Function \"start\". Param \"module\" is null\n");
		return -1;
	}

	if (module->func == NULL) {
		printf("module->func for main task required\n");
		return -1;
	}

	int err = rt_heap_alloc(&module->h_shmem,
	SHMEM_BLOCK1_SIZE,
	TM_INFINITE, &module->shmem);
	if (err != 0)
		printf("Error rt_heap_alloc for block1\n");

	err = rt_task_start(&module->task_main, module->func, NULL);
	if (err != 0)
		printf("Error start main task\n");

	err = rt_task_start(&module->task_transmit, &task_transmit_body, module);
	if (err != 0) {
		printf("Error start transmit task. err=%i\n", err);
		print_task_error(err);
	}

	return err;
}

int stop(module_t* module) {

	int res = rt_heap_free(&module->h_shmem, module->shmem);

	return res;
}

int create_xenomai_services(module_t* module) {
	// Create input queue
	char name_queue[64] = "";
	strcat(name_queue, module->instance_name);
	strcat(name_queue, "_queue");
	int queue_poolsize = 200;
	int err = rt_queue_create(&module->in_queue, name_queue, queue_poolsize, 10,
	Q_FIFO);
	if (err != 0) {
		fprintf(stdout, "Error create queue \"%s\"\n", name_queue);
		return err;
	}

	// Create shared memory
	char name_shmem[64] = "";
	strcat(name_shmem, module->instance_name);
	strcat(name_shmem, "_shmem");
	err = rt_heap_create(&module->h_shmem, name_shmem, SHMEM_HEAP_SIZE, H_PRIO);
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

	// Create main task
	char name_task_main[64] = "";
	strcat(name_task_main, module->instance_name);
	strcat(name_task_main, "_task");
	err = rt_task_create(&module->task_main, name_task_main, TASK_STKSZ,
			module->task_priority,
			TASK_MODE);
	if (err != 0) {
		fprintf(stdout, "Error create work task \"%s\"\n", name_task_main);
		return err;
	}

	// Create transmit task
	char name_tr_task_main[64] = "";
	strcat(name_tr_task_main, module->instance_name);
	strcat(name_tr_task_main, "_tr_task");
	err = rt_task_create(&module->task_transmit, name_tr_task_main, TASK_STKSZ,
			99,
			TASK_MODE);
	if (err != 0) {
		fprintf(stdout, "Error create transmit task \"%s\"\n",
				name_tr_task_main);
		return err;
	}
	return 0;
}

void read_shmem(module_t* module, char* data, int* datalen) {

}

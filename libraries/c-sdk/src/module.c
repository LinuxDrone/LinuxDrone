#include "../include/module.h"
#include <native/queue.h>
#include <native/heap.h>
#include <native/event.h>
#include <stdio.h>

#define TASK_MODE  0  /* No flags */
#define TASK_STKSZ 0  /* Stack size (use default one) */

/**
 * \~english Main thread
 * \~russian
 */
RT_TASK task_main;

/**
 * \~english input queue
 */
RT_QUEUE in_queue;
RTIME queue_timeout;

/**
 * \~english Shared memory
 */
RT_HEAP shmem;

RT_EVENT eflags;

/**
 * \~english Name Main thread
 * \~russian
 */
const char* instance_name;

int32_t task_priority;

/**
 * \~english Transfer thread
 * \~russian
 */
RT_TASK task_transfer;

static t_callback_business m_business_callback = NULL;

void register_business_callback(t_callback_business callback) {
	if (callback != NULL)
		m_business_callback = callback;
}

void debug_print_bson(bson_t* bson) {
	char* str = bson_as_json(bson, NULL);
	fprintf(stdout, "%s\n", str);
	bson_free(str);
}

int init(const uint8_t * data, uint32_t length) {
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
	instance_name = bson_iter_utf8(&iter, NULL);
	fprintf(stdout, "instance name=%s\n", instance_name);

	// Task Priority
	if (!bson_iter_init_find(&iter, &bson, "Task Priority")) {
		printf("Not found property \"Task Priority\"");
		return -1;
	}
	if (!BSON_ITER_HOLDS_INT32(&iter)) {
		printf("Property \"Task Priority\" not INT32 type");
		return -1;
	}
	task_priority = bson_iter_int32(&iter);
	fprintf(stdout, "Task Priority=%i\n", task_priority);

	// Task Period
	if (!bson_iter_init_find(&iter, &bson, "Task Period")) {
		printf("Not found property \"Task Period\"");
		return -1;
	}
	if (!BSON_ITER_HOLDS_INT32(&iter)) {
		printf("Property \"Task Period\" not INT32 type");
		return -1;
	}
	queue_timeout = bson_iter_int32(&iter) * 1000000;
	fprintf(stdout, "queue_timeout=%i\n", task_priority);

	// Start required xenomai services
	int err = create_xenomai_services();
	if (err != 0) {
		printf("Error create xenomai services\n");
		return err;
	}
}

Reason4callback get_input_data() {
	char name_queue[256];

	//int was_read = rt_queue_read(&in_queue, name_queue, 256,
		//	rt_timer_ns2tsc(queue_timeout));

	return obtained_data;
}

int start(t_cycle_function func) {
	if (m_business_callback == NULL) {
		printf("m_business_callback is NULL\n");
		return -1;
	}

	int err = rt_task_start(&task_main, func, NULL);
	if (err != 0)
		printf("m_business_callback is NULL\n");

	return err;
}

int create_xenomai_services() {
	// Create input queue
	char name_queue[64] = "";
	strcat(name_queue, instance_name);
	strcat(name_queue, "_queue");
	int err = rt_queue_create(&in_queue, name_queue, 200, 10, Q_FIFO);
	if (err != 0) {
		fprintf(stdout, "Error create queue \"%s\"\n", name_queue);
		return err;
	}

	// Create shared memory
	char name_shmem[64] = "";
	strcat(name_shmem, instance_name);
	strcat(name_shmem, "_shmem");
	err = rt_heap_create(&shmem, name_shmem, 200, H_SINGLE | H_PRIO);
	if (err != 0) {
		fprintf(stdout, "Error create shared memory \"%s\"\n", name_shmem);
		return err;
	}

	// Create event service
	char name_eflags[64] = "";
	strcat(name_eflags, instance_name);
	strcat(name_eflags, "_flags");
	err = rt_event_create(&eflags, name_eflags, ULONG_MAX, EV_PRIO);
	if (err != 0) {
		fprintf(stdout, "Error create event service \"%s\"\n", name_eflags);
		return err;
	}

	// Create task
	char name_task_main[64] = "";
	strcat(name_task_main, instance_name);
	strcat(name_task_main, "_task");
	err = rt_task_create(&task_main, name_task_main, TASK_STKSZ, task_priority,
			TASK_MODE);
	if (err != 0) {
		fprintf(stdout, "Error create work task \"%s\"\n", name_task_main);
		return err;
	}
}


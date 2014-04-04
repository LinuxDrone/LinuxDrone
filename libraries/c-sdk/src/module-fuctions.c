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

int init_publisher_set(shmem_publisher_set_t * pset, char* instance_name,
		char* out_name) {
	// Create shared memory
	char name_shmem[64] = "";
	strcat(name_shmem, instance_name);
	strcat(name_shmem, out_name);
	strcat(name_shmem, "_shmem");
	int err = rt_heap_create(&pset->h_shmem, name_shmem, pset->shmem_len,
			H_SHARED | H_PRIO);
	if (err != 0) {
		fprintf(stdout, "Error create shared memory \"%s\"\n", name_shmem);
		return err;
	}

	// Alloc shared memory block
	err = rt_heap_alloc(&pset->h_shmem, 0, TM_INFINITE, &pset->shmem);
	if (err != 0)
		printf("Error rt_heap_alloc for block1 err=%i\n", err);
	memset(pset->shmem, 0, pset->shmem_len);

	// Create event service
	char name_eflags[64] = "";
	strcat(name_eflags, instance_name);
	strcat(name_eflags, out_name);
	strcat(name_eflags, "_flags");
	err = rt_event_create(&pset->eflags, name_eflags, ULONG_MAX, EV_PRIO);
	if (err != 0) {
		fprintf(stdout, "Error create event service \"%s\"\n", name_eflags);
		return err;
	}

	// Create mutex for read shared memory
	char name_rmutex[64] = "";
	strcat(name_rmutex, instance_name);
	strcat(name_rmutex, out_name);
	strcat(name_rmutex, "_rmutex");
	err = rt_mutex_create(&pset->mutex_read_shmem, name_rmutex);
	if (err != 0) {
		fprintf(stdout, "Error create mutex_read_shmem \"%s\"\n", name_rmutex);
		return err;
	}
	return 0;
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
	module->transmit_task_period = rt_timer_ns2ticks(
			bson_iter_int32(&iter) * 1000);
	fprintf(stdout, "transmit_task_period=%i\n", module->transmit_task_period);

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

void write_shmem(shmem_publisher_set_t* set, const char* data,
		unsigned short datalen) {
	unsigned long after_mask;
	int res = rt_event_clear(&set->eflags, ~SHMEM_WRITER_MASK, &after_mask);
	if (res != 0) {
		printf("error write_shmem: rt_event_clear1\n");
		return;
	}

	//printf("was mask = 0x%08X\n", after_mask);

	/**
	 * \~russian Подождем, пока все читающие потоки выйдут из функции чтения и обнулят счетчик читающих потоков
	 */
	res = rt_event_wait(&set->eflags, SHMEM_WRITER_MASK, &after_mask,
	EV_ALL,
	TM_INFINITE);
	if (res != 0) {
		printf("error write_shmem: rt_event_wait\n");
		return;
	}

	// В первые два байта сохраняем длину блока
	*((unsigned short*) set->shmem) = datalen;

	// в буфер (со смещением в два байта) копируем блок данных
	memcpy(set->shmem + sizeof(unsigned short), data, datalen);
	//printf("datalen write_shmem: %i\n", datalen);
	res = rt_event_signal(&set->eflags, ~SHMEM_WRITER_MASK);
	if (res != 0) {
		printf("error write_shmem: rt_event_signal\n");
		return;
	}
}

void read_shmem(shmem_publisher_set_t* set, void* data, unsigned short* datalen) {
	unsigned long after_mask;
	/**
	 * \~russian Подождем, если пишущий поток выставил флаг, что он занят записью
	 */
	int res = rt_event_wait(&set->eflags, ~SHMEM_WRITER_MASK, &after_mask,
	EV_ALL,
	TM_INFINITE);
	if (res != 0) {
		printf("error read_shmem: rt_event_wait\n");
		return;
	}

	/**
	 * Залочим мьютекс
	 */
	res = rt_mutex_acquire(&set->mutex_read_shmem, TM_INFINITE);
	if (res != 0) {
		printf("error read_shmem: rt_mutex_acquire1\n");
		return;
	}

	/**
	 * Считываем показания счетчика (младших битов флагов)
	 */
	RT_EVENT_INFO info;
	res = rt_event_inquire(&set->eflags, &info);
	if (res != 0) {
		printf("error read_shmem: rt_event_inquire1\n");
		return;
	}
	//printf("read raw mask = 0x%08X\n", info.value);

	// инкрементируем показания счетчика
	unsigned long count = (~(info.value & SHMEM_WRITER_MASK))
			& SHMEM_WRITER_MASK;
	//printf("masked raw mask = 0x%08X\n", count);
	if (count == 0)
		count = 1;
	else
		count = count << 1;

	//printf("clear mask = 0x%08X\n", count);

	// Сбросим флаги в соответствии со значением счетчика
	res = rt_event_clear(&set->eflags, count, &after_mask);
	if (res != 0) {
		printf("error read_shmem: rt_event_clear\n");
		return;
	}

	res = rt_mutex_release(&set->mutex_read_shmem);
	if (res != 0) {
		printf("error read_shmem:  rt_mutex_release1\n");
		return;
	}

	// из первых двух байт считываем блину последующего блока
	unsigned short buflen = *((unsigned short*) set->shmem);
	//printf("buflen read_shmem: %i\n", buflen);

	if (buflen != 0) {
		// со смещением в два байта читаем следующий блок данных
		memcpy(data, set->shmem + sizeof(unsigned short), buflen);
	}
	*datalen = buflen;

	/**
	 * Залочим мьютекс
	 */
	res = rt_mutex_acquire(&set->mutex_read_shmem, TM_INFINITE);
	if (res != 0) {
		printf("error read_shmem: rt_mutex_acquire2\n");
		return;
	}

	/**
	 * Считываем показания счетчика (младших битов флагов)
	 */
	res = rt_event_inquire(&set->eflags, &info);
	if (res != 0) {
		printf("error read_shmem: rt_event_inquire1\n");
		return;
	}
	// декрементируем показания счетчика
	count = (~(info.value & SHMEM_WRITER_MASK));

	count = count ^ (count >> 1);

	//printf("set mask = 0x%08X\n", count);

	// Установим флаги в соответствии со значением счетчика
	res = rt_event_signal(&set->eflags, count);
	if (res != 0) {
		printf("error read_shmem: rt_event_signal\n");
		return;
	}

	res = rt_mutex_release(&set->mutex_read_shmem);
	if (res != 0) {
		printf("error read_shmem:  rt_mutex_release2\n");
		return;
	}

}

ReciveResult get_input_data(void* p_module) {
	module_t* module = p_module;
	char buf[256];

	int res_read = rt_queue_read(&module->in_queue, buf, 256,
			module->queue_timeout);
	if (res_read > 0)
		printf("ofiget' ne vstat'\n");

	unsigned short retlen;

	read_shmem(module->shmem_publishers[0], buf, &retlen);
	//printf("retlen=%i\n", retlen);

	bson_t bson;
	if (retlen > 0) {
		bson_init_static(&bson, buf, retlen);
		debug_print_bson(&bson);
	}

	return obtained_data;
}

void task_transmit_body(void *cookie) {
	module_t* module = cookie;
	int cycle = 0;
	for (;;) {
		rt_task_sleep(module->transmit_task_period);

		bson_t bson;
		bson_init_static(&bson, module->obj1_data, module->obj1_length);
		//debug_print_bson(&bson);

		bson_iter_t iter;
		if (!bson_iter_init_find(&iter, &bson, "Task Priority")) {
			printf("not found Task Priority\n");
		}

		bson_iter_overwrite_int32(&iter, cycle++);

		int i=0;
		shmem_publisher_set_t* set = module->shmem_publishers[i];
		while(set)
		{
			write_shmem(set, bson_get_data(&bson), bson.len);
			set = module->shmem_publishers[++i];
		}

		//printf("task_transmit cycle %i\n", cycle++);
	}
}

int start(void* p_module) {
	module_t* module = p_module;

	if (module == NULL) {
		printf("Function \"start\". Param \"module\" is null\n");
		return -1;
	}

	if (module->func == NULL) {
		printf("module->func for main task required\n");
		return -1;
	}

	int err = rt_task_start(&module->task_main, module->func, p_module);
	if (err != 0)
		printf("Error start main task\n");

	err = rt_task_start(&module->task_transmit, &task_transmit_body, p_module);
	if (err != 0) {
		printf("Error start transmit task. err=%i\n", err);
		print_task_error(err);
	}

	return err;
}

int stop(void* module) {

	//int res = rt_heap_free(&module->h_shmem, module->shmem);

	return 0;
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


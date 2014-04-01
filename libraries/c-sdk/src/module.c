#include "../include/module.h"
#include <native/queue.h>
#include <native/heap.h>
#include <native/event.h>
#include <stdio.h>

#define TASK_PRIO  99 /* Highest RT priority */
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


/**
 * \~english Transfer thread
 * \~russian
 */
RT_TASK task_transfer;


static t_callback_business m_business_callback = NULL;


void register_business_callback(t_callback_business callback)
{
	if(callback!=NULL)
		m_business_callback = callback;
}

int init(const uint8_t * data, uint32_t length)
{
	bson_t bson;

	bson_init_static (&bson, data, length);

	char* str = bson_as_json(&bson, NULL);
	fprintf(stdout, "%s\n", str);
	bson_free(str);

	bson_iter_t iter;
	if(!bson_iter_init_find (&iter, &bson, "type"))
	{
		printf("Not found property \"name\"");
		return -1;
	}

	if(!BSON_ITER_HOLDS_UTF8(&iter))
	{
		printf("Property \"name\" not UTF8 type");
		return -1;
	}


	instance_name=bson_iter_utf8(&iter, NULL);
	fprintf(stdout, "module name=%s\n", instance_name);

}

void task_main_body (void *cookie)
{
    for (;;) {
    	(*m_business_callback) (obtained_data);
    }
}

int start()
{
	if(!m_business_callback)
	{
		printf("m_business_callback is NULL\n");
		return -1;
	}

	// Create input queue
	char name_queue[64] = "";
	strcat(name_queue, instance_name);
	strcat(name_queue, "_queue");
	int err = rt_queue_create	(&in_queue, name_queue, 200, 10, Q_FIFO);
	if(err!=0)
	{
		fprintf(stdout, "Error create queue \"%s\"\n", name_queue);
		return err;
	}

	// Create shared memory
	char name_shmem[64] = "";
	strcat(name_shmem, instance_name);
	strcat(name_shmem, "_shmem");
	err = rt_heap_create(&shmem, name_shmem, 200, H_SINGLE | H_PRIO);
	if(err!=0)
	{
		fprintf(stdout, "Error create shared memory \"%s\"\n", name_shmem);
		return err;
	}

	// Create event service
	char name_eflags[64] = "";
	strcat(name_eflags, instance_name);
	strcat(name_eflags, "_flags");
	err = rt_event_create(&eflags, name_eflags, ULONG_MAX, EV_PRIO);
	if(err!=0)
	{
		fprintf(stdout, "Error create event service \"%s\"\n", name_eflags);
		return err;
	}



	// Create task
	char name_task_main[64] = "";
	strcat(name_task_main, instance_name);
	strcat(name_task_main, "_task");

	fprintf(stdout, "task name=%s\n", name_task_main);

    err = rt_task_create(&task_main,
    					name_task_main,
                         TASK_STKSZ,
                         TASK_PRIO,
                         TASK_MODE);
    if (!err)
        err = rt_task_start(&task_main, &task_main_body, NULL);

    return err;
}

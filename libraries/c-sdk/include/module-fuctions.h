#ifndef MODULE_FUNCTIONS_H_
#define MODULE_FUNCTIONS_H_

#include <native/task.h>
#include <bson.h>
#include <native/queue.h>
#include <native/heap.h>
#include <native/event.h>
#include <native/mutex.h>

/**
 * \enum Reason4callback
 * \~russian ������� ������ ������� ������-�������
 */
typedef enum {
	/**
	 * \~russian
	 */
	timeout,

	/**
	 * \~russian
	 */
	obtained_data
} Reason4callback;

typedef void (*t_cycle_function)(void *cookie);

typedef struct
{
	/**
	 * \~english Main task
	 * \~russian
	 */
	RT_TASK task_main;

	/**
	 * \~english Transmit task
	 * \~russian
	 */
	RT_TASK task_transmit;
	RTIME transmit_task_period;

	/**
	 * \~english input queue
	 */
	RT_QUEUE in_queue;
	RTIME queue_timeout;

	/**
	 * \~english Shared memory
	 */
	RT_HEAP h_shmem;
	void* shmem;
	int shmem_len;

	RT_EVENT eflags;

	/**
	 * \~english Name Main thread
	 * \~russian
	 */
	const char* instance_name;

	int task_priority;

	RT_MUTEX mutex_read_shmem;

	t_cycle_function func;

	const uint8_t * obj1_data;
	uint32_t obj1_length;

} module_t;


int init(module_t* module, const uint8_t * data, uint32_t length);

int start(module_t* module);

int stop(module_t* module);

Reason4callback get_input_data(module_t* module);

#endif

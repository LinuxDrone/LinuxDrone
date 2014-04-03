#ifndef MODULE_FUNCTIONS_H_
#define MODULE_FUNCTIONS_H_

#include <native/task.h>
#include <bson.h>
#include <native/queue.h>
#include <native/heap.h>
#include <native/event.h>

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
	RTIME transfer_task_period;

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

	t_cycle_function func;

} module_t;


int init(module_t* module, const uint8_t * data, uint32_t length);

int start(module_t* module);

Reason4callback get_input_data(module_t* module);

#endif

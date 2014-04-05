#ifndef MODULE_FUNCTIONS_H_
#define MODULE_FUNCTIONS_H_

#include <native/task.h>
#include <bson.h>
#include <native/queue.h>
#include <native/heap.h>
#include <native/event.h>
#include <native/mutex.h>
#include <native/cond.h>

typedef enum
{
    Empty,
    Writing,
    Transferring,
    Filled
} StatusObj;

typedef int (*p_obj2bson)(void* obj, bson_t* bson);
typedef void (*p_print_obj)(void* obj);

typedef struct {
	RT_HEAP h_shmem;
	void* shmem;
	int shmem_len;

	RT_EVENT eflags;

    /**
     * @brief mutex_read_shmem
     * /~russian мьютех используемый для синхронизации при обмене через разделяемую память,
     * потоков разных модулей
     */
	RT_MUTEX mutex_read_shmem;

    void* obj1;
    StatusObj status_obj1;

    void* obj2;
    StatusObj status_obj2;

    p_obj2bson obj2bson;
    p_obj2bson bson2obj;
    p_print_obj print_obj;

} shmem_publisher_set_t;


/**
 * \enum Reason4callback
 * \~russian
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
} ReceiveResult;

typedef void (t_cycle_function)(void *cookie);
typedef t_cycle_function* p_cycle_function;


typedef struct
{
	const char* instance_name;

	/**
	 * \~english Main task
	 * \~russian
	 */
	RT_TASK task_main;
	int task_priority;


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


    // массив структур предназначенных для передачи объектов в шаред мемори
    shmem_publisher_set_t** shmem_sets;

	p_cycle_function func;

    /**
     * @brief mutex_obj_exchange
     * /~russian мьютех используемый для синхронизации при обмене основного потока с потоком передачи
     * в разделяемую память
     */
    RT_MUTEX mutex_obj_exchange;

    /**
     * @brief obj_cond
     * /~russian для синхронизации при обмене основного потока с потоком передачи
     * в разделяемую память
     */
    RT_COND obj_cond;

    void* input_buf;
    //int size_input_buf;


} module_t;


int init(module_t* module, const uint8_t * data, uint32_t length);

int start(void* module);

int stop(void* module);

ReceiveResult get_input_data(void* module);

#endif

#include "../include/module.h"
#include <stdio.h>

#define TASK_PRIO  99 /* Highest RT priority */
#define TASK_MODE  0  /* No flags */
#define TASK_STKSZ 0  /* Stack size (use default one) */

/**
 * \~english Main thread
 * \~russian Главный поток модуля, в котором выполняется бизнес-функция
 */
RT_TASK task_main;

/**
 * \~english Name Main thread
 * \~russian Имя задачи главного потока модуля
 */
char* name_task_main;


/**
 * \~english Transfer thread
 * \~russian Поток подготовки и передачи bson объектов
 */
RT_TASK task_transfer;


static t_callback_business m_business_callback;


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
	if(!bson_iter_init_find (&iter, &bson, "name"))
	{
		printf("Not found property \"name\"");
		return -1;
	}

	if(!BSON_ITER_HOLDS_UTF8(&iter))
	{
		printf("Property \"name\" not UTF8 type");
		return -1;
	}

}

void task_main_body (void *cookie)
{
    for (;;) {
    	(*m_business_callback) (obtained_data);
    }
}

int start()
{
    int err = rt_task_create(&task_main,
    					name_task_main,
                         TASK_STKSZ,
                         TASK_PRIO,
                         TASK_MODE);
    if (!err)
        err = rt_task_start(&task_main, &task_main_body, NULL);

    return err;
}

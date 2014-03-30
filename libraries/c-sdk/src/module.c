#include "../include/module.h"

#define TASK_PRIO  99 /* Highest RT priority */
#define TASK_MODE  0  /* No flags */
#define TASK_STKSZ 0  /* Stack size (use default one) */

/**
 * \~english Main thread
 * \~russian ������� ����� ������, � ������� ����������� ������-�������
 */
RT_TASK task_main;

/**
 * \~english Name Main thread
 * \~russian ��� ������ �������� ������ ������
 */
char* name_task_main;


/**
 * \~english Transfer thread
 * \~russian ����� ���������� � �������� bson ��������
 */
RT_TASK task_transfer;


static t_callback_business m_business_callback;


void register_business_callback(t_callback_business callback)
{
	if(callback!=NULL)
		m_business_callback = callback;
}

int init()
{

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

#include <stdio.h>
#include <sys/mman.h>
#include <native/task.h>

#include "../include/module.h"

typedef int (*tcallback) (int a, int b);
static tcallback my_callback;

void RegisterCallback (tcallback cb)
{
    my_callback = cb;
}

int InvokeManagedCode (int a, int b)
{
    if (my_callback == NULL){
         printf ("Managed code has not initialized this library yet");
         //abort ();
    }
    return (*my_callback) (a, b);
}



void work(Reason4callback reason)
{
	int s=10;
    for (;;) {
    	int res = InvokeManagedCode (s++, 5);
    	rt_task_sleep(100000000);
    	printf("step %i\n", res);
    }
}


int run ()
{
	register_business_callback(&work);
	init();
	start();
}


#include "../include/websocket_service.h"

/*
extern int count_subscriptions;
// Массив подписок
extern shmem_in_set_t** subscriptions;


// Оформление подписки на данные инстанса
int add_subscription(char* instance_name, char* out_name)
{
    count_subscriptions +=1;
    subscriptions = realloc(subscriptions, sizeof(shmem_in_set_t*)*count_subscriptions);
    shmem_in_set_t* new_remote_shmem = calloc(1, sizeof(shmem_in_set_t));

    new_remote_shmem->name_instance = malloc(strlen(instance_name)+1);
    strcpy(new_remote_shmem->name_instance, instance_name);

    new_remote_shmem->name_outgroup = malloc(strlen(out_name)+1);
    strcpy(new_remote_shmem->name_outgroup, out_name);

    subscriptions[count_subscriptions-1] = new_remote_shmem;

    return 0;
}
*/

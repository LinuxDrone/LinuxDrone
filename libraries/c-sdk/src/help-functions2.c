#include "../include/module-functions.h"


void debug_print_bson(char* where, bson_t* bson) {
    printf("%s\n", where);
    char* str = bson_as_json(bson, NULL);
    fprintf(stdout, "%s\n", str);
    bson_free(str);
    printf("\n");
}

void print_task_start_error(int err) {
    switch (err)
    {
        case -EINVAL:
            printf("is returned if task is not a task descriptor.\n");
            break;
            
        case -EIDRM:
            printf("is returned if task is a deleted task descriptor.\n");
            break;
            
        case -EBUSY:
            printf("is returned if task is already started.\n");
            break;
            
        case -EPERM:
            printf("is returned if this service was called from an asynchronous context.\n");
            break;
            
        default:
            printf("Unknown task error: %i.\n", err);
    }
}

void print_event_wait_error(int err) {
    switch (err)
    {
        case -EINVAL:
            printf("is returned if event is not a event group descriptor.\n");
            break;
            
        case -EIDRM:
            printf("is returned if event is a deleted event group descriptor, including if the deletion occurred while the caller was sleeping on it before the request has been satisfied.\n");
            break;
            
        case -EWOULDBLOCK:
            printf("is returned if timeout is equal to TM_NONBLOCK and the current event mask value does not satisfy the request.\n");
            break;
            
        case -EINTR:
            printf("is returned if rt_task_unblock() has been called for the waiting task before the request has been satisfied.\n");
            break;
            
        case -ETIMEDOUT:
            printf("is returned if the request has not been satisfied within the specified amount of time.\n");
            break;
            
        case -EPERM:
            printf("is returned if this service should block, but was called from a context which cannot sleep (e.g. interrupt, non-realtime context).\n");
            break;
            
        default:
            printf("Unknown event wait error: %i.\n", err);
    }
}

void print_obj_status(int number_obj, StatusObj status) {
    switch (status)
    {
        case Empty:
            printf("Obj%i=Empty\n", number_obj);
            break;
            
        case Writing:
            printf("Obj%i=Writing\n", number_obj);
            break;
            
        case Transferring:
            printf("Obj%i=Transferring\n", number_obj);
            break;
            
        case Filled:
            printf("Obj%i=Filled\n", number_obj);
            break;
            
        default:
            printf("Unknown status of obj: %i.\n", status);
    }
}

/* return a new string with every instance of ch replaced by repl */
char *replace(const char *s, char ch, const char *repl) {
    int count = 0;
    const char *t;
    for(t=s; *t; t++)
        count += (*t == ch);

    size_t rlen = strlen(repl);
    char *res = malloc(strlen(s) + (rlen-1)*count + 1);
    char *ptr = res;
    for(t=s; *t; t++) {
        if(*t == ch) {
            memcpy(ptr, repl, rlen);
            ptr += rlen;
        } else {
            *ptr++ = *t;
        }
    }
    *ptr = 0;
    return res;
}

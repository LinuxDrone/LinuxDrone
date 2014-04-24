#include "../include/module-functions.h"


bson_t* get_bson_from_file() {
    bson_json_reader_t *reader;
    bson_error_t error;
    const char *filename;
    bson_t *b_out;
    int i;
    int b;

    b_out = bson_new();

    filename = "/root/testConfig.json";


    if (!(reader = bson_json_reader_new_from_file(filename, &error))) {
        fprintf(stderr, "Failed to open \"%s\": %s\n", filename,
                error.message);
        return NULL ;
    }


    /*
     * Convert incoming document to JSON.
     */
    if ((b = bson_json_reader_read(reader, b_out, &error))) {
        if (b < 0) {
            fprintf(stderr, "Error in json parsing:\n%s\n", error.message);
            abort();
        }
        //fwrite(bson_get_data(b_out), 1, b_out->len, stdout);

        bson_json_reader_destroy(reader);

        return b_out;
    }
    return NULL ;
}

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

// rt_heap_create
void print_heap_create_error(int err) {
    switch (err)
    {
        case -EEXIST:
            printf("is returned if the name is already in use by some registered object.\n");
            break;

        case -EINVAL:
            printf("is returned if heapsize is null, greater than the system limit, or name is null or empty for a mappable heap.\n");
            break;

        case -ENOMEM:
            printf("is returned if not enough system memory is available to create or register the heap. Additionally, and if H_MAPPABLE has been passed in mode, errors while mapping the block pool in the caller's address space might beget this return code too.\n");
            break;

        case -EPERM:
            printf("is returned if this service was called from an invalid context.\n");
            break;

        case -ENOSYS:
            printf("is returned if mode specifies H_MAPPABLE, but the real-time support in user-space is unavailable.\n");
            break;

        default:
            printf("Unknown heap_create: %i.\n", err);
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

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
    fprintf(stderr, "%s\n", where);
    char* str = bson_as_json(bson, NULL);
    fprintf(stdout, "%s\n", str);
    bson_free(str);
    fprintf(stderr, "\n");
}


void print_task_start_error(int err) {
    switch (err)
    {
        case -EINVAL:
            fprintf(stderr, "is returned if task is not a task descriptor.\n");
            break;
            
        case -EIDRM:
            fprintf(stderr, "is returned if task is a deleted task descriptor.\n");
            break;
            
        case -EBUSY:
            fprintf(stderr, "is returned if task is already started.\n");
            break;
            
        case -EPERM:
            fprintf(stderr, "is returned if this service was called from an asynchronous context.\n");
            break;
            
        default:
            fprintf(stderr, "Unknown task error: %i.\n", err);
    }
}


void print_task_receive_error(int err) {
    switch (err)
    {
        case -ENOBUFS:
            fprintf(stderr, "is returned if mcb_r does not point at a message area large enough to collect the remote task's message.\n");
            break;

        case -EWOULDBLOCK:
            fprintf(stderr, "is returned if timeout is equal to TM_NONBLOCK and no remote task is currently waiting for sending a message to the caller.\n");
            break;

        case -ETIMEDOUT:
            fprintf(stderr, "is returned if no message was received within the timeout.\n");
            break;

        case -EINTR:
            fprintf(stderr, "is returned if rt_task_unblock() has been called for the caller before any message was available.\n");
            break;

        case -EPERM:
            fprintf(stderr, "is returned if this service was called from a context which cannot sleep (e.g. interrupt, non-realtime or scheduler locked).\n");
            break;

        default:
            fprintf(stderr, "Unknown task_receive error: %i.\n", err);
    }
}


void print_task_reply_error(int err) {
    switch (err)
    {
        case -EINVAL:
            fprintf(stderr, "is returned if flowid is invalid.\n");
            break;

        case -ENXIO:
            fprintf(stderr, "is returned if flowid does not match the expected identifier returned from the latest call of the current task to rt_task_receive(), or if the remote task stopped waiting for the reply in the meantime (e.g. the client could have been deleted or forcibly unblocked).\n");
            break;

        case -EPERM:
            fprintf(stderr, "is returned if this service was called from an invalid context (e.g. interrupt, or non-primary).\n");
            break;

        default:
            fprintf(stderr, "Unknown task_reply error: %i.\n", err);
    }
}


void print_rt_pipe_write_error(int err) {
    switch (err)
    {
        case -EINVAL:
            fprintf(stderr, "is returned if pipe is not a pipe descriptor.\n");
            break;

        case -ENOMEM:
            fprintf(stderr, "is returned if not enough buffer space is available to complete the operation.\n");
            break;

        case -EIDRM:
            fprintf(stderr, "is returned if pipe is a closed pipe descriptor.\n");
            break;

        case -ENODEV:
            fprintf(stderr, "are returned if pipe is scrambled.\n");
            break;

        case -EBADF:
            fprintf(stderr, "are returned if pipe is scrambled.\n");
            break;

        default:
            fprintf(stderr, "Unknown pipe error: %i.\n", err);
    }
}


void print_rt_heap_bind_error(int err) {
    switch (err)
    {
        case -EFAULT:
            fprintf(stderr, "is returned if heap or name is referencing invalid memory.\n");
            break;

        case -EINTR:
            fprintf(stderr, "is returned if rt_task_unblock() has been called for the waiting task before the retrieval has completed.\n");
            break;

        case -EWOULDBLOCK:
            fprintf(stderr, "is returned if timeout is equal to TM_NONBLOCK and the searched object is not registered on entry.\n");
            break;

        case -ETIMEDOUT:
            fprintf(stderr, "is returned if the object cannot be retrieved within the specified amount of time.\n");
            break;

        case -EPERM:
            fprintf(stderr, "is returned if this service should block, but was called from a context which cannot sleep (e.g. interrupt, non-realtime context).\n");
            break;

        case -ENOENT:
            fprintf(stderr, "is returned if the special file /dev/rtheap (character-mode, major 10, minor 254) is not available from the filesystem. This device is needed to map the shared heap memory into the caller's address space. udev-based systems should not need manual creation of such device entry.\n");
            break;

        default:
            fprintf(stderr, "Unknown rt_heap_bind error: %i.\n", err);
    }
}


void print_rt_mutex_bind_error(int err) {
    switch (err)
    {
        case -EFAULT:
            fprintf(stderr, "is returned if mutex or name is referencing invalid memory.\n");
            break;

        case -EINTR:
            fprintf(stderr, "is returned if rt_task_unblock() has been called for the waiting task before the retrieval has completed.\n");
            break;

        case -EWOULDBLOCK:
            fprintf(stderr, "is returned if timeout is equal to TM_NONBLOCK and the searched object is not registered on entry.\n");
            break;

        case -ETIMEDOUT:
            fprintf(stderr, "is returned if the object cannot be retrieved within the specified amount of time.\n");
            break;

        case -EPERM:
            fprintf(stderr, "is returned if this service should block, but was called from a context which cannot sleep (e.g. interrupt, non-realtime context).\n");
            break;

        default:
            fprintf(stderr, "Unknown rt_mutex_bind error: %i.\n", err);
    }
}


void print_rt_mutex_acquire(int err) {
    switch (err)
    {
        case -EINVAL:
            fprintf(stderr, "is returned if mutex is not a mutex descriptor.\n");
            break;

        case -EIDRM:
            fprintf(stderr, "is returned if mutex is a deleted mutex descriptor, including if the deletion occurred while the caller was sleeping on it.\n");
            break;

        case -EWOULDBLOCK:
            fprintf(stderr, "is returned if timeout is equal to TM_NONBLOCK and the mutex is not immediately available.\n");
            break;

        case -EINTR:
            fprintf(stderr, "is returned if rt_task_unblock() has been called for the waiting task before the mutex has become available.\n");
            break;

        case -ETIMEDOUT:
            fprintf(stderr, "is returned if the mutex cannot be made available to the calling task within the specified amount of time.\n");
            break;

        case -EPERM:
            fprintf(stderr, "is returned if this service was called from a context which cannot be given the ownership of the mutex (e.g. interrupt, non-realtime context).\n");
            break;

        default:
            fprintf(stderr, "Unknown rt_mutex_acquire error: %i.\n", err);
    }
}


void print_task_bind_error(int err) {
    switch (err)
    {
        case -EFAULT:
            fprintf(stderr, "is returned if task or name is referencing invalid memory.\n");
            break;

        case -EINTR:
            fprintf(stderr, "is returned if rt_task_unblock() has been called for the waiting task before the retrieval has completed.\n");
            break;

        case -EWOULDBLOCK:
            fprintf(stderr, "is returned if timeout is equal to TM_NONBLOCK and the searched object is not registered on entry.\n");
            break;

        case -ETIMEDOUT:
            fprintf(stderr, "is returned if the object cannot be retrieved within the specified amount of time.\n");
            break;

        case -EPERM:
            fprintf(stderr, "is returned if this service should block, but was called from a context which cannot sleep (e.g. interrupt, non-realtime context).\n");
            break;

        default:
            fprintf(stderr, "Unknown task_bind error: %i.\n", err);
    }
}


void print_task_send_error(int err) {
    switch (err)
    {
        case -ENOBUFS:
            fprintf(stderr, "is returned if mcb_r does not point at a message area large enough to collect the remote task's reply. This includes the case where mcb_r is NULL on entry albeit the remote task attempts to send a reply message.\n");
            break;

        case -EWOULDBLOCK:
            fprintf(stderr, "is returned if timeout is equal to TM_NONBLOCK and task is not currently blocked on the rt_task_receive() service.\n");
            break;

        case -EIDRM:
            fprintf(stderr, "is returned if task has been deleted while waiting for a reply.\n");
            break;

        case -EINTR:
            fprintf(stderr, "is returned if rt_task_unblock() has been called for the caller before any reply was available.\n");
            break;

        case -EPERM:
            fprintf(stderr, "is returned if this service should block, but was called from a context which cannot sleep (e.g. interrupt, non-realtime context).\n");
            break;

        case -ESRCH:
            fprintf(stderr, "is returned if task cannot be found (when called from user-space only).\n");
            break;

        default:
            fprintf(stderr, "Unknown task_send error: %i.\n", err);
    }
}


void print_rt_event_bind_error(int err) {
    switch (err)
    {
        case -EFAULT:
            fprintf(stderr, "is returned if event or name is referencing invalid memory.\n");
            break;

        case -EINTR:
            fprintf(stderr, "is returned if rt_task_unblock() has been called for the waiting task before the retrieval has completed.\n");
            break;

        case -EWOULDBLOCK:
            fprintf(stderr, "is returned if timeout is equal to TM_NONBLOCK and the searched object is not registered on entry.\n");
            break;

        case -ETIMEDOUT:
            fprintf(stderr, "is returned if the object cannot be retrieved within the specified amount of time.\n");
            break;

        case -EPERM:
            fprintf(stderr, "is returned if this service should block, but was called from a context which cannot sleep (e.g. interrupt, non-realtime context).\n");
            break;

        default:
            fprintf(stderr, "Unknown rt_event_bind error: %i.\n", err);
    }
}


void print_heap_create_error(int err) {
    switch (err)
    {
        case -EEXIST:
            fprintf(stderr, "is returned if the name is already in use by some registered object.\n");
            break;

        case -EINVAL:
            fprintf(stderr, "is returned if heapsize is null, greater than the system limit, or name is null or empty for a mappable heap.\n");
            break;

        case -ENOMEM:
            fprintf(stderr, "is returned if not enough system memory is available to create or register the heap. Additionally, and if H_MAPPABLE has been passed in mode, errors while mapping the block pool in the caller's address space might beget this return code too.\n");
            break;

        case -EPERM:
            fprintf(stderr, "is returned if this service was called from an invalid context.\n");
            break;

        case -ENOSYS:
            fprintf(stderr, "is returned if mode specifies H_MAPPABLE, but the real-time support in user-space is unavailable.\n");
            break;

        default:
            fprintf(stderr, "Unknown heap_create: %i.\n", err);
    }
}


void print_event_wait_error(int err) {
    switch (err)
    {
        case -EINVAL:
//fprintf(stderr, "is returned if event is not a event group descriptor.\n");
            break;
            
        case -EIDRM:
            fprintf(stderr, "is returned if event is a deleted event group descriptor, including if the deletion occurred while the caller was sleeping on it before the request has been satisfied.\n");
            break;
            
        case -EWOULDBLOCK:
            fprintf(stderr, "is returned if timeout is equal to TM_NONBLOCK and the current event mask value does not satisfy the request.\n");
            break;
            
        case -EINTR:
            fprintf(stderr, "is returned if rt_task_unblock() has been called for the waiting task before the request has been satisfied.\n");
            break;
            
        case -ETIMEDOUT:
            fprintf(stderr, "is returned if the request has not been satisfied within the specified amount of time.\n");
            break;
            
        case -EPERM:
            fprintf(stderr, "is returned if this service should block, but was called from a context which cannot sleep (e.g. interrupt, non-realtime context).\n");
            break;
            
        default:
            fprintf(stderr, "Unknown event wait error: %i.\n", err);
    }
}


void print_obj_status(int number_obj, StatusObj status) {
    switch (status)
    {
        case Empty:
            fprintf(stderr, "Obj%i=Empty\n", number_obj);
            break;
            
        case Writing:
            fprintf(stderr, "Obj%i=Writing\n", number_obj);
            break;
            
        case Transferring:
            fprintf(stderr, "Obj%i=Transferring\n", number_obj);
            break;

        case Transferred2Queue:
            fprintf(stderr, "Obj%i=Transferred2Queue\n", number_obj);
            break;

        case Filled:
            fprintf(stderr, "Obj%i=Filled\n", number_obj);
            break;
            
        default:
            fprintf(stderr, "Unknown status of obj: %i.\n", status);
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


/**
 * @brief remove_element Remove element from array
 * @param array
 * @param index
 * @param array_length
 */
void remove_element(void** array, int index, int array_length)
{
   int i;
   for(i = index; i < array_length - 1; i++) array[i] = array[i + 1];
}


TypeFieldObj convert_port_type_str2type(const char* str_type)
{
    if(strcmp(str_type, "char")==0)
        return field_char;

    if(strcmp(str_type, "short")==0)
        return field_short;

    if(strcmp(str_type, "int")==0)
        return field_int;

    if(strcmp(str_type, "long")==0)
        return field_long;

    if(strcmp(str_type, "long long")==0)
        return field_long_long;

    if(strcmp(str_type, "float")==0)
        return field_float;

    if(strcmp(str_type, "double")==0)
        return field_double;

    if(strcmp(str_type, "const char*")==0)
        return field_const_char;

    if(strcmp(str_type, "bool")==0)
        return field_bool;

    return -1;
}

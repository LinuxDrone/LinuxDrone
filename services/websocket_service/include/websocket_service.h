#pragma once


#include <bcon.h>
#include <bson.h>


typedef struct
{
    int out1;
    int out2;
} subscription_t;


typedef struct
{
    int len_subscription;
    subscription_t** subscription;
} connection_t;

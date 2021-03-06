//===-- main.cpp ------------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// C includes
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

pthread_t g_thread_1 = NULL;
pthread_t g_thread_2 = NULL;
pthread_t g_thread_3 = NULL;

char *g_char_ptr = NULL;

void
do_bad_thing_with_location(unsigned index, char *char_ptr, char new_val)
{
    unsigned what = new_val;
    printf("new value written to array(%p) and index(%u) = %u\n", char_ptr, index, what);
    char_ptr[index] = new_val;
}

uint32_t access_pool (uint32_t flag = 0);

uint32_t
access_pool (uint32_t flag)
{
    static pthread_mutex_t g_access_mutex = PTHREAD_MUTEX_INITIALIZER;
    static unsigned idx = 0; // Well-behaving thread only writes into indexs from 0..6.
    if (flag == 0)
        ::pthread_mutex_lock (&g_access_mutex);

    // idx valid range is [0, 6].
    if (idx > 6)
        idx = 0;

    if (flag != 0) {
        // Write into a forbidden area.
        do_bad_thing_with_location(7, g_char_ptr, 99);
    }

    unsigned index = idx++;

    if (flag == 0)
        ::pthread_mutex_unlock (&g_access_mutex);
    return g_char_ptr[index];
}

void *
thread_func (void *arg)
{
    uint32_t thread_index = *((uint32_t *)arg);
    printf ("%s (thread index = %u) startng...\n", __FUNCTION__, thread_index);

    uint32_t count = 0;
    uint32_t val;
    while (count++ < 15)
    {
        // random micro second sleep from zero to 3 seconds
        int usec = ::rand() % 3000000;
        printf ("%s (thread = %u) doing a usleep (%d)...\n", __FUNCTION__, thread_index, usec);
        ::usleep (usec);
        
        if (count < 7)
            val = access_pool ();
        else
            val = access_pool (1);
                
        printf ("%s (thread = %u) after usleep access_pool returns %d (count=%d)...\n", __FUNCTION__, thread_index, val, count);
    }
    printf ("%s (thread index = %u) exiting...\n", __FUNCTION__, thread_index);
    return NULL;
}


int main (int argc, char const *argv[])
{
    int err;
    void *thread_result = NULL;
    uint32_t thread_index_1 = 1;
    uint32_t thread_index_2 = 2;
    uint32_t thread_index_3 = 3;

    g_char_ptr = (char *)malloc (10);
    for (int i = 0; i < 10; ++i)
        *g_char_ptr = 0;

    // Create 3 threads
    err = ::pthread_create (&g_thread_1, NULL, thread_func, &thread_index_1);
    err = ::pthread_create (&g_thread_2, NULL, thread_func, &thread_index_2);
    err = ::pthread_create (&g_thread_3, NULL, thread_func, &thread_index_3);

    struct {
        int a;
        int b;
        int c;
    } MyAggregateDataType;

    printf ("Before turning all three threads loose...\n"); // Set break point at this line.

    // Join all of our threads
    err = ::pthread_join (g_thread_1, &thread_result);
    err = ::pthread_join (g_thread_2, &thread_result);
    err = ::pthread_join (g_thread_3, &thread_result);

    return 0;
}

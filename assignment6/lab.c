#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>

// This file contains the definition of MAX_ITERATIONS, among other things.
#include "samples.h"

#include <string.h> //////////////////////////////////////////////////////////////////////


// Delay in nanoseconds (1 millisecond)
#define DELAY 1000000

// Number of samples that do_work() handles
#define PROCESSING_INTERVAL  256

// Could be a local variable, but you may need it as a static variable
// here when you modify this file according to the lab instructions.
static int sample_buffer[PROCESSING_INTERVAL];
static int sample_buffer_dowork[PROCESSING_INTERVAL];

int sample_task_FINISHED = 0;

sem_t full_buffer_sem; //////////////////////////////////////////////////////////////////////////////
pthread_mutex_t mutex; //////////////////////////////////////////////////////////////////////////////

struct timespec firsttime;

void do_work(int *samples)
{
        int i;

        //  A busy loop. (In a real system this would do something
        //  more interesting such as an FFT or a particle filter,
        //  etc...)
        volatile int dummy; // A compiler warning is ok here
        for(i=0; i < 20000000;i++){
                dummy=i;
        }

        // Write out the samples.
        for(i=0; i < PROCESSING_INTERVAL; i++){
                write_sample(0,samples[i]);
        }

}

void* sample_task(void* unused)
{
    int channel = 0;
    struct timespec current;
    int i;
    int samplectr = 0;
    current = firsttime;

    // ensure that we have real time priority set //////////////////////////////////////////////////////////
    struct sched_param sp;
    sp.sched_priority = 10;
    if(pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp))
    {
      fprintf(stderr,"WARNING: Failed to set sample_task to realtime priority\n");
    }

    for(i=0; i < MAX_ITERATIONS; i++)
    {
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &current, NULL);

        sample_buffer[samplectr] = read_sample(channel);
        samplectr++;
        if(samplectr == PROCESSING_INTERVAL)
        {
            samplectr = 0;
            memcpy(sample_buffer_dowork, sample_buffer, sizeof(sample_buffer));
            sem_post(&full_buffer_sem); //////////////////////////////////////////////////////////////
        }

        // Increment current time point
        current.tv_nsec +=  DELAY;
        if(current.tv_nsec >= 1000000000)
        {
            current.tv_nsec -= 1000000000;
            current.tv_sec++;
        }
    }

    sample_task_FINISHED = 1;

    return NULL;
}

void* dowork_task(void* unused) /////////////////////////////////////////////////////////////
{
    // ensure that we have real time priority set //////////////////////////////////////////////////////////
    struct sched_param sp;
    sp.sched_priority = 5;
    if(pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp))
    {
      fprintf(stderr,"WARNING: Failed to set sample_task to realtime priority\n");
    }

    while (!sample_task_FINISHED)
    {
        sem_wait(&full_buffer_sem);
        do_work(sample_buffer_dowork);
    }

    return NULL;
}

int main(int argc,char **argv)
{
    sem_init(&full_buffer_sem, 0, 0); ///////////////////////////////////////////////////////////////
    pthread_mutex_init(&mutex, NULL); ////////////////////////////////////////////////////////////////

    // ensure that all memory that we allocate is locked, to prevent swapping: ////////////////////////////////
    if(mlockall(MCL_FUTURE|MCL_CURRENT))
    {
        fprintf(stderr, "WARNING: Could not lock memory with mlockall()\n");
    }

    pthread_t sample_task_handle;
    pthread_attr_t sample_task_attr;

    pthread_t dowork_task_handle;
    pthread_attr_t dowork_task_attr;

    clock_gettime(CLOCK_MONOTONIC, &firsttime);

    // Start the sampling at an even multiple of a second (to make
    // the sample times easy to analyze by hand if necessary)
    firsttime.tv_sec+=2;
    firsttime.tv_nsec = 0;
    printf("Starting sampling at about t+2 seconds\n");

    samples_init(&firsttime);

    if(pthread_attr_init(&sample_task_attr)){
          perror("pthread_attr_init sample_task_attr");
    }
    // Set default stacksize to 64 KiB (should be plenty)
    if(pthread_attr_setstacksize(&sample_task_attr, 65536)){
          perror("pthread_attr_setstacksize() sample_task_attr");
    }

    if(pthread_attr_init(&dowork_task_attr)){
          perror("pthread_attr_init dowork_task_attr");
    }
    // Set default stacksize to 64 KiB (should be plenty)
    if(pthread_attr_setstacksize(&dowork_task_attr, 65536)){
          perror("pthread_attr_setstacksize() dowork_task_attr");
    }

    // create threads:
    pthread_create(&sample_task_handle, &sample_task_attr, sample_task, NULL);
    pthread_create(&dowork_task_handle, &dowork_task_attr, dowork_task, NULL);

    // join threads
    pthread_join(sample_task_handle, NULL);
    pthread_join(dowork_task_handle, NULL);

    // Dump output data which will be used by the analyze.m script
    dump_outdata();
    dump_sample_times();

    return 0;
}

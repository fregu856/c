/* A clock program

Two tasks are used, clock_task for updating a clock, and
set_task for setting the clock

*/

/* standard library includes */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "si_ui.h"
#include "display.h"
#include "clock.h"

/* clock variables */

/* functions operating on the clock */

/* increment_time: increments the current time with
   one second */
void increment_time(void)
{
    /* reserve clock variables */
    pthread_mutex_lock(&Mutex);

    /* increment time */
    Seconds++;
    if (Seconds > 59)
    {
        Seconds = 0;
        Minutes++;
        if (Minutes > 59)
        {
            Minutes = 0;
            Hours++;
            if (Hours > 23)
            {
                Hours = 0;
            }
        }
    }

    /* release clock variables */
    pthread_mutex_unlock(&Mutex);
}

/* get_time: read time from common clock variables */
void get_time(int *hours, int *minutes, int *seconds)
{
    /* reserve clock variables */
    pthread_mutex_lock(&Mutex);

    /* read values */
    *hours = Hours;
    *minutes = Minutes;
    *seconds = Seconds;

    /* release clock variables */
    pthread_mutex_unlock(&Mutex);
}

/* time_from_set_time_message: extract time from set time
   message from user interface */
void time_from_set_time_message(char message[], int *hours, int *minutes, int *seconds)
{
    sscanf(message,"set %d %d %d", hours, minutes, seconds);
}

/* time_ok: returns nonzero if hours, minutes and seconds represents a valid time */
int time_ok(int hours, int minutes, int seconds)
{
    return hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59 &&
        seconds >= 0 && seconds <= 59;
}

/* tasks */

/* clock_thread: clock thread */
void* clock_thread(void *unused)
{
    /* local copies of the current time */
    int hours, minutes, seconds;

    /* infinite loop */
    while (1)
    {
        /* read and display current time */
        get_time(&hours, &minutes, &seconds);
        display_time(hours, minutes, seconds);

        /* increment time */
        increment_time();

        /* wait one second */
        usleep(1000000);
    }
}

/* alarm_thread: alarm thread */
void* alarm_thread(void *unused)
{
    /* infinite loop */
    while (1)
    {

    }
}

/* GUI_thread: reads messages from the user interface, and
   sets the clock, or exits the program */
void* GUI_thread(void *unused)
{
    /* message array */
    char message[SI_UI_MAX_MESSAGE_SIZE];

    /* time read from user interface */
    int hours, minutes, seconds;

    /* set GUI size */
    si_ui_set_size(400, 200);

    while(1)
    {
        /* read a message */
        si_ui_receive(message);
        /* check if it is a set message */
        if (strncmp(message, "set", 3) == 0)
        {
            time_from_set_message(message, &hours, &minutes, &seconds);
            if (time_ok(hours, minutes, seconds))
            {
                set_time(hours, minutes, seconds);
            }
            else
            {
                si_ui_show_error("Illegal value for hours, minutes or seconds");
            }
        }
        /* check if it is an exit message */
        else if (strcmp(message, "exit") == 0)
        {
            exit(0);
        }
        /* not a legal message */
        else
        {
            si_ui_show_error("unexpected message type");
        }
    }
}

/* main */
int main(void)
{
    /* initialise UI channel */
    si_ui_init();

    /* initialise variables */
    init_clock();

    /* create tasks */
    pthread_t clock_thread_handle;
    pthread_t GUI_thread_handle;

    pthread_create(&clock_thread_handle, NULL, clock_thread, 0);
    pthread_create(&GUI_thread_handle, NULL, GUI_thread, 0);

    pthread_join(clock_thread_handle, NULL);
    pthread_join(GUI_thread_handle, NULL);
    /* will never be here! */
    return 0;
}

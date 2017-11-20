/* Alarm Clock

*/

/* standard library includes */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "si_ui.h"

#include "clock.h"
#include "display.h"


/* time_ok: returns nonzero if hours, minutes and seconds represents a valid time */
int time_ok(int hours, int minutes, int seconds)
{
    return hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59 &&
        seconds >= 0 && seconds <= 59;
}

/* tasks */

/* clock_task: clock task */
void *clock_thread(void *unused)
{
    /* local copies of the current time */
    int hours, minutes, seconds;

    /* infinite loop */
    while (1)
    {
        /* read and display current time */
        get_time(&hours, &minutes, &seconds);
	compare_time(hours, minutes, seconds);
        display_time(hours, minutes, seconds);
        /* increment time */
        increment_time();

        /* wait 1 seconds */
        usleep(1000000);
    }
}

/* set_task: reads messages from the user interface, and
   sets the clock, or exits the program */
void * set_thread(void *unused)
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
            time_from_set_time_message(message, &hours, &minutes, &seconds);
            if (time_ok(hours, minutes, seconds))
            {
                clock_set_time(hours, minutes, seconds);
            }
            else
            {
                si_ui_show_error("Illegal value for hours, minutes or seconds");
            }
        }
	/* check if its a alarm message */
	else if (strncmp(message, "alarm", 3) == 0)
	{
	    time_from_set_alarm_time_message(message, &hours, &minutes, &seconds);
	    if (time_ok(hours, minutes, seconds))
            {
                alarm_clock_set_time(hours, minutes, seconds);
		display_alarm_time(hours, minutes, seconds);
		alarm_clock_enable();
            }
            else
            {
                si_ui_show_error("Illegal value for hours, minutes or seconds");
            }
	}
	/* check if an alarm should be acnowledged */
	else if (strcmp(message, "reset") == 0)
	{
	    erase_alarm_time();
	    alarm_clock_disable();
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

/* alarm_thread: alarm task for when the alarm is activated */
void *alarm_thread(void *unused)
{
    while(1)
    {
	wait_for_active();
	display_alarm_text();
	usleep(1500000);    	
    }
}

/* main */
int main(void)
{
    /* initialise UI channel */
    si_ui_init();

    /* initialise variables */
    clock_init();
    display_init();

    /* create tasks */
    pthread_t clock_thread_handle;
    pthread_t set_thread_handle;

    pthread_create(&clock_thread_handle, NULL, clock_thread, 0);
    pthread_create(&set_thread_handle, NULL, set_thread, 0);

    pthread_join(clock_thread_handle, NULL);
    pthread_join(set_thread_handle, NULL);
    /* will never be here! */
    return 0;
}

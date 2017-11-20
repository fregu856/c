// Three threads are used, clock_thread for updating a clock, alarm_thread for
// handling alarm functionality, and GUI_thread for reading user input

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

////////////////////////////////////////////////////////////////////////////////
// helper functions
////////////////////////////////////////////////////////////////////////////////

/* time_from_set_time_message: extract time from set time
   message from user interface */
void time_from_set_time_message(char message[], int *hours, int *minutes, int *seconds)
{
    sscanf(message, "set %d %d %d", hours, minutes, seconds);
}

/* time_from_alarm_time_message: extract time from alarm time
   message from user interface */
void time_from_alarm_time_message(char message[], int *hours, int *minutes, int *seconds)
{
    sscanf(message, "alarm %d %d %d", hours, minutes, seconds);
}

/* time_ok: returns nonzero if hours, minutes and seconds represents a valid time */
int time_ok(int hours, int minutes, int seconds)
{
    return hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59 &&
        seconds >= 0 && seconds <= 59;
}

////////////////////////////////////////////////////////////////////////////////
// threads
////////////////////////////////////////////////////////////////////////////////

/* clock_thread: clock thread */
void* clock_thread(void *unused)
{
    /* local copies of the current time */
    int hours, minutes, seconds;

    // local copies of the alarm time:
    int alarm_hours, alarm_minutes, alarm_seconds;

    /* infinite loop */
    while (1)
    {
        /* read and display current time */
        clock_get_time(&hours, &minutes, &seconds);
        display_time(hours, minutes, seconds);

        // check if the alarm is enabled:
        if (clock_alarm_enabled())
        {
            // get the alarm time:
            clock_get_alarm_time(&alarm_hours, &alarm_minutes, &alarm_seconds);

            // check if the alarm time equals the current time:
            if (hours == alarm_hours && minutes == alarm_minutes && seconds == alarm_seconds)
            {
              // signal that the alarm shall be activated:
              clock_activation_signal();
            }
        }

        /* increment time */
        clock_increment_time();

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
        // wait for signal to activate the alarm:
        clock_activation_wait();

        // activate the alarm every 1.5 seconds until the user acknowledges it:
        while(clock_alarm_enabled())
        {
            display_alarm_text();

            // wait 1.5 seconds:
            usleep(1500000);
        }
    }
}

// GUI_thread: reads messages from the user interface and sets the clock, or sets
// the alarm, or resets the alarm, or exits the program
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

        /* check if it's a set message */
        if (strncmp(message, "set", 3) == 0)
        {
            time_from_set_time_message(message, &hours, &minutes, &seconds);
            if (time_ok(hours, minutes, seconds))
            {
                clock_set_time(hours, minutes, seconds);
            }
            else
            {
                si_ui_show_error("set: Illegal value for hours, minutes or seconds");
            }
        }
        // check if it's an alarm message:
        else if (strncmp(message, "alarm", 5) == 0)
        {
            time_from_alarm_time_message(message, &hours, &minutes, &seconds);
            if (time_ok(hours, minutes, seconds))
            {
                clock_set_alarm_time(hours, minutes, seconds);
                clock_enable_alarm();

                display_alarm_time(hours, minutes, seconds);
            }
            else
            {
                si_ui_show_error("alarm: Illegal value for hours, minutes or seconds");
            }
        }
        // check if it's a reset message:
        else if (strcmp(message, "reset") == 0)
        {
            // acknowledge the alarm:
            clock_disable_alarm();
            erase_alarm_time();
            erase_alarm_text();
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

////////////////////////////////////////////////////////////////////////////////
// main
////////////////////////////////////////////////////////////////////////////////

int main(void)
{
    // initialize UI channel:
    si_ui_init();

    // initialize display:
    display_init();

    // initialize clock:
    clock_init();

    // create threads:
    pthread_t clock_thread_handle;
    pthread_t alarm_thread_handle;
    pthread_t GUI_thread_handle;
    pthread_create(&clock_thread_handle, NULL, clock_thread, 0);
    pthread_create(&alarm_thread_handle, NULL, alarm_thread, 0);
    pthread_create(&GUI_thread_handle, NULL, GUI_thread, 0);

    // join threads:
    pthread_join(clock_thread_handle, NULL);
    pthread_join(alarm_thread_handle, NULL);
    pthread_join(GUI_thread_handle, NULL);

    return 0;
}

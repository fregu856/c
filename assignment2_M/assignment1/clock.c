/* standard library includes */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "si_ui.h"

#include "clock.h"

/* time data type */ 
typedef struct 
{
    int hours; 
    int minutes; 
    int seconds; 
} time_data_type; 

/* clock data type */ 
typedef struct
{
    /* the current time */ 
    time_data_type time; 
    /* alarm time */ 
    time_data_type alarm_time; 
    /* alarm enabled flag */ 
    int alarm_enabled; 

    /* semaphore for mutual exclusion */ 
    pthread_mutex_t mutex; 

    /* semaphore for alarm activation */ 
    sem_t start_alarm; 

} clock_data_type;

/* the actual clock */ 
static clock_data_type Clock;

/* clock_init: initialise clock */ 
void clock_init(void)
{
    /* initialise time and alarm time */ 

    Clock.time.hours = 0; 
    Clock.time.minutes = 0; 
    Clock.time.seconds = 0; 

    Clock.alarm_time.hours = 0; 
    Clock.alarm_time.minutes = 0; 
    Clock.alarm_time.seconds = 0; 
    
    /* alarm is not enabled */ 
    Clock.alarm_enabled = 0; 

    /* initialise semaphores */ 
    pthread_mutex_init(&Clock.mutex, NULL); 
    sem_init(&Clock.start_alarm, 0, 0); 
}

/* clock_set_time: set current time to hours, minutes and seconds */ 
void clock_set_time(int hours, int minutes, int seconds)
{
    pthread_mutex_lock(&Clock.mutex); 

    Clock.time.hours = hours; 
    Clock.time.minutes = minutes; 
    Clock.time.seconds = seconds; 

    pthread_mutex_unlock(&Clock.mutex); 
}

/* time_from_set_time_message: extract time from set time 
   message from user interface */ 
void time_from_set_time_message(
    char message[], int *hours, int *minutes, int *seconds)
{
    sscanf(message,"set %d %d %d", hours, minutes, seconds); 
}

/* get_time: read time from common clock variables */
void get_time(int *hours, int *minutes, int *seconds)
{
    /* reserve clock variables */
    pthread_mutex_lock(&Clock.mutex);

    /* read values */
    *hours = Clock.time.hours;
    *minutes = Clock.time.minutes;
    *seconds = Clock.time.seconds;

    /* release clock variables */
    pthread_mutex_unlock(&Clock.mutex);
}

/* clock_set_alarm_time: set current time to hours, minutes and seconds */ 
void alarm_clock_set_time(int hours, int minutes, int seconds)
{
    pthread_mutex_lock(&Clock.mutex); 

    Clock.alarm_time.hours = hours; 
    Clock.alarm_time.minutes = minutes; 
    Clock.alarm_time.seconds = seconds; 
    
    Clock.alarm_enabled = 1;
    sem_post(&Clock.start_alarm);
    

    pthread_mutex_unlock(&Clock.mutex); 
}

/* alarm_clock_activate: activate alarm when alarm is enabled and time is alarm_time */
void alarm_clock_activate()
{
    sem_post(&Clock.start_alarm);
}

/* alarm_clock_enable: enable alarm when alarm time is set */
void alarm_clock_enable()
{
    pthread_mutex_lock(&Clock.mutex);
    
    Clock.alarm_enabled = 1;

    pthread_mutex_unlock(&Clock.mutex); 
}

/* alarm_clock_disable: disable alarm when reset is called */
void alarm_clock_disable()
{
    pthread_mutex_lock(&Clock.mutex);
    
    Clock.alarm_enabled = 0;

    pthread_mutex_unlock(&Clock.mutex); 
}

/* compare_time: compares time with alarm_time if alarm is enabled. */
void compare_time(int hours, int minutes, int seconds)
{
    int h, m, s;
    pthread_mutex_lock(&Clock.mutex);
    get_alarm_time(&h, &m, &s);
    if(Clock.alarm_enabled == 1)
    {    
	if(h == hours && m == minutes && s == seconds)
    	{
	    alarm_clock_activate();
    	}
    }
    pthread_mutex_unlock(&Clock.mutex);
}

void wait_for_active()
{
    sem_wait(&Clock.start_alarm);
}

/* time_from_set_alarm_time_message: extract time from set time 
   message from user interface */ 
void time_from_set_alarm_time_message(
    char message[], int *hours, int *minutes, int *seconds)
{
    sscanf(message,"alarm %d %d %d", hours, minutes, seconds); 
}

/* get_alarm_time: read time from common clock variables */
void get_alarm_time(int *hours, int *minutes, int *seconds)
{
    /* reserve clock variables */
    pthread_mutex_lock(&Clock.mutex);

    /* read values */
    *hours = Clock.alarm_time.hours;
    *minutes = Clock.alarm_time.minutes;
    *seconds = Clock.alarm_time.seconds;

    /* release clock variables */
    pthread_mutex_unlock(&Clock.mutex);
}

/* increment_time: increments the current time with
   one second */
void increment_time(void)
{
    /* reserve clock variables */
    pthread_mutex_lock(&Clock.mutex);

    /* increment time */
    Clock.time.seconds++;
    if (Clock.time.seconds > 59)
    {
        Clock.time.seconds = 0;
        Clock.time.minutes++;
        if (Clock.time.minutes > 59)
        {
            Clock.time.minutes = 0;
            Clock.time.hours++;
            if (Clock.time.hours > 23)
            {
                Clock.time.hours = 0;
            }
        }
    }

    /* release clock variables */
    pthread_mutex_unlock(&Clock.mutex);
}

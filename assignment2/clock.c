#include <pthread.h>
#include <semaphore.h>

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
static clock_data_type Clock; // (this use of static means that the declared variable is visible only in this file (where it is declared))

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
    sem_init(&Clock.start_alarm, 0, 0); // (sem_init(&Semaphore, 0, init_value), init_value = 0 for assym sync)
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

/* clock_increment_time: increments the current time with one second */
void clock_increment_time(void)
{
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

    pthread_mutex_unlock(&Clock.mutex);
}

/* clock_get_time: get the current time */
void clock_get_time(int *hours, int *minutes, int *seconds)
{
    pthread_mutex_lock(&Clock.mutex);
    *hours = Clock.time.hours;
    *minutes = Clock.time.minutes;
    *seconds = Clock.time.seconds;
    pthread_mutex_unlock(&Clock.mutex);
}

/* clock_set_alarm_time: set alarm time to hours, minutes and seconds */
void clock_set_alarm_time(int hours, int minutes, int seconds)
{
    pthread_mutex_lock(&Clock.mutex);
    Clock.alarm_time.hours = hours;
    Clock.alarm_time.minutes = minutes;
    Clock.alarm_time.seconds = seconds;
    pthread_mutex_unlock(&Clock.mutex);
}

/* clock_get_alarm_time: get the alarm time */
void clock_get_alarm_time(int *hours, int *minutes, int *seconds)
{
    pthread_mutex_lock(&Clock.mutex);
    *hours = Clock.alarm_time.hours;
    *minutes = Clock.alarm_time.minutes;
    *seconds = Clock.alarm_time.seconds;
    pthread_mutex_unlock(&Clock.mutex);
}

/* clock_enable_alarm: enable alarm */
void clock_enable_alarm(void)
{
  pthread_mutex_lock(&Clock.mutex);
  Clock.alarm_enabled = 1;
  pthread_mutex_unlock(&Clock.mutex);
}

/* clock_alarm_enabled: returns 1 if the alarm is enabled */
int clock_alarm_enabled(void)
{
    int alarm_enabled;

    pthread_mutex_lock(&Clock.mutex);
    alarm_enabled = Clock.alarm_enabled;
    pthread_mutex_unlock(&Clock.mutex);

    return alarm_enabled;
}

/* clock_disable_alarm: disable alarm */
void clock_disable_alarm(void)
{
  pthread_mutex_lock(&Clock.mutex);
  Clock.alarm_enabled = 0;
  pthread_mutex_unlock(&Clock.mutex);
}

/* clock_activation_signal: signal that the alarm shall be activated */
void clock_activation_signal(void)
{
    sem_post(&Clock.start_alarm);
}

/* clock_activation_wait: wait for the alarm to be activated */
void clock_activation_wait(void)
{
    sem_wait(&Clock.start_alarm);
}

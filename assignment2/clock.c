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

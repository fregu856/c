#ifndef CLOCK_H
#define CLOCK_H

/* clock_init: initialise clock */
void clock_init(void);

/* clock_set_time: set current time to hours, minutes and seconds */
void clock_set_time(int hours, int minutes, int seconds);

/* clock_increment_time: increments the current time with one second */
void clock_increment_time(void);

/* clock_get_time: get the current time */
void clock_get_time(int *hours, int *minutes, int *seconds);

/* clock_set_alarm_time: set alarm time to hours, minutes and seconds */
void clock_set_alarm_time(int hours, int minutes, int seconds);

/* clock_get_alarm_time: get the alarm time */
void clock_get_alarm_time(int *hours, int *minutes, int *seconds);

/* clock_enable_alarm: enable alarm */
void clock_enable_alarm(void);

/* clock_alarm_enabled: returns 1 if the alarm is enabled */
int clock_alarm_enabled(void);

/* clock_disable_alarm: disable alarm */
void clock_disable_alarm(void);

/* clock_activation_signal: signal that the alarm shall be activated */
void clock_activation_signal(void);

/* clock_activation_wait: wait for the alarm to be activated */
void clock_activation_wait(void);

#endif

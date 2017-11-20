#include "lift.h"

/* Simple_OS include */
#include <pthread.h>

/* drawing module */
#include "draw.h"

/* standard includes */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/* panic function, to be called when fatal errors occur */
static void lift_panic(const char message[])
{
    printf("LIFT_PANIC!!! ");
    printf("%s", message);
    printf("\n");
    exit(0);
}

/* --- monitor data type for lift and operations for create and delete START --- */

/* lift_create: creates and initialises a variable of type lift_type */
lift_type lift_create(void)
{
    /* the lift to be initialised */
    lift_type lift;

    /* floor counter */
    int floor;

    /* loop counter */
    int i;

    /* allocate memory */
    lift = (lift_type) malloc(sizeof(lift_data_type));

    /* initialise variables */

    /* initialise floor */
    lift->floor = 0;

    /* set direction of lift travel to up */
    lift->up = 1;

    /* the lift is not moving */
    lift->moving = 0;

    /* initialise person information */
    for (floor = 0; floor < N_FLOORS; floor++)
    {
        for (i = 0; i < MAX_N_PERSONS; i++)
        {
            lift->persons_to_enter[floor][i].id = NO_ID;
            lift->persons_to_enter[floor][i].to_floor = NO_FLOOR;
        }
    }

    /* initialise passenger information */
    for (i = 0; i < MAX_N_PASSENGERS; i++)
    {
        lift->passengers_in_lift[i].id = NO_ID;
        lift->passengers_in_lift[i].to_floor = NO_FLOOR;
    }

    /* initialise mutex and event variable */
    pthread_mutex_init(&lift->mutex, NULL);
    pthread_cond_init(&lift->change, NULL);

    return lift;
}

/* lift_delete: deallocates memory for lift */
void lift_delete(lift_type lift)
{
    free(lift);
}

/* --- monitor data type for lift and operations for create and delete END --- */


/* --- functions related to lift task START --- */

/* MONITOR function lift_next_floor: computes the floor to which the lift
   shall travel. The parameter *change_direction indicates if the direction
   shall be changed */
void lift_next_floor(lift_type lift, int *next_floor, int *change_direction) /////////////////////////
{
    int floor;
    int up;

    pthread_mutex_lock(&lift->mutex);
    floor = lift->floor;
    up = lift->up;
    pthread_mutex_unlock(&lift->mutex);

    if (floor == 0)
    {
        *next_floor = 1;
        *change_direction = 1;
    }
    else if (floor == N_FLOORS - 1)
    {
        *next_floor = N_FLOORS - 2;
        *change_direction = 1;
    }
    else if (up)
    {
        *next_floor = floor + 1;
        *change_direction = 0;
    }
    else // (if (!up))
    {
        *next_floor = floor - 1;
        *change_direction = 0;
    }
}

void lift_move(lift_type lift, int next_floor, int change_direction)
{
    /* reserve lift */
    pthread_mutex_lock(&lift->mutex);

    /* the lift is moving */
    lift->moving = 1;

    /* release lift */
    pthread_mutex_unlock(&lift->mutex);

    /* it takes two seconds to move to the next floor */
    usleep(2000000);

    /* reserve lift */
    pthread_mutex_lock(&lift->mutex);

    /* the lift is not moving */
    lift->moving = 0;

    /* the lift has arrived at next_floor */
    lift->floor = next_floor;

    /* check if direction shall be changed */
    if (change_direction)
    {
        lift->up = !lift->up;
    }

    /* draw, since a change has occurred */
    draw_lift(lift);

    /* release lift */
    pthread_mutex_unlock(&lift->mutex);
}

/* this function is used also by the person tasks */
static int n_passengers_in_lift(lift_type lift)
{
    int n_passengers = 0;
    int i;

    for (i = 0; i < MAX_N_PASSENGERS; i++)
    {
        if (lift->passengers_in_lift[i].id != NO_ID)
        {
            n_passengers++;
        }
    }
    return n_passengers;
}

// possible_to_enter_lift: returns 1 if there is room for at least one more
// passenger to enter the lift, otherwise returns 0: ///////////////////////////////////////////////////
static int possible_to_enter_lift(lift_type lift)
{
    if (n_passengers_in_lift(lift) < MAX_N_PASSENGERS)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// passengers_to_enter_lift: returns 1 if there is at least one passenger
// waiting to enter the lift on the current floor, otherwise returns 0: /////////////////////////////////////////
static int passengers_to_enter_lift(lift_type lift)
{
    int current_floor = lift->floor;

    int person_found = 0;
    for (int i = 0; i < MAX_N_PERSONS && !person_found; i++)
    {
        if (lift->persons_to_enter[current_floor][i].id != NO_ID)
        {
            person_found = 1;
        }
    }

    return person_found;
}

// passengers_to_exit_lift: returns 1 if there is at least one passenger in the
// lift that shall exit the lift on the current floor, otherwise returns 0: /////////////////////////////////////////
static int passengers_to_exit_lift(lift_type lift)
{
    int current_floor = lift->floor;

    int person_found = 0;
    for (int i = 0; i < MAX_N_PASSENGERS && !person_found; i++)
    {
        if(lift->passengers_in_lift[i].to_floor == current_floor)
        {
            person_found = 1;
        }
    }

    return person_found;
}

// lift_wait_for_passengers: returns 1 if the lift shall wait for
// passengers to enter/leave the lift, otherwise returns 0: /////////////////////////////////////////
static int lift_wait_for_passengers(lift_type lift)
{
    if (passengers_to_exit_lift(lift))
    {
        return 1;
    }
    else if (passengers_to_enter_lift(lift) && possible_to_enter_lift(lift))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/* MONITOR function lift_has_arrived: shall be called by the lift task
   when the lift has arrived at the next floor. This function indicates
   to other tasks that the lift has arrived, and then waits until the lift
   shall move again. */
void lift_has_arrived(lift_type lift) ////////////////////////////////////////////////
{
    pthread_mutex_lock(&lift->mutex);

    // signal that the lift has arrived at the next floor:
    pthread_cond_broadcast(&lift->change);

    while(lift_wait_for_passengers(lift))
    {
        pthread_cond_wait(&lift->change, &lift->mutex);
    }

    pthread_mutex_unlock(&lift->mutex);
}

/* --- functions related to lift task END --- */


/* --- functions related to person task START --- */

/* passenger_wait_for_lift: returns non-zero if the passenger shall
   wait for the lift, otherwise returns zero */
static int passenger_wait_for_lift(lift_type lift, int wait_floor)
{
    int waiting_ready =
        /* the lift is not moving */
        !lift->moving &&
        /* and the lift is at wait_floor */
        lift->floor == wait_floor &&
        /* and the lift is not full */
        n_passengers_in_lift(lift) < MAX_N_PASSENGERS;

    return !waiting_ready;
}

// passenger_wait_for_exit: returns non-zero if the passenger shall wait for the
// lift to arrive at his/her destination floor, otherwise returns zero: ////////////////////////////////
static int passenger_wait_for_exit(lift_type lift, int destination_floor)
{
    int waiting_ready =
        /* the lift is not moving */
        !lift->moving &&
        /* and the lift is at destination_floor */
        lift->floor == destination_floor;

    return !waiting_ready;
}

/* enter_floor: makes a person with id id stand at floor floor */
static void enter_floor(lift_type lift, int id, int floor)
{
    int i;
    int floor_index;
    int found;

    /* stand at floor */
    found = 0;
    for (i = 0; i < MAX_N_PERSONS && !found; i++)
    {
        if (lift->persons_to_enter[floor][i].id == NO_ID)
        {
            found = 1;
            floor_index = i;
        }
    }

    if (!found)
    {
        lift_panic("cannot enter floor");
    }

    /* enter floor at index floor_index */
    lift->persons_to_enter[floor][floor_index].id = id;
    lift->persons_to_enter[floor][floor_index].to_floor = NO_FLOOR;
}

/* leave_floor: makes a person with id id at enter_floor leave
   enter_floor */
static void leave_floor(lift_type lift, int id, int enter_floor)
/* fig_end lift_c_prot */
{
    int i;
    int floor_index;
    int found;

    /* leave the floor */
    found = 0;
    for (i = 0; i < MAX_N_PERSONS && !found; i++)
    {
        if (lift->persons_to_enter[enter_floor][i].id == id)
        {
            found = 1;
            floor_index = i;
        }
    }

    if (!found)
    {
        lift_panic("cannot leave floor");
    }

    /* leave floor at index floor_index */
    lift->persons_to_enter[enter_floor][floor_index].id = NO_ID;
    lift->persons_to_enter[enter_floor][floor_index].to_floor = NO_FLOOR;
}

// enter_lift: makes person *person enter the lift: /////////////////////////////////////////////////////////
static void enter_lift(lift_type lift, person_type person)
{
    int found_empty, passenger_index;

    found_empty = 0;
    for (int i = 0; i < MAX_N_PASSENGERS && !found_empty; i++)
    {
        if (lift->passengers_in_lift[i].id == NO_ID) // (if empty spot in the lift)
        {
            found_empty = 1;
            passenger_index = i;
        }
    }

    if (!found_empty)
    {
        lift_panic("cannot enter the lift!");
    }

    lift->passengers_in_lift[passenger_index] = *person;
}

// leave_lift: makes person *person leave the lift: /////////////////////////////////////////////////////////
static void leave_lift(lift_type lift, person_type person)
{
    int found, passenger_index;

    found = 0;
    for (int i = 0; i < MAX_N_PASSENGERS && !found; i++)
    {
        if (lift->passengers_in_lift[i].id == person->id)
        {
            found = 1;
            passenger_index = i;
        }
    }

    if (!found)
    {
        lift_panic("cannot leave the lift since the passenger wasn't found!");
    }

    lift->passengers_in_lift[passenger_index].id = NO_ID;
    lift->passengers_in_lift[passenger_index].to_floor = NO_FLOOR;
}

/* MONITOR function lift_travel: performs a journey with the lift
   starting at from_floor, and ending at to_floor */
void lift_travel(lift_type lift, int id, int from_floor, int to_floor) ///////////////////////////////
{
    // TODO! IS THIS FUNCTION CORRECT?!!

    person_data_type person;
    person.id = id;
    person.to_floor = to_floor;

    pthread_mutex_lock(&lift->mutex);

    enter_floor(lift, id, from_floor);
    pthread_cond_broadcast(&lift->change);

    draw_lift(lift);

    while(passenger_wait_for_lift(lift, from_floor))
    {
        pthread_cond_wait(&lift->change, &lift->mutex);
    }

    leave_floor(lift, id, from_floor);
    enter_lift(lift, &person);
    pthread_cond_broadcast(&lift->change);

    draw_lift(lift);

    while(passenger_wait_for_exit(lift, to_floor))
    {
        pthread_cond_wait(&lift->change, &lift->mutex);
    }

    leave_lift(lift, &person);
    pthread_cond_broadcast(&lift->change);

    draw_lift(lift);

    pthread_mutex_unlock(&lift->mutex);
}

/* --- functions related to person task END --- */

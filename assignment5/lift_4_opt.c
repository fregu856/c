#include "lift_4_opt.h" ///////////////////////////////////////////////////

/* drawing module */
#include "draw_4.h"

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

/* lift_create: creates and initialises a variable of type lift_type */
lift_type lift_create(void) ////////////////////////////////////////////////////////////////////////////
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

    /* initialise person information */
    for (floor = 0; floor < N_FLOORS; floor++)
    {
        for (i = 0; i < MAX_N_PERSONS; i++)
        {
            //////////////////////////////////////////////////////////////////////////////////////////////////
            lift->persons_to_enter[floor][i].id = NO_ID;
            lift->persons_to_enter[floor][i].index = NO_INDEX;
        }
    }

    /* initialise passenger information */
    for (i = 0; i < MAX_N_PASSENGERS; i++)
    {
        ///////////////////////////////////////////////////////////////////////////////////////////////////
        lift->passengers_in_lift[i].id = NO_ID;
        lift->passengers_in_lift[i].index = NO_INDEX;
    }

    return lift;
}

/* lift_delete: deallocates memory for lift */
void lift_delete(lift_type lift)
{
    free(lift);
}

/* lift_next_floor: computes the floor to which
   the lift shall travel. The parameter *change_direction
   indicates if the direction shall be changed */
void lift_next_floor(lift_type lift, int *next_floor, int *change_direction)
{
    int floor;
    int up;

    floor = lift->floor;
    up = lift->up;

    if (up)
    {
        if (floor == N_FLOORS - 1)
        {
            *next_floor = N_FLOORS - 2;
            *change_direction = 1;
        }
        else
        {
            *next_floor = floor + 1;
            *change_direction = 0;
        }
    }
    else
    {
        if (floor == 0)
        {
            *next_floor = 1;
            *change_direction = 1;
        }
        else
        {
            *next_floor = floor - 1;
            *change_direction = 0;
        }
    }
}

/* lift_move: makes the lift move from its current
   floor to next_floor. The parameter change_direction indicates if
   the move includes a change of direction. */
void lift_move(lift_type lift, int next_floor, int change_direction)
{
    /* the lift has arrived at next_floor */
    lift->floor = next_floor;

    /* check if direction shall be changed */
    if (change_direction)
    {
        lift->up = !lift->up;
    }
}

/* get_current_floor: returns the floor on which the lift is positioned */
int get_current_floor(lift_type lift)
{
    return lift->floor;
}

/* enter_floor: makes the person with id id and destination to_floor stand
   at floor floor */
void enter_floor(lift_type lift, person_type person_ptr) ///////////////////////////////////////////////////
{
    int i;
    int person_index;
    int found;

    int floor = person_ptr->from_floors[person_ptr->index]; //////////////////////////////////////////////

    /* stand at floor */
    found = 0;
    for (i = 0; i < MAX_N_PERSONS && !found; i++)
    {
        if (lift->persons_to_enter[floor][i].id == NO_ID)
        {
            found = 1;
            person_index = i;
        }
    }

    if (!found)
    {
        lift_panic("cannot enter floor");
    }

    /* enter floor at index person_index */ ///////////////////////////////////////////////////////
    lift->persons_to_enter[floor][person_index] = *person_ptr;
}

/* leave_floor: makes a person, standing at floor floor, leave the
   floor. The id and destination of the person are returned in the
   parameters *id and *to_floor */
void leave_floor(lift_type lift, int floor, person_type person_ptr) /////////////////////////////////////////////
{
    int i;
    int person_index;
    int found;

    /* leave the floor */
    found = 0;
    for (i = 0; i < MAX_N_PERSONS && !found; i++)
    {
        if (lift->persons_to_enter[floor][i].id == person_ptr->id)
        {
            found = 1;
            person_index = i;
        }
    }

    if (!found)
    {
        lift_panic("cannot leave floor");
    }

    // /* leave floor at index person_index */ ////////////////////////////////////////////////////////////////
    lift->persons_to_enter[floor][person_index].id = NO_ID;
    lift->persons_to_enter[floor][person_index].index = NO_INDEX;
}

/* enter_lift: makes the person with id id and destination to_floor
   enter the lift */
void enter_lift(lift_type lift, person_type person_ptr) //////////////////////////////////////////////////////
{
    int found_empty, passenger_index;

    found_empty = 0;
    int i;
    for (i = 0; i < MAX_N_PASSENGERS && !found_empty; i++)
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

    lift->passengers_in_lift[passenger_index] = *person_ptr; //////////////////////////////////////////
}

/* leave_lift: makes a person, standing inside the lift and having
   destination floor equal to floor, leave the lift. The id of the
   person is returned in the parameter *id */
void leave_lift(lift_type lift, int floor, person_type person_ptr) /////////////////////////////////////////////
{
    int found, passenger_index;

    found = 0;
    int i = 0;
    for (i = 0; i < MAX_N_PASSENGERS && !found; i++)
    {
        if (lift->passengers_in_lift[i].id == person_ptr->id)
        {
            found = 1;
            passenger_index = i;
        }
    }

    if (!found)
    {
        lift_panic("cannot leave the lift since the passenger wasn't found!");
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    lift->passengers_in_lift[passenger_index].id = NO_ID;
    lift->passengers_in_lift[passenger_index].index = NO_INDEX;
}

/* n_passengers_to_leave: returns the number of passengers in the
   lift having the destination floor equal to floor */
int n_passengers_to_leave(lift_type lift, int floor) //////////////////////////////////////////////
{
    int n_passengers = 0;
    int i;

    person_data_type passenger;

    for (i = 0; i < MAX_N_PASSENGERS; i++)
    {
        passenger = lift->passengers_in_lift[i];
        if (passenger.to_floors[passenger.index] == floor)
        {
            n_passengers++;
        }
    }
    return n_passengers;
}

/* n_persons_to_enter: returns the number of persons standing on
   floor floor */
int n_persons_to_enter(lift_type lift, int floor)
{
    int n_persons = 0;
    int i;

    for (i = 0; i < MAX_N_PERSONS; i++)
    {
        if (lift->persons_to_enter[floor][i].id != NO_ID)
        {
            n_persons++;
        }
    }
    return n_persons;
}

/* lift_is_full: returns nonzero if the lift is full, returns zero
   otherwise */
int lift_is_full(lift_type lift)
{
    return n_passengers_in_lift(lift) >= MAX_N_PASSENGERS;
}

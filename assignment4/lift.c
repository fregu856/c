#include "lift.h"

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

/* lift_move: makes the lift move from its current
   floor to next_floor. The parameter change_direction indicates if
   the move includes a change of direction. */
void lift_move(lift_type lift, int next_floor, int change_direction)
{

}

/* get_current_floor: returns the floor on which the lift is positioned */
int get_current_floor(lift_type lift)
{

}

/* enter_floor: makes the person with id id and destination to_floor stand
   at floor floor */
void enter_floor(lift_type lift, int id, int floor, int to_floor)
{

}

/* leave_floor: makes a person, standing at floor floor, leave the
   floor. The id and destination of the person are returned in the
   parameters *id and *to_floor */
void leave_floor(lift_type lift, int floor, int *id, int *to_floor)
{

}

/* enter_lift: makes the person with id id and destination to_floor
   enter the lift */
void enter_lift(lift_type lift, int id, int to_floor)
{

}

/* leave_lift: makes a person, standing inside the lift and having
   destination floor equal to floor, leave the lift. The id of the
   person is returned in the parameter *id */
void leave_lift(lift_type lift, int floor, int *id)
{

}

/* n_passengers_to_leave: returns the number of passengers in the
   lift having the destination floor equal to floor */
int n_passengers_to_leave(lift_type lift, int floor)
{

}

/* n_persons_to_enter: returns the number of persons standing on
   floor floor */
int n_persons_to_enter(lift_type lift, int floor)
{

}

/* lift_is_full: returns nonzero if the lift is full, returns zero
   otherwise */
int lift_is_full(lift_type lift)
{

}

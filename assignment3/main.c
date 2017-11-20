#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/prctl.h>

#include "lift.h"
#include "si_ui.h"
#include "debug.h"

// Unfortunately the rand() function is not thread-safe. However, the
// rand_r() function is thread-safe, but need a pointer to an int to
// store the current state of the pseudo-random generator.  This
// pointer needs to be unique for every thread that might call
// rand_r() simultaneously. The functions below are wrappers around
// rand_r() which should work in the environment encountered in
// assignment 3.
//

// semaphore for asymmetric synchronization (unique id to person threads) /////////////////////////
sem_t id_read;

// array of passenger thread handles: //////////////////////////////////////////////////////
pthread_t passenger_thread_handles[MAX_N_PERSONS];

static unsigned int rand_r_state[MAX_N_PERSONS];

// Get a random value between 0 and maximum_value. The passenger_id
// parameter is used to ensure that the rand_r() function is called in
// a thread-safe manner.
static int get_random_value(int passenger_id, int maximum_value)
{
		return rand_r(&rand_r_state[passenger_id]) % (maximum_value + 1);
}

static lift_type Lift;

// Initialize the random seeds used by the get_random_value() function
// above.
static void init_random(void)
{
			int i;
			prctl(PR_SET_NAME,"Lift Thread",0,0,0); // Sets a name shown in debuggers

			for(i=0; i < MAX_N_PERSONS; i++)
			{
					// Use this statement if the same random number sequence
					// shall be used for every execution of the program.
					rand_r_state[i] = i;

					// Use this statement if the random number sequence
					// shall differ between different runs of the
					// program. (As the current time (as measured in
					// seconds) is used as a seed.)
					rand_r_state[i] = i+time(NULL);
		}
}

////////////////////////////////////////////////////////////////////////////////
// threads
////////////////////////////////////////////////////////////////////////////////

static void* lift_thread(void *unused) /////////////////////////////////////////
{
		int next_floor, change_direction;

		while(1)
		{
				// compute which floor the lift should move to next:
				lift_next_floor(Lift, &next_floor, &change_direction);

				// move the lift to the next floor:
				lift_move(Lift, next_floor, change_direction);

				// signal that the lift has arrived at the next floor, wait until the
				// lift shall move again:
				lift_has_arrived(Lift);
		}

		return NULL;
}

static void* passenger_thread(void *idptr) /////////////////////////////////////
{
		// Code that reads the passenger ID from the idptr pointer
		// (due to the way pthread_create works we first need to cast
		// the void pointer to an int pointer).
		int *tmp = (int*) idptr;
		int id = *tmp;

		// signal that it's safe to modify *idptr: ////////////////////////////////////////////////
		sem_post(&id_read);

	  // Sets a unique name shown in debuggers
	  char buf[100];
	  sprintf(buf, "Passenger #%d", id);
		prctl(PR_SET_NAME,buf,0,0,0);

		int from_floor, to_floor;

		while(1)
		{
				// * Select random floors
				// * Travel between these floors
				// * Wait a little while

				from_floor = get_random_value(id, N_FLOORS-1);
				to_floor = get_random_value(id, N_FLOORS-1);
				while (to_floor == from_floor)
				{
					to_floor = get_random_value(id, N_FLOORS-1);
				}

				debug_check_override(id, &from_floor, &to_floor);
				lift_travel(Lift, id, from_floor, to_floor);

				// sleep for 5 seconds:
				usleep(5000000);
		}

		return NULL;
}

static void* user_thread(void *unused) /////////////////////////////////////////
{
		// initialize the semaphore for asymmetric synchronization:
		sem_init(&id_read, 0, 0);

		int current_passenger_id = 0;
		char message[SI_UI_MAX_MESSAGE_SIZE];

		si_ui_set_size(670, 700);
		prctl(PR_SET_NAME,"User Thread",0,0,0); // Sets the name shown in debuggers for this thread

		int number_of_threads;

		while(1)
		{
				// Read a message from the GUI
				si_ui_receive(message);

				if(!strcmp(message, "new"))
				{
						// create a new passenger if possible, else
						// use si_ui_show_error() to show an error
						// message if too many passengers have been
						// created. Make sure that each passenger gets
						// a unique ID between 0 and MAX_N_PERSONS-1.

						if (current_passenger_id < MAX_N_PERSONS)
						{
								pthread_create(&passenger_thread_handles[current_passenger_id],
												NULL, passenger_thread, (void*) &current_passenger_id);

								// wait for current_passenger_id to be read by the passenger thread:
								sem_wait(&id_read);

								current_passenger_id++;
						}
						else
						{
								si_ui_show_error("Too many persons have been created!");
						}
				}
				else if(!strncmp(message, "new", 3))
				{
					sscanf(message, "new %d", &number_of_threads);
					if(number_of_threads > 0 && number_of_threads <= MAX_N_PERSONS)
					{
						int i;
						for(i = 0; i < number_of_threads; i++)
						{
							if (current_passenger_id < MAX_N_PERSONS)
							{
									pthread_create(&passenger_thread_handles[current_passenger_id],
													NULL, passenger_thread, (void*) &current_passenger_id);

									// wait for current_passenger_id to be read by the passenger thread:
									sem_wait(&id_read);

									current_passenger_id++;
							}
							else
							{
									si_ui_show_error("Too many persons have been created!");
							}
						}
					}
				}
				else if(!strcmp(message, "exit"))
				{
						lift_delete(Lift);
						exit(0);
				}
				else if(!strcmp(message, "pause"))
				{
						debug_pause();
				}
				else if(!strcmp(message, "unpause"))
				{
						debug_unpause();
				}
				else if(!strcmp(message, "test"))
				{
					// Test full hiss
					debug_override(0, 0, 4);
					debug_override(1, 0, 4);
					debug_override(2, 0, 4);
					debug_override(3, 0, 4);
					debug_override(4, 0, 4);
					debug_override(5, 2, 4);
					debug_override(6, 2, 4);
					debug_override(7, 2, 4);
					debug_override(8, 2, 4);
					debug_override(9, 2, 4);
				}
				else if(!strcmp(message, "test2"))
				{
					// Test
					debug_override(0, 0, 0);
					debug_override(1, 0, 0);
					debug_override(2, 0, 0);
					debug_override(3, 0, 0);
					debug_override(4, 0, 0);
					debug_override(5, 2, 2);
					debug_override(6, 2, 2);
					debug_override(7, 2, 2);
					debug_override(8, 2, 2);
					debug_override(9, 2, 2);
				}
		}

		return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// main
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) ////////////////////////////////////////////////
{
		si_ui_init();
		debug_init();

		init_random();

		Lift = lift_create();

		// create threads:
		pthread_t lift_thread_handle;
		pthread_t user_thread_handle;
		pthread_create(&lift_thread_handle, NULL, lift_thread, 0);
		pthread_create(&user_thread_handle, NULL, user_thread, 0);

		// join threads:
		pthread_join(lift_thread_handle, NULL);
		pthread_join(user_thread_handle, NULL);
		int i;
		for (i = 0; i < MAX_N_PERSONS; ++i)
		{
				pthread_join(passenger_thread_handles[i], NULL); 
		}

		return 0;
}

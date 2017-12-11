#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include "lift_4_mod.h" //////////////////////////////////////////////////////////
#include "si_ui.h"
#include "messages.h"

#include "draw_4.h"

#include <sys/time.h>

#define QUEUE_UI 0
#define QUEUE_LIFT 1
#define QUEUE_FIRSTPERSON 10

#define NUM_ITERATIONS 10000

int current_person_id = 0;

// These variables keeps track of the process IDs of all processes
// involved in the application so that they can be killed when the
// exit command is received.
static pid_t lift_pid;
static pid_t uidraw_pid;
static pid_t liftmove_pid;
static pid_t person_pid[MAX_N_PERSONS];

typedef enum {LIFT_TRAVEL, // A travel message is sent to the list process when a person would
                   				// like to make a lift travel
	      LIFT_TRAVEL_DONE, // A travel done message is sent to a person process when a
	                        // lift travel is finished
	      LIFT_MOVE,         // A move message is sent to the lift task when the lift shall move
	                        // to the next floor
			  VOID_TYPE
} lift_msg_type;

struct lift_msg{
		lift_msg_type type;  // Type of message
		int person_id;       // Specifies the person
		int from_floor;      // Specify source and destion for the LIFT_TRAVEL message.
		int to_floor;
};

////////////////////////////////////////////////////////////////////////////////
// general helper functions:
////////////////////////////////////////////////////////////////////////////////

static int get_random_value(int person_id, int maximum_value)
{
		return rand() % (maximum_value + 1);
}

// Initialize the random seeds used by the get_random_value() function
// above.
static void init_random(void)
{
		srand(getpid()); // The pid should be a good enough initialization for
                   	 // this case at least.
}

////////////////////////////////////////////////////////////////////////////////
// lift helper functions:
////////////////////////////////////////////////////////////////////////////////

// for all passengers that want to leave the lift, remove the passenger from the
// lift and send a LIFT_TRAVEL_DONE message:
static void drop_off_passengers_to_leave(lift_type lift)
{
		int id;

		struct lift_msg done_msg;
		done_msg.type = LIFT_TRAVEL_DONE;

		int current_floor = get_current_floor(lift);

		int i;
		for (i = 0; i < MAX_N_PASSENGERS; i++)
		{
				if(lift->passengers_in_lift[i].to_floor == current_floor)
				{
						// get the passenger's id:
						id = lift->passengers_in_lift[i].id;

						// make the passenger leave the lift:
						leave_lift(lift, current_floor, &id);

						// send a LIFT_TRAVEL_DONE message to the passenger's person_process:
						message_send((char *) &done_msg, sizeof(done_msg), QUEUE_FIRSTPERSON + id, 0);
				}
		}
}

// for all persons that want and CAN enter the lift, remove the
// person from the floor and make it enter the lift:
static void pick_up_persons_to_enter(lift_type lift)
{
		int id, to_floor;

		int current_floor = get_current_floor(lift);

		int i;
		for (i=0; i < MAX_N_PERSONS; i++)
		{
				if (lift_is_full(lift))
				{
						break;
				}

				if (lift->persons_to_enter[current_floor][i].id != NO_ID)
				{
						id = lift->persons_to_enter[current_floor][i].id;
						to_floor = lift->persons_to_enter[current_floor][i].to_floor;

						leave_floor(lift, current_floor, &id, &to_floor);

						enter_lift(lift, id, to_floor);
				}
		}
}

////////////////////////////////////////////////////////////////////////////////
// processes:
////////////////////////////////////////////////////////////////////////////////

static void liftmove_process(void)
{
		struct lift_msg m;
		m.type = LIFT_MOVE;

		while(1)
		{
				// sleep for two seconds:
				//usleep(2000000);

				// send a message to lift_process to move the lift:
				message_send((char *) &m, sizeof(m), QUEUE_LIFT, 0);
		}
}

static void lift_process(void)
{
	  lift_type Lift;
		Lift = lift_create();
		int change_direction, next_floor;
		char msgbuf[4096];

		while(1)
		{
				struct lift_msg *m;

				// draw the lift
				//message_send((char *) Lift, sizeof(*Lift), QUEUE_UI, 0);

				int len = message_receive(msgbuf, 4096, QUEUE_LIFT); // Wait for a message
				if(len < sizeof(struct lift_msg))
				{
						fprintf(stderr, "Message too short\n");
						continue; // (skip to the next while loop iteration)
				}

				m = (struct lift_msg *) msgbuf;
				switch(m->type)
				{
						case LIFT_MOVE:
								//    Check if passengers want to leave elevator
                //        Remove the passenger from the elevator
                //        Send a LIFT_TRAVEL_DONE for each passenger that leaves
                //        the elevator
								//    Check if passengers want to enter elevator
                //        Remove the passenger from the floor and into the elevator
								//    Move the lift

								// get how the lift should move:
								lift_next_floor(Lift, &next_floor, &change_direction);

								// move the lift:
								lift_move(Lift, next_floor, change_direction);

								// drop off all passengers that want to leave at the current floor:
								drop_off_passengers_to_leave(Lift);

								// pick upp all persons that want and CAN enter the lift:
								pick_up_persons_to_enter(Lift);

								break;
						case LIFT_TRAVEL:
	              // update the Lift structure so that the person with the given
								// ID is now present on the floor:
								enter_floor(Lift, m->person_id, m->from_floor, m->to_floor);
								break;
				}
		}
}

static void person_process(int id)
{
		init_random();
		char rec_msgbuf[4096];
		struct lift_msg rec_msg_data;
		struct lift_msg *rec_msg = &rec_msg_data;
		struct lift_msg msg;

		int from_floor, to_floor;

		int num_iterations = 0;

		struct timeval starttime;
    struct timeval endtime;
    long long int timediff;
    gettimeofday(&starttime, NULL);

		while(1)
		{

				if (num_iterations == NUM_ITERATIONS)
				{
						gettimeofday(&endtime, NULL);
						timediff = (endtime.tv_sec*1000000ULL + endtime.tv_usec) -
											 (starttime.tv_sec*1000000ULL + starttime.tv_usec);
						printf("%lld\n", timediff);
				}

				from_floor = get_random_value(id, N_FLOORS - 1);
				to_floor = get_random_value(id, N_FLOORS - 1);

				// send a LIFT_TRAVEL message to lift_process:
				msg.type = LIFT_TRAVEL;
				msg.person_id = id;
				msg.from_floor = from_floor;
				msg.to_floor = to_floor;
				message_send((char *) &msg, sizeof(msg), QUEUE_LIFT, 0);

				// wait for a LIFT_TRAVEL_DONE message: // TODO! valid method to deal with communication errors??
				rec_msg->type = VOID_TYPE;
				while (rec_msg->type != LIFT_TRAVEL_DONE)
				{
						int len = message_receive(rec_msgbuf, 4096, QUEUE_FIRSTPERSON + id);
						if(len < sizeof(struct lift_msg))
						{
								fprintf(stderr, "Received a too short message in person_process\n");
								continue; // (skip to the next while loop iteration)
						}

						rec_msg = (struct lift_msg *) rec_msgbuf;
						if (rec_msg->type != LIFT_TRAVEL_DONE)
						{
								fprintf(stderr, "Received message of invalid type in person_process\n");
								continue; // (skip to the next while loop iteration)
						}
				}

				// sleep for 5 seconds:
				//usleep(5000000);

				num_iterations++;
		}
}

// This is the final process called by main()
// It is responsible for:
//   * Receiving and executing commands from the java GUI
//   * Killing off all processes when exiting the application
void uicommand_process(void)
{
		int i;
		char message[SI_UI_MAX_MESSAGE_SIZE];

		while(1)
		{
				// Read a message from the GUI
				si_ui_receive(message);

				if(!strcmp(message, "exit"))
				{
						// The code below sends the SIGINT signal to
						// all processes involved in this application
						// except for the uicommand process itself
						// (which is exited by calling exit())
						kill(uidraw_pid, SIGINT);
						kill(lift_pid, SIGINT);
						kill(liftmove_pid, SIGINT);

						for(i=0; i < MAX_N_PERSONS; i++)
						{
								if(person_pid[i] > 0)
								{
										kill(person_pid[i], SIGINT);
								}
						}

						exit(0);
				}
		}
}

// This process is responsible for drawing the lift. Receives lift_type structures
// as messages.
void uidraw_process(void)
{
		char msg[1024];
		si_ui_set_size(670, 700);

		while(1)
		{
				message_receive(msg, 1024, QUEUE_UI);
				lift_type Lift = (lift_type) &msg[0];
				//draw_lift(Lift);
		}
}

////////////////////////////////////////////////////////////////////////////////
// main:
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
		message_init();
	  si_ui_init(); // Initialize user interface. (Must be done here!)

		lift_pid = fork();
		if(!lift_pid)
		{
				lift_process(); // (child process 1)
		}

		uidraw_pid = fork();
		if(!uidraw_pid)
		{
				uidraw_process(); // (child process 2)
		}

		liftmove_pid = fork();
		if(!liftmove_pid)
		{
				liftmove_process(); // (child process 3
		}

		int i;
		for(i = 0; i < MAX_N_PERSONS; i++)
		{
				person_pid[current_person_id] = fork();
				if(!person_pid[current_person_id])
				{
						person_process(current_person_id); // (child process)
				}

				current_person_id++;
		}

		uicommand_process(); // (parent process)

		return 0;
}

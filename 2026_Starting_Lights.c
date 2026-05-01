#include <kipr/wombat.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>

const int RUN_TIME = 120;

const int SERVO_A_PORT = 0;
const int SERVO_B_PORT = 3;

const int DROP = 1870;
const int CATCH = 1150;

const double DROP_GAP = 1.000;        // seconds
const double FIRST_DROP_AT = 9.500;   // seconds after match start
const double DROP_PERIOD = 7.000;     // seconds between drop starts

volatile sig_atomic_t dropper_should_stop = 0;

void set_all_servos(int pos);
void motors_on();
void flash(int number);
void rand_color();
void *dropper(void *arg);
void drop_at(double t);
void count5();
void wait_b();
void checklist();
void run();

double now()
{
	return seconds();
}

void sleep_until(double target_time)
{
	while (!dropper_should_stop && now() < target_time)
	{
		double remaining = target_time - now();

		if (remaining > 0.020)
		{
			msleep(10);
		}
		else
		{
			msleep(1);
		}
	}
}

int main()
{
	printf("Starting tournament UI . . .\n");
	enable_servos();
	run();
	return 0;
}

// Sets any plugged in servos
void set_all_servos(int pos)
{
	set_servo_position(0, pos);
	set_servo_position(1, pos);
	set_servo_position(2, pos);
	set_servo_position(3, pos);
}

// Turns on any plugged in lights
void motors_on()
{
	motor(0, 100);
	motor(1, 100);
	motor(2, 100);
	motor(3, 100);
}

void flash(int number)
{
	for (int x = 0; x < number; x++)
	{
		motors_on();
		msleep(500);
		ao();
		msleep(500);
	}
}

void rand_color()
{
	srand(seconds());

	printf("Upper large crate: ");
	if (rand() % 2) printf("RED\n");
	else printf("GREEN\n");

	printf("Small crate stack order: ");

	int rand_num = rand() % 6;
	int order[3];

	switch(rand_num)
	{
		case 0: order[0] = 0; order[1] = 1; order[2] = 2; break;
		case 1: order[0] = 0; order[1] = 2; order[2] = 1; break;
		case 2: order[0] = 1; order[1] = 0; order[2] = 2; break;
		case 3: order[0] = 1; order[1] = 2; order[2] = 0; break;
		case 4: order[0] = 2; order[1] = 0; order[2] = 1; break;
		case 5: order[0] = 2; order[1] = 1; order[2] = 0; break;
	}

	for (int i = 0; i < 3; i++)
	{
		switch(order[i])
		{
			case 0: printf("RED "); break;
			case 1: printf("GREEN "); break;
			case 2: printf("YELLOW "); break;
		}
	}

	printf("\n\n");
}

void drop_at(double t)
{
	sleep_until(t);

	if (dropper_should_stop) return;

	set_all_servos(DROP);

	sleep_until(t + DROP_GAP);

	if (dropper_should_stop) return;

	set_all_servos(CATCH);

	sleep_until(t + 2.0 * DROP_GAP);
}

void *dropper(void *arg)
{
	double match_start_time = *(double *)arg;

	for (int i = 0; i < 8; i++)
	{
		double drop_start_time = match_start_time + FIRST_DROP_AT + i * DROP_PERIOD;
		drop_at(drop_start_time);

		if (dropper_should_stop) break;
	}

	return NULL;
}

void count5()
{
	for (int i = 5; i >= 1; i--)
	{
		char text[8];
		sprintf(text, "%d", i);

		graphics_clear();
		graphics_print_string(text, 378, 150, 255, 255, 255, 6);
		graphics_update();

		msleep(1000);
	}

	graphics_clear();
}

void wait_b()
{
	while (!b_button())
	{
		if (c_button()) exit(EXIT_SUCCESS);
		msleep(10);
	}

	while (b_button())
	{
		msleep(10);
	}
}

void checklist()
{
	printf("Checklist for starting lights:\n");
	printf("- Lights plugged into any motor port.\n");
	printf("- Dropper servos plugged any servo ports.\n");
}

void draw_timer(int seconds_left)
{
	char sec[8];

	sprintf(sec, "%d", seconds_left);

	graphics_clear();
	graphics_print_string("Timer", 296, 50, 255, 255, 255, 6);
	graphics_print_string(sec, 340, 150, 255, 255, 255, 6);
	graphics_print_string("Press B button to Stop", 25, 250, 255, 255, 255, 5);
	graphics_update();
}

// Run program on loop
void run()
{
	double start_time;
	double end_time;
	double next_display_update;
	pthread_t dropper_thread;

	while (c_button() == 0)
	{
		printf("Press B button to set servos\n");
		wait_b();

		set_all_servos(CATCH);

		printf("Servos set . . .\n");
		printf("Press B button to proceed\n");
		wait_b();

		console_clear();

		checklist();

		printf("Press B button to proceed\n");
		wait_b();

		printf("After teams are calibrated, press B button to get random items\n\n");
		wait_b();

		rand_color();

		printf("Press B button to Start\n");
		printf("Press C button to stop program\n");
		wait_b();

		graphics_open(800, 325);
		msleep(500);
		count5();

		start_time = now();
		end_time = start_time + RUN_TIME;
		next_display_update = start_time;

		dropper_should_stop = 0;

		motors_on();

		pthread_create(&dropper_thread, NULL, dropper, &start_time);

		printf("Press B button to Stop\n");

		while (now() < end_time)
		{
			double current_time = now();
			double remaining = end_time - current_time;

			if (b_button()) break;
			if (c_button()) break;

			// Lights: on during first 5 seconds
			if (current_time < start_time + 5.0)
			{
				motors_on();
			}
			// Lights: off during main match
			else if (remaining > 5.0)
			{
				ao();
			}
			// Last 5 seconds: blink at 1 Hz using absolute time
			else
			{
				int half_second_phase = (int)((current_time - (end_time - 5.0)) * 2.0);

				if (half_second_phase % 2 == 0)
				{
					motors_on();
				}
				else
				{
					ao();
				}
			}

			// Timer display update, locked to absolute 1-second boundaries
			if (current_time >= next_display_update)
			{
				int seconds_left = (int)(remaining + 0.999);

				if (seconds_left < 0) seconds_left = 0;

				draw_timer(seconds_left);

				next_display_update += 1.0;

				// If graphics update was late, skip missed frames instead of drifting
				while (next_display_update < current_time)
				{
					next_display_update += 1.0;
				}
			}

			msleep(5);
		}

		printf("Stopping...\n");

		dropper_should_stop = 1;
		pthread_join(dropper_thread, NULL);

		graphics_close();
		ao();
		console_clear();

		msleep(3000);

		printf("Press B button to restart or C to exit\n");
		wait_b();

		msleep(1000);
		console_clear();
	}
}

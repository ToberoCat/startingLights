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
const int GAP = 1000;

void set_all_servos(int pos);
void motors_on();
void flash(int number);
void rand_color();
void *dropper();
void drop();
void count5();
void wait_b();
void checklist();
void run();

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
	int x = 0;
	for (x = 0; x < number; x++)
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
	switch(rand_num) {
		case 0:
			order[0] = 0;
			order[1] = 1;
			order[2] = 2;
			break;
		case 1:
			order[0] = 0;
			order[1] = 2;
			order[2] = 1;
			break;
		case 2:
			order[0] = 1;
			order[1] = 0;
			order[2] = 2;
			break;
		case 3:
			order[0] = 1;
			order[1] = 2;
			order[2] = 0;
			break;
		case 4:
			order[0] = 2;
			order[1] = 0;
			order[2] = 1;
			break;
		case 5:
			order[0] = 2;
			order[1] = 1;
			order[2] = 0;
			break;
	}
	// Bottom
	switch(order[0]) {
		case 0:
			printf("RED ");
			break;
		case 1:
			printf("GREEN ");
			break;
		case 2:
			printf("YELLOW ");
			break;
	}
	// Middle
	switch(order[1]) {
		case 0:
			printf("RED ");
			break;
		case 1:
			printf("GREEN ");
			break;
		case 2:
			printf("YELLOW ");
			break;
	}
	// Top
	switch(order[2]) {
		case 0:
			printf("RED");
			break;
		case 1:
			printf("GREEN");
			break;
		case 2:
			printf("YELLOW");
			break;
	}
	printf("\n\n");
}

void *dropper() {
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	msleep(9500);
	drop();
	for (int i = 0; i < 7; i++)
	{
		msleep(7000 - GAP * 2);
		drop();
	}
	return 0;
}

void drop()
{
	set_all_servos(DROP);
	msleep(GAP);
	set_all_servos(CATCH);
	msleep(GAP);
}

void count5()
{
	graphics_print_string("5", 378, 150, 255, 255, 255, 6);
	graphics_update();
	msleep(1000);
	graphics_clear();
	graphics_print_string("4", 378, 150, 255, 255, 255, 6);
	graphics_update();
	msleep(1000);
	graphics_clear();
	graphics_print_string("3", 378, 150, 255, 255, 255, 6);
	graphics_update();
	msleep(1000);
	graphics_clear();
	graphics_print_string("2", 378, 150, 255, 255, 255, 6);
	graphics_update();
	msleep(1000);
	graphics_clear();
	graphics_print_string("1", 378, 150, 255, 255, 255, 6);
	graphics_update();
	msleep(1000);
	graphics_clear();
}

void wait_b() {
	while (!b_button())
	{
		if (c_button()) exit(EXIT_SUCCESS);
	}
	while(b_button());
}

void checklist()
{
	printf("Checklist for starting lights:\n");
	printf("- Lights plugged into any motor port.\n");
	printf("- Dropper servos plugged any servo ports.\n");
}

// Run program on loop
void run()
{
	double start_time, end_time, curr_time, countdown, prev_time;
	// int time115 = 0;
	// int time5 = 0;
	char sec[3];
	int time;
	double decimal;
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

		// Press grey button to randomize crates
		printf("After teams are calibrated, press B button to get random items\n\n");
		wait_b();
		rand_color();

		// Wait to start
		printf("Press B button to Start\n");
		printf("Press C button to stop program\n");
		wait_b();

		// Countdown
		graphics_open(800, 325);
		msleep(500);
		count5();

		// Start
		start_time = seconds();
		// printf("Start time: %lf\n", start_time);
		end_time = start_time + RUN_TIME;
		curr_time = start_time;
		motors_on();
		pthread_create(&dropper_thread, NULL, dropper, NULL);
		graphics_clear();
		graphics_print_string("Timer", 296, 50, 255, 255, 255, 6);
		graphics_print_string("119", 340, 150, 255, 255, 255, 6);
		printf("Press B button to Stop\n");
		graphics_update();
		while (curr_time < end_time)
		{
			if (b_button()) break;
			if (c_button()) return;
			countdown = end_time - curr_time;
			if (countdown < RUN_TIME-5 && countdown > 5) ao();

			// Show time left
			time = (int)countdown;
			if (curr_time - prev_time > 1)
			{
				prev_time = curr_time;
				sprintf(sec, "%d", time);
				graphics_clear();
				graphics_print_string("Timer", 296, 50, 255, 255, 255, 6);
				graphics_print_string(sec, 340, 150, 255, 255, 255, 6);
				graphics_print_string("Press B button to Stop", 25, 250, 255, 255, 255, 5);
				graphics_update();
			}

			if (countdown <= 5)
			{
				decimal = countdown - (double)time;
				printf("decimal = %lf", decimal);
				if (decimal > 0.5) motors_on();
				else ao();
			}
			curr_time = seconds();
			msleep(10);
		}

		// Stopped
		printf("Stopping...\n");
		graphics_close();
		pthread_cancel(dropper_thread);
		pthread_join(dropper_thread, NULL);
		ao();
		console_clear();
		msleep(3000);

		// Wait to restart
		printf("Press B button to restart or C to exit\n");
		wait_b();
		msleep(1000);
		console_clear();
	}
}

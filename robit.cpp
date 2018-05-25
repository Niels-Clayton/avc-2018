#include <stdio.h>
#include <time.h>
#include "E101.h"
#include <sys/time.h>

/*
 *
 * Function: do_gate
 * ----------------------------
 *   Returns the colour threshold i.e. when a pixel colour is considered black
 *   (less than threshold) or white (more than threshold)
 *   returns: int threshold
 */
void do_Gate()
{
	char srvr_adr[15] = {"130.195.6.196"};
    connect_to_server(srvr_adr, 1024);
    char message[24] = {"Please"};
    send_to_server(message);
    receive_from_server(message);
    send_to_server(message);
}

/*
 * Function: get_color_threshold
 * ----------------------------
 *   Returns the colour threshold i.e. when a pixel colour is considered black
 *   (less than threshold) or white (more than threshold)
 *
 *   orientation: the orientation to check the presence of white pixels
 * 				  (0 for horizontal, -1 for left side vertical, 1 for
 * 					right side vertical)
 *
 *   returns: int threshold
 *
 */
int get_color_threshold(int orientation)
{
	int max = 0;
	int min = 255;

	if(orientation == 0)
	{
		for(int i = 0; i < 320; i++)
		{
			int pixel = get_pixel(120, i, 3);
			if(pixel > max)
			{
				max = pixel;
			}else if(pixel < min){
				min = pixel;
			}
		}
	}
	else
	{
		//check for white pixels in the vertical direction
		int col = (orientation == 1)?220:100; //check the left side first
		for(int row = 0; row < 240; row++)
		{
			int pixel = get_pixel(row, col, 3);
			if(pixel > max)
			{
				max = pixel;
			}else if(pixel < min){
				min = pixel;
			}
		}

	}

	int threshold = (max+min)/2;

	/* Check if image is all one colour */
	if(((max-min) < 100) && max > 120)
	{
		/* image is likely to be all white so set thr to 0 */
		threshold = 0;
	}else if(((max-min) < 100) && max < 120)
	{
		/* image is likely to be all black so set thr to 255 */
		threshold = 255;
	}
	//printf("%d \n %d" , max, min);

	return threshold;
}

/*
 * Function: get_white_pixels
 * ----------------------------
 *   Returns the number of white pixels in the middle row (120)
 *
 *   threshold: when a pixel colour is considered black (less than threshold)
 *              or white (more than threshold)
 *   pixels[]:  array to hold white and black pixels
 *
 * 	 orientation: the orientation to check the presence of white pixels
 * 				  (0 for horizontal, -1 for left side vertical, 1 for
 * 					right side vertical)
 *
 *   returns: int number of white pixels
 *
 */
void get_white_pixels(int threshold, int pixels[], int orientation)
{
	if(orientation == 0)
	{
		for(int i = 0; i < 320; i++)
		{
			pixels[i] = 0; // pixel is black

			int pixel = get_pixel(120, i, 3);
			if(pixel > threshold)
			{
				 pixels[i] = 1; // pixel is white
			}
		}
	}
	else
	{
		int col = (orientation == 1)?220:100; //check the left side first
		for(int row = 0; row < 240; row++)
		{
			pixels[row] = 0; // pixel is black

			int pixel = get_pixel(row, col, 3);
			if(pixel > threshold)
			{
				 pixels[row] = 1; // pixel is white
			}
		}
	}
}

/*
 * Function: red_pixels
 * ----------------------------
 *   Checks for the presence of red pixels
 *
 *   returns: int red - the number of red pixels
 *
 */
int red_pixels()
{
	int red = 0;
	for(int i = 0; i < 320; i++)
	{
		if(get_pixel(120, i, 0)>200 && get_pixel(120, i, 1)<100 && get_pixel(120, i, 2)<100)
		{
			red ++;
		}
	}
	return red;
}

/*
 * Function: calculate_error
 * ----------------------------
 *   Calculates the error signal (how far away sensor is from the white line)
 *
 *   returns: int error (normalized by number of white pixels)
 *
 */
int calculate_error(int orientation)
{
	int error = 0;
	int wp = 0; // number of white pixels
	int pixels[(orientation == 0)?320:240];
	if(orientation == 0)
	{
		get_white_pixels(get_color_threshold(0),pixels,0); //initialise array

		for(int i = 0; i < 320; i++)
		{
			error += (i-160)*pixels[i];
			if(pixels[i] == 1)
			{
				wp++;
			}
		}

		if(wp == 320){
			return 20000; // case for if sensor only senses white
		}else if(wp == 0){
			return 10000; // case for if sensor only senses black
		}else{
			return error/wp;
		}
	}
	else
	{
		get_white_pixels(get_color_threshold(orientation),pixels,orientation); //initialise array

		for(int i = 0; i < 240; i++)
		{
			if(pixels[i] == 1)
			{
				wp++;
			}
		}
		return ((wp > 0) ? 30000 : 0);
	}
	//else
	//{
	//	get_white_pixels(get_color_threshold(orientation),pixels,orientation); //initialise array
//
	//	for(int i = 0; i < 240; i++)
	//	{
	//		if(pixels[i] == 1)
	//		{
	//			wp++;
	//		}
	//	}
	//	return ((wp > 0) ? -30000 : 0);
	//}

}

/*
 * Function: calculate_pid
 * ----------------------------
 *   Calculates the pid
 *
 *   returns: int final error
 *
 */
int calculate_pid(int currentError, int previousError, long elapsed)
{

	/* Set kp and kd */
	double kp = 0.95;
	double kd = -0.35;
	double proportionalSignal = 0.0;
	double derivativeSignal = 0.0;

	/* Calculate Signals */
	proportionalSignal = (double)currentError * kp;

	derivativeSignal = (((double)currentError - (double)previousError) * kd)/elapsed;

	return proportionalSignal + derivativeSignal; // return final signal

}

/*
 * Function: set_motor_speed
 * ----------------------------
 *   Sets the speed for the left and right motor
 *
 */
void set_motor_speed(int motor1Speed, int motor2Speed)
{
	set_motor(1, motor1Speed);
	set_motor(2, motor2Speed);
}

void quadrant2()
{
	while(true)
	{
		/* Calculate Time */
		struct timeval t1;
		struct timeval t2;
		long elapsed = 1;
		gettimeofday(&t1,0);

		/* Take Images */
		take_picture();

		//if no red pixels have been detect, do normal motor controls
		if(calculate_error(0) != 20000)
		{
			/* Motor Speed Calculations */
			int currentError = calculate_error(0);
			int previousError = 1;

			if(currentError == 10000)
			{
				// If image is all black go backwards
				set_motor_speed(-110,-110);
				sleep1(0,250000);
				// If image is still black in next image, slowly go backwards again
				if(currentError == 10000 && previousError == 10000)
				{
					set_motor_speed(-80, -80);
				}
			}

			int finalSignal = calculate_pid(currentError, previousError, elapsed);
			previousError = currentError;
			t2 = t1;
			elapsed = (t2.tv_sec - t1.tv_sec)*1000000 +(t2.tv_usec-t1.tv_usec);

			if(currentError != 10000)
			{
				set_motor_speed(100 + finalSignal, 100 - finalSignal);

			}

		//	sleep1(0, 5000000);
		}
		else
		{
			set_motor_speed(80, 80);
			sleep1(0, 500000);
			set_motor_speed(0, 0);
			break;
		}
	}
}

void quadrant3()
{
	while(true)
	{
		/* Calculate Time */
		struct timeval t1;
		struct timeval t2;
		long elapsed = 1;
		gettimeofday(&t1,0);

		/* Take Images */
		take_picture();

		//if no red pixels have been detect, do normal motor controls
		if(red_pixels() <50)
		{
			/* Motor Speed Calculations */
			int currentError = calculate_error(0);
			int previousError = 1;

			if(calculate_error(-1) == 30000 && calculate_error(1) == 30000 && currentError == 20000)
			{
				//move forward and turn left if intersection detected
				set_motor_speed(100,100);
				sleep1(0, 300000);
				set_motor_speed(0, 100);
				sleep1(0,400000 );
			}

			else if(currentError == 10000)
			{
				if(calculate_error(1) == 30000 && calculate_error(-1) == 0)
				{
					//white on right side detected, turn right
					set_motor_speed(120, -120);
			    	sleep1(0,500000 );

				}
				else if(calculate_error(-1) == 30000 && calculate_error(1) == 0)
				{
					//white on left side detected, turn left
					set_motor_speed(-120, 120);
					sleep1(0,500000 );
					printf("turning left");
				}
				else
				{
					// If image is all black go backwards
					set_motor_speed(-100,-100);
					sleep1(0,500000);
					// If image is still black in next image, slowly go backwards again
				//	if(currentError == 10000 && previousError == 10000)
				//	{
				//		set_motor_speed(-80, -80);
				//	}
				}
			}

			int finalSignal = calculate_pid(currentError, previousError, elapsed);
			previousError = currentError;
			t2 = t1;
			elapsed = (t2.tv_sec - t1.tv_sec)*1000000 +(t2.tv_usec-t1.tv_usec);

			if(currentError != 10000)
			{
				set_motor_speed(90 + finalSignal, 90 - finalSignal);

			}

		//	sleep1(0, 5000000);
		}
		else
		{
			set_motor_speed(120, 120);
			sleep1(1,500000);
			set_motor_speed(0, 0);
			break;
		}
	}
}

int main()
{
	init();
	int quadrant = 1;
	while(true){
		switch(quadrant)
		{
			case 1 :
				do_Gate();
				quadrant = 2;
				break;
			case 2 :
				quadrant2();
				quadrant = 3;
				break;
			case 3 :
				quadrant3();
				quadrant = 4;
				break;
			default:
				set_motor_speed(0,0);
		}
	}
	return 0;

}

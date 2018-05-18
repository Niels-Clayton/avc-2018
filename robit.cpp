#include <stdio.h>
#include <time.h>
#include "E101.h"
#include <sys/time.h>

/** Opens the gate **/
void doGate(){
   //connects to server with the ip address 
   connect_to_server("130.195.6.196", 1024);
   //sends a message to the connected server
   send_to_server("Please");
   //receives message from the connected server
   char message[24];
   receive_from_server(message); //this may be buggy!
   send_to_server(message);
	}


int move_forward(int speed){
	set_motor (1, speed);
	set_motor (2, speed);
	sleep1(0, 250000);
	set_motor(1, 0);
	set_motor(2, 0);
}

/*
 * Function: get_color_threshold
 * ----------------------------
 *   Returns the colour threshold i.e. when a pixel colour is considered black
 *   (less than threshold) or white (more than threshold)
 *
 *   returns: int threshold
 *
 */
int get_color_threshold()
{
	int max = 0;
	int min = 255;
	
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
	printf("%d \n %d" , max, min);

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
 *   returns: int number of white pixels
 *
 */
void get_white_pixels(int threshold, int pixels[])
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

/*
 * Function: calculate_error
 * ----------------------------
 *   Calculates the error signal (how far away sensor is from the white line)
 *
 *   returns: int error (normalized by number of white pixels)
 *
 */
int calculate_error()
{
	int error = 0;
	int wp = 0; // number of white pixels
	int pixels[320];

	get_white_pixels(get_color_threshold(),pixels);

	for(int i = 0; i < 320; i++)
	{ 
		error += (i-160)*pixels[i];
		if(pixels[i] == 1)
		{
			wp++;
		}
	}

	return (wp >= 1) ? error/wp : 10000;
}

/*
 * Function: calculate_pid
 * ----------------------------
 *   Calculates the pid
 *
 *   returns: int final error
 *
 */
int calculate_pid()
{
	/* 
	 * NOTE: move previousError and takePic to main function once code for motor 
	 *       control has been made 
	 */
	int currentError = calculate_error();
	if(currentError == 10000)
	{
		/* 
		 * do something (go back?) - 10000 is an error signal to indicate that 
		 * the robot does not detect a line (i.e. all background)
		 */
	}
	int previousError = 1;
	double kp = 0.005;
	double kd = 0.005;
	double proportionalSignal = 0.0;
	double derivativeSignal = 0.0;

	proportionalSignal = (double)currentError * kp;
	derivativeSignal = ((double)currentError - (double)previousError) * kd;
	double finalSignal = proportionalSignal + derivativeSignal;

	previousError = currentError;

	return finalSignal;

}



int main(){
	/*
	init();
	doGate();
	take_picture();
	int previousError = 1;
	int speed = calculate_pid();
	set_motor(1, 50 + speed);
	set_motor(2, 50 - speed);
	set_motor (1, 255);
	set_motor (2, 255);
	sleep1(4, 000000);
	set_motor (1, 127);
	sleep1(2, 000000);
	set_motor (1, 0);
	set_motor (2, 0);
	sleep1(1, 000000);
	set_motor (1, -127);
	set_motor (2, -127);
	sleep1(4, 000000);
	set_motor (2, -40);
	sleep(2, 000000);
	*/
	init();
	int sector = 1;
	while(true){
		struct timeval t1;
		struct timeval t2;
		
		if(sector == 0){
			//doGate();
			//Whatever makes it go forward
			sector++;
		}
		else if(sector == 1){
			struct timeval t1;
			struct timeval t2;
			long elapsed = 1;
			/** Sector 1 Code Goes in here - Squiggly Line **/
			open_screen_stream();
			take_picture();
			update_screen();
			//int previousError = 1;
			int currentError = calculate_error();
			int previousFinalSig;
			int previousError = 1;
			gettimeofday(&t1,0);
			
			if(currentError == 10000)
			{
				/* 
				 * do something (go back?) - 10000 is an error signal to indicate that 
				 * the robot does not detect a line (i.e. all background)
				 */
				set_motor(1,-120);
				set_motor(2,-120);
				sleep1(0,250000);
				if(currentError == 10000 && previousError == 10000){
				set_motor(1, -80);
				set_motor(2, -80);
				}
			}
			
			double kp = 0.90;
			double kd = -0.15;
			double proportionalSignal = 0.0;
			double derivativeSignal = 0.0;

			proportionalSignal = (double)currentError * kp;
			
			derivativeSignal = (((double)currentError - (double)previousError) * kd)/elapsed;
			
			double finalSignal = proportionalSignal + derivativeSignal;

			previousError = currentError;
			previousFinalSig = finalSignal;
			t2 = t1;
			elapsed = (t2.tv_sec - t1.tv_sec)*1000000 +(t2.tv_usec-t1.tv_usec);
			
			printf("current error: %d\n", currentError);
			printf("previous error: %d\n", previousError);
			printf("pid: %f\n", finalSignal);
			printf("leftMot: %d\n", (int)(120 + finalSignal));
			printf("rightMot: %d\n",(int) (120 - finalSignal));
			//return finalSignal;
			if(finalSignal != 10000){
				set_motor(1, 120 + finalSignal);
				set_motor(2, 120 - finalSignal);
			}else{
				printf("Going backward\n");
				set_motor(1, 135);
				set_motor(2, 135);
				sleep1(0,200000);
				set_motor(1, 0);
				set_motor(2, 0);
				
			}
			sleep1(0, 5000000);
			
		}
		else if(sector == 2){
			/** Sector 2 Code goes in here - Straight Lines **/
		}
		else if(sector == 3){
			/** Sector 3 Code goes in here - Maze Naviagaton **/
		}
	}
	/*
	//Reset motors to 0
	set_motor(1, 0);
	set_motor(2, 0);
	sleep1(5, 000000);
	//int speed = 40;
	move_forward(speed);
	* */
return 0;}

#include <stdio.h>
#include <time.h>
#include "E101.h"

int main(){
	init();
	set_motor (1, 255);
	set_motor (2, 255);
	sleep1(4, 000000);
	set_motor (1, 127);
	sleep1(2, 000000)
	set_motor (1, 0);
	set_motor (2, 0);
	sleep1(1, 000000)
	set_motor (1, -127);
	set_motor (2, -127);
	sleep1(4, 000000);
	set_motor (2, -40);
	sleep(2, 000000);
	set_motor(1, 0);
	set_motor(2, 0);
	sleep1(5, 000000);
	int speed = 40;
	move_forward(speed);
return 0;}

int move_forward(speed){
	set_motor (1, speed);
	set_motor (2, speed);
	sleep1(10,000000);
	set_motor(1, 0);
	set_motor(2, 0);
}
//Opens a connection to the gate, if a connection is established, 0 is returned.
int establishConnectionToGate(){
	int reply = connect_to_server("130.195.6.196", 1024);
	return reply;
}

int sendMessageToGate(char message){
	int reply = send_to_server(message);
	return reply;
}

int recieveMessageFromGate(){}

int openGate(){
	chat message = "Please";
	if(establishConnectionToGate == 0){
		if(sendMessageToGate(message) == 0){
			boolean done = false;
			while(message != "Please" && done == false){
				receive_from_server(message);
				done = true;
			}
			sendMessageToGate(message);
		}
	}
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
  /* Return threshold */
  return (max+min)/2;
}

/*
 * Function: get_white_pixels
 * ----------------------------
 *   Returns the number of white pixels in the middle row (120)
 *
 *   threshold: when a pixel colour is considered black (less than threshold) 
 *              or white (more than threshold)
 *
 *   returns: int number of white pixels
 *
 */
int get_white_pixels(int threshold)
{
  int whitePixels;
  for(int i = 0; i < 320; i++)
  { 
    int pixel = get_pixel(120, i, 3);
    if(pixel > threshold)
    {
      whitePixels++;
    }
  }
  return whitePixels;
}

/*
 * Function: get_error
 * ----------------------------
 *   Calculates the error signal (how far away sensor is from the white line)
 *
 *   returns: int error
 *
 */
int get_error()
{
  int error = 0;
  for(int i = 0; i < 320; i++)
  { 
    int pixel = get_pixel(120, i, 3);
    error += (i-160)*pixel;
  }
  return (error/get_white_pixels(get_color_threshold()));
}

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

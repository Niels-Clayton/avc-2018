#include <stdio.h>
#include <time.h>
#include "E101.h"
#include <sys/time.h>


int main()
{
    init();
    int quadrant = 1;
    while(true){
        switch(quadrant)
        {
            case 1 :
            do_Gate();
            quadrant ++;
            break;
            case 2 :
            quadrant2();
            quadrant ++;
            break;
            case 3 :
            quadrant3();
            quadrant ++;
            break;
            default:
            set_motor_speed(0,0);
        }
    }
    return 0;
}

/*
* Function: do_gate
* ------------------
* Sends the array of message "Please" to the server
* then recieves a password back, and sends it back
* to the server to open the gate.
*/
void do_Gate()
{
    connect_to_server("130.195.6.196", 1024);
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
int calculate_error()
{
    int error = 0;
    int wp = 0; // number of white pixels
    int pixels[320];

    get_white_pixels(get_color_threshold(),pixels); //initialise array

    for(int i = 0; i < 320; i++)
    {
        error += (i-160)*pixels[i];
        if(pixels[i] == 1)
        {
            wp++;
        }
    }

    if(wp >= 310){
        return 20000; // case for if sensor only senses white
    }else if(wp <= 10){
        return 10000; // case for if sensor only senses black
    }else if((wp > 100 && wp < 180) && error < 0){
        return -30000; // case for if half the screen has white
    }else if ((wp > 100 && wp < 180) && error > 0){
        return 30000;
    }else{
        return error/wp;
    }
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
    double kd = -0.40;
    double proportionalSignal = 0.0;
    double derivativeSignal = 0.0;

    /* Calculate Signals */
    proportionalSignal = (double)currentError * kp;

    derivativeSignal = (((double)currentError - (double)previousError) * kd)/elapsed;

    return proportionalSignal + derivativeSignal; // return final signal

}

void set_motor_speed(int motor1Speed, int motor2Speed)
{
    set_motor(1, motor1Speed);
    set_motor(2, motor2Speed);
}

// This code navigates the curved white line in the maze
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
        open_screen_stream();
        take_picture();
        update_screen();

        /* Motor Speed Calculations */
        int currentError = calculate_error();
        int previousError = 1;

        /* Calculates the speed that each motor is set*/
        int finalSignal = calculate_pid(currentError, previousError, elapsed);
        previousError = currentError;
        t2 = t1;
        elapsed = (t2.tv_sec - t1.tv_sec)*1000000 +(t2.tv_usec-t1.tv_usec);

        /* Different error signals for different situations*/
        if(currentError == 10000)
        {
            // If image is all black go backwards
            set_motor_speed(-120,-120);
            sleep1(0,250000);
            // If image is still black in next image, slowly go backwards again
            if(currentError == 10000 && previousError == 10000)
            {
                set_motor_speed(-80, -80);
            }
        }

        else if(currentError != 10000)
        {
            set_motor_speed(120 + finalSignal, 120 - finalSignal);

        }

        else if(currentError == 20000)
        {
            //if image is all white break the loop and go to quadrant 3
            break;
        }
        sleep1(0, 5000000);
    }
}

// This code will handle following the line, and making shark rightangle turns,
// It should be mostly the same as quadrant2, but with extra error statments
// to make the rightangle turns
// atm im using the old error message for half the screen white, we shoud implement
// the two vertival scanns instead as it will be more reliable.
void quadrant3()
{
    //if no red pixels have been detecedt, do normal motor controls
    if(red_pixels() <50)
    {
        while(true)
        {
            /* Calculate Time */
            struct timeval t1;
            struct timeval t2;
            long elapsed = 1;
            gettimeofday(&t1,0);

            /* Take Images */
            open_screen_stream();
            take_picture();
            update_screen();

            /* Motor Speed Calculations */
            int currentError = calculate_error();
            int previousError = 1;

            /* Calculates the speed that each motor is set*/
            int finalSignal = calculate_pid(currentError, previousError, elapsed);
            previousError = currentError;
            t2 = t1;
            elapsed = (t2.tv_sec - t1.tv_sec)*1000000 +(t2.tv_usec-t1.tv_usec);

            /* Different error signals for different situations*/
            if(currentError == 10000)
            {
                // If image is all black go backwards
                set_motor_speed(-120,-120);
                sleep1(0,250000);
                // If image is still black in next image, slowly go backwards again
                if(currentError == 10000 && previousError == 10000)
                {
                    set_motor_speed(-80, -80);
                }
            }

            else if(currentError != 10000)
            {
                set_motor_speed(120 + finalSignal, 120 - finalSignal);

            }

            else if(currentError == 20000)
            {
                //if image is all white, right angle turn left.

            }
            else if(currentError == -30000)
            {
                //If half of the image has white only on the left side
                set_motor_speed(100,100);
                sleep1(0, 500000);
                set_motor_speed(0, 100);
                sleep1(0,500000 );
            }
            else if(currentError == 30000 || currentError == 20000)
            {
                // If half of the image has white only on the right side
                set_motor_speed(100,100);
                sleep1(0, 500000);
                set_motor_speed(100, 0);
                sleep1(0,500000 );
            }
        }
        else
        {
            break;
        }
        sleep1(0, 5000000);
    }
}

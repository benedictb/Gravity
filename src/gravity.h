#include <stdio.h>
#include "gfx5.h"
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#define DELTA_T 20000 //time between frames
#define CLOUD_SIZE 2000 //number of particles
#define SPECK_SIZE 4 //size of the dots in dot mode
#define STREAK_LENGTH 20 //length of the streaks in streak mode
#define PULSE_CONSTANT .93 //determines how often the program pulses in pulse mode

#define RANDOM 0
#define GRADIENT 1
#define DOT 0
#define STREAK 1
#define GRAVITY 0
#define EXPLOSION 1
#define FADE 2
#define PULSE 3

typedef struct
{
	double xpos;
	double ypos;
	int color[3];
	double xvel;
	double yvel;
	double theta; //from the 'center'
	double radius; //from the 'center'
} Speck;

typedef struct 
{
	int x;
	int y;
	int style; //dots or streaks
	int color; //random or gradient 
	int acc; //acc constant for the system 
	int vel;  //max vel of the particles
	int accrate; //pulse, flow, gravity, fade 
} Star;

double updateAngle(Speck * mite, Star sol); //updates the theta for all specks
void updateRadius(Speck * mite, Star sol); //updates the radius from the 'center' for all specks
double ClickBurst(Speck * mite, Star * sol, int flag); //gives specks random velocity and sets position at 'center'
void updateColor(Speck * mite); //updates the color according to the gradient
void updateVel(Speck * mite, Star sol); //updates the velocity for each speck
void updatePos(Speck * mite); //updates the new position for each speck
void updateScreen(Speck * mite, Star sol); //displays all the dots in their current position with current color
void Circle(Speck * mite, Star sol); //I think this didn't work out
void convergeCircle(Speck * mite, Star sol); //also this
void flow(Speck * mite, Star sol); //this is for the flow pattern for the dots, a continuous stream  
void pulse(Speck * mite, Star sol); //this function displays a pulse of dots every few seconds

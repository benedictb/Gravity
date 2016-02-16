/*Ben Becker
bbecker5
Final Project
To compile: gcc gravity.c gfx5.c -lm -lX11 -o Gravity
**Sometimes in the pulse setting, it can get laggy in between. Once it reaches the next pulse the lag goes away.
**Also moving the mouse is very laggy, probably due to all the mouse events registering 
**I got it up to 2000 particles, but I'm not sure how these computers will do with that many 
*/




#include "gravity.h"


int main(void){	
	int k;
	char c,d;


	gfx_open(1000,1000,"Particles");
	gfx_text(100,100,"Welcome to Particle Sandbox! Click to reset, right click for a bigger reset");  
	gfx_text(100,110,"'s' for a different dot syle | 'r' for a different color scheme | 'a' to increase acc and 'z' to decrease it");
	gfx_text(100,120,"'e' for a flow pattern | 'f' for a fade pattern | 'g' for a gravity pattern | 'p' for a pulse pattern");
	gfx_text(100,130,"'.' to pause | 'q' to quit");
	d = gfx_wait();
	d=gfx_wait();


	time_t t; 
	srand(time(NULL));

	Speck cloud[CLOUD_SIZE]; //create all the specks
	Star sol; // set the initial parameters on the Star
	sol.x=500;
	sol.y=500;
	sol.style=0;
	sol.color=1;
	sol.acc = 3;
	sol.vel = 50;

	
	ClickBurst(cloud,&sol,1); //create a clickburst in the center

	while(1){ //animation loop

		gfx_clear();
		updateScreen(cloud,sol); //lower function that calls most of the other functions
		gfx_flush();
		usleep(DELTA_T);


		if (k=gfx_event_waiting()){
			c = gfx_wait();


			if (k==5) {sol.x=gfx_xpos(); sol.y = gfx_ypos();} //mouse movement (sometimes very laggy)

			if (k==2 || k==4){	//button release or key release	

				switch (c) {
					case 'q':
					if (c=='q') return 0; //quits

					case 1:
					ClickBurst(cloud,&sol,0); //resets all the specks
					break;

					case 3:
					ClickBurst(cloud, &sol, 2); //larger clickburst
					break;

					case 's':
					sol.style++; //change the shape of the specks dot or line
					break;

					case 'r':
					sol.color++; //change the color of the specks from random to gradient
					break;

					case 'a':
					sol.acc++; //up the acc to the center
					break;

					case 'z':
					sol.acc*=.75; //cut the acc to the center
					break;

					case 'e':
					sol.accrate = EXPLOSION; //flow animation
					break;

					case 'f':
					sol.accrate = FADE; //fade animation
					break;

					case 'g':
					sol.accrate = GRAVITY; //regular animation
					break;

					case 'p':
					sol.accrate = PULSE; //pulse animation
					break;

					case '.':
					c = gfx_wait(); //this pauses
					break;
				}
			}
		}

	}
	return 0;
}


double ClickBurst(Speck * mite, Star * sol, int flag){ //flag ==1 is initial, flag ==2 is explosion
	int i=0, maxVel=sol->vel;
	double magVel;
	
	if (flag==2) maxVel = 1000; //big burst

	if (flag==1){
		sol->x = 500;
		sol->y = 500; //initial one
	} else {
		sol->x = gfx_xpos(); //center on mouse
		sol->y = gfx_ypos();
	}



	for(i=0;i<CLOUD_SIZE;i++){ //this sets a random velocity to each speck, this block comes up again with variations
		magVel = maxVel - rand()%(maxVel*2 +1);
		mite->xpos = sol->x;
		mite->ypos = sol->y;

		int num = rand()%1001;
		double ratio = (double)num / 1000;


		mite->theta = ratio * (2 * M_PI);
		mite->xvel = magVel * cos(mite->theta); 
		mite->yvel = magVel * sin(mite->theta);
		mite++;
	}
}

double updateAngle(Speck * mite, Star sol){ //atan2 for a angle between pi and -pi
	double angle;
	int i;

	for(i=0;i<CLOUD_SIZE;i++){	
		mite->theta = atan2(sol.y - mite->ypos,mite->xpos - sol.x);
		mite++;
	}
}

void updateRadius(Speck * mite, Star sol){ //radius from center
	int i=0;
	for(i=0;i<CLOUD_SIZE;i++){
		mite->radius =pow(pow(mite->xpos-(sol.x),2)+pow((sol.y)-mite->ypos,2),.5);
		mite++;
	}
}

void updateColor(Speck * mite){ 
	int i=0;
	for(i=0;i<CLOUD_SIZE;i++){
		double ratio = mite->radius / 175;
		mite->color[0]= mite->xpos / 1000 * 255;    //x determines R
		mite->color[1]= mite->ypos / 1000 * 255;    //y determines G
		mite->color[2]=	ratio * 255;				//r determines B

		if (mite->color[0]<0) mite->color[0]=51;
		if (mite->color[1]<0) mite->color[1]=51;

		mite++;
	}
}

void updateVel(Speck * mite,Star sol){
	int i;

	if (sol.accrate == EXPLOSION){ //run flow function
		flow(mite,sol);
		return;
	} else if (sol.accrate==PULSE){ //run pulse function
		pulse(mite, sol);
		return;
	} else if (sol.accrate==FADE){ //reduce the vel of each speck as a function of its radius
		for(i=0;i<CLOUD_SIZE;i++){
			if (mite->radius >10){
				mite->yvel -= sol.acc*sin(mite->theta) / mite->radius;
				mite->xvel -= sol.acc*cos(mite->theta) / mite->radius;
				mite++;
			}
		}
	} else {
		for(i=0;i<CLOUD_SIZE;i++){ //swarm the specks back to the center

			if (abs(mite->xvel) > 15) mite->xvel*=.45; 
			if (abs(mite->yvel) > 15) mite->yvel*=.45;

			if (mite->radius >1){
				mite->yvel -= sol.acc*sin(mite->theta);
				mite->xvel -= sol.acc*cos(mite->theta);
				mite++;

			}
			if (mite->radius < 1){
				mite->yvel*=.75;
				mite->xvel*=.75;
				mite++;
			}

		}
	}
}

void updatePos(Speck * mite){ //update position using the velocity
	int i;
	for(i=0;i<CLOUD_SIZE;i++){
		mite->xpos+= 2*mite->xvel;
		mite->ypos-=2*mite->yvel;
		mite++;
	}
}

void updateScreen(Speck * mite, Star sol){
	
	updateRadius(mite, sol);
	updateColor(mite);
	updateAngle(mite, sol);
	updateVel(mite, sol);
	updatePos(mite); //run all the functions, then display
	int i; 


	for(i=0;i<CLOUD_SIZE;i++){
		
		switch (sol.color%2){
			
			case RANDOM:
			gfx_color(rand()%255,rand()%255,rand()%255); //random colors
			break;

			case GRADIENT:
			gfx_color(mite->color[0],mite->color[1],mite->color[2]);
			break;
		}


		
		switch (sol.style%2){

			case DOT:
			gfx_fill_circle(mite->xpos,mite->ypos,SPECK_SIZE); //filled circle
			break;

			case STREAK:
			gfx_line(mite->xpos,mite->ypos,mite->xpos+STREAK_LENGTH*cos(mite->theta),mite->ypos-STREAK_LENGTH*sin(mite->theta)); //line
			break;
		}



		mite++;
	}
}

void flow(Speck * mite, Star sol){
	int i,maxVel = 20;
	double magVel;
	Speck * clone = mite;

	for(i=0;i<CLOUD_SIZE;i++){
		magVel = pow(pow(mite->xvel,2)+pow(mite->yvel,2),.5);
		if(mite->xpos < -500 || mite->xpos > 1500 || mite->ypos <-500 || mite->ypos > 1500 || magVel >8 || magVel < 5){ 
			//if the dot is way out of bounds, set it back to the center and give it a new velocity in that range

			magVel = maxVel - rand()%(maxVel*2 +1);


			int num = rand()%1001;
			double ratio = (double)num / 1000;


			mite->theta = ratio * (2 * M_PI);
			mite->xvel = magVel * cos(mite->theta); 
			mite->yvel = magVel * sin(mite->theta);

			mite->xpos = sol.x;
			mite->ypos = sol.y;


		}

		mite++;

	}

	updateColor(clone);
}

void pulse(Speck * mite, Star sol){
	int i,bounds=0, maxVel=10;
	double magVel,actualMagVel;
	Speck * clone = mite;

	actualMagVel = pow(pow(mite->xvel,2)+pow(mite->yvel,2),.5);

	for(i=0;i<CLOUD_SIZE;i++){
		actualMagVel = pow(pow(mite->xvel,2)+pow(mite->yvel,2),.5);
		if(mite->xpos < 0 || mite->xpos > 1000 || mite->ypos <0 || mite->ypos > 1000){
			bounds++; //determine how many specks are out of bounds
		}
		if (actualMagVel < 7 || actualMagVel==0){ //makes sure none of them are going to fast 
			magVel = maxVel - rand()%(maxVel*2 +1);
			mite->xpos = sol.x;
			mite->ypos = sol.y;

			int num = rand()%1001;
			double ratio = (double)num / 1000;


			mite->theta = ratio * (2 * M_PI);
			mite->xvel = magVel * cos(mite->theta); 
			mite->yvel = magVel * sin(mite->theta);
		}

	}

	if (bounds < PULSE_CONSTANT * (CLOUD_SIZE)) return; //if too many are out of bounds, continue

	for(i=0;i<CLOUD_SIZE;i++){ //this resets all the specks that are out of bounds to the center again
		if(mite->xpos < 0 || mite->xpos > 1000 || mite->ypos <0 || mite->ypos > 1000){
			magVel = maxVel - rand()%(maxVel*2 +1);
			mite->xpos = sol.x;
			mite->ypos = sol.y;

			int num = rand()%1001;
			double ratio = (double)num / 1000;


			mite->theta = ratio * (2 * M_PI);
			mite->xvel = magVel * cos(mite->theta); 
			mite->yvel = magVel * sin(mite->theta);

		}
		mite++;
	}
	updateColor(clone);
}


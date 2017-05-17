//modified by: Sean Nickell
//date: 1/26/2017	
//purpose: creating a waterfall particle system
//
//cs3350 Spring 2017 Lab-1
//author: Gordon Griesel
//date: 2014 to present
//This program demonstrates the use of OpenGL and XWindows
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "fonts.h"

extern "C"
{
	#include "fonts.h"
}

#define TRIANGLES 180
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 30000
#define GRAVITY 0.17
#define rnd() (float)rand() / (float)RAND_MAX

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
};

struct Game {
	Shape box[6], circle;
	Particle particle[MAX_PARTICLES];
	int n;
	int bubbler;
	int mouse[2];
	Game() { n=0; bubbler = 0;};  

};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);


int main(void)
{
	int done=0;
	srand(time(NULL));
	initXWindows();
	init_opengl();
	//declare game object
	Game game;
	game.n=0;

	
	//declare a box shape
	
	for(int i=0; i<6; i++)
	{
	if(i==5)
	{
	game.box[i].width =85;
	game.box[i].height =15;
	game.box[i].center.x =500;
	game.box[i].center.y =115;
	}    
	else
	{    
	game.box[i].width = 85;
	game.box[i].height = 15;
	game.box[i].center.x = 120 + i*65;
	game.box[i].center.y = 500 - i*60;
	}
   	}		
	
	game.circle.radius = 100;
	game.circle.center.x = 500;
	game.circle.center.y = 20;
	
	
	
	
	//start animation
	while (!done) {
		while (XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_mouse(&e, &game);
			done = check_keys(&e, &game);
		}
		movement(&game);
		render(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	cleanup_fonts();
	return 0;
}

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "335 Lab1   LMB for particle");
}

void cleanupXWindows(void)
{
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(void)
{
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if (vi == NULL) {
		std::cout << "\n\tno appropriate visual found\n" << std::endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask | PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
		InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.8, 1, 1.0);
	//fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
}

void makeParticle(Game *game, int x, int y)
{
	if (game->n >= MAX_PARTICLES)
		return;
	std::cout << "makeParticle() " << x << " " << y << std::endl;
	//position of particle
	Particle *p = &game->particle[game->n];
	p->s.center.x = x;
	p->s.center.y = y;
	p->velocity.y = rnd() * 0.5 - 0.25;
	p->velocity.x = rnd() * 0.5 - 0.25;
	game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
	static int savex = 0;
	static int savey = 0;
	//static int n = 0;

	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = WINDOW_HEIGHT - e->xbutton.y;
			for(int i=0;i<10;i++)
				makeParticle(game, e->xbutton.x, y);
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			return;
		}
	}
	//Did the mouse move?
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		savex = e->xbutton.x;
		savey = e->xbutton.y;
		
		int y = WINDOW_HEIGHT - e->xbutton.y;
		if(game->bubbler ==0)
		{    
			game->mouse[0] = savex;
			game->mouse[1] = y;
		}
		for(int i=0;i<10;i++)
			makeParticle(game, e->xbutton.x, y);
		//if (++n < 10)
			//return;
	}
}

int check_keys(XEvent *e, Game *game)
{
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
		
		if(key==XK_b)
		{
		    game->bubbler ^= 1;
		
		}    
		
		
		if (key == XK_Escape) {
			return 1;
		}
		
		//You may check other keys here.



	}
	return 0;
}

void movement(Game *game)
{
	Particle *p;

	if (game->n <= 0)
		return;

	
	if(game->bubbler != 0)
	{
	    for(int i=0; i<15; i++)
	    {
	    	makeParticle(game, game->mouse[0], game->mouse[1]);
	
	    }
	}
	
	
	
	for(int i=0;i<game->n; i++) 
	{
	    p = &game->particle[i];
	    p->velocity.y -= GRAVITY;
	    p->s.center.x += p->velocity.x;
	    p->s.center.y += p->velocity.y;

	    //check for collision with shapes...
	    Shape *s;	 
	    for(int j=0; j<6; j++)
            {		
	    s = &game->box[j];
	    
	    if(j==5)
	    {
	    	 if(p->s.center.y < 101 &&
		 p->s.center.x >= s->center.x - s->width &&
		 p->s.center.x <= s->center.x + s->width) 
	         {
		  	game->particle[i] = game->particle[--game->n];
	         }
	    
	    }	
	    
	    
	    else
 	    {	
	    
	    if(p->s.center.y < s->center.y + s->height &&
		    p->s.center.x >= s->center.x - s->width &&
		    p->s.center.x <= s->center.x + s->width) 
	    {
		p->s.center.y = s->center.y + s->height;
		p->velocity.y = -p->velocity.y * 0.5f;
		p->velocity.x += 0.005f;
	    }
	    
		
	    }
	    }
	    
	   
	    
	    
	    
	//check for off-screen
	if (p->s.center.y < 0.0) 
	{
		std::cout << "off screen" << std::endl;
		game->particle[i] = game->particle[--game->n];
	}

	}
}
void render(Game *game)
{
	float w, h, rad, cx, cy, x, y;
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw shapes...
	
	//draw circle
	Shape *s;
	glColor3ub(0,0,0);
	s = &game->circle;
	rad = s->radius;
	cx = s->center.x;
	cy = s->center.y;
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	glBegin(GL_TRIANGLE_FAN);
	for(int i =0; i<TRIANGLES; i++)
	{
	    x = rad * cos(i) - cx;
	    y = rad * sin(i) + cy;
	    glVertex3f(x+cx, y-cy, 0);

	    x = rad * cos(i+ 0.1) -cx;\
		y = rad* sin(i +0.1)+ cy;
	    glVertex3f(x+cx, y-cy, 0);
	}
	glEnd();
	glPopMatrix();
	
	
	for(int i=0; i<6; i++)
	{    
	//draw box
	Shape *s;
	if(i==0)
	glColor3ub(255,40,40);
		
	if(i==1)
	glColor3ub(255,128,40);

	if(i==2)
	glColor3ub(255,255,40);

	if(i==3)
	glColor3ub(40,255,40);

	if(i==4)
	glColor3ub(40,40,255);
	
	if(i==5)
	glColor3ub(25.5,204,255); 
	
	
	s = &game->box[i];
	
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	w = s->width;
	h = s->height;
	glBegin(GL_QUADS);
		glVertex2i(-w,-h);
		glVertex2i(-w, h);
		glVertex2i( w, h);
		glVertex2i( w,-h);
	glEnd();
	glPopMatrix();
	}
	//draw all particles here
	for(int i=0;i<game->n;i++) {
	    glPushMatrix();
	    glColor3ub(255,225,0);
	    Vec *c = &game->particle[i].s.center;
	    w = 2;
	    h = 2;
	    glBegin(GL_QUADS);
	    glVertex2i(c->x-w, c->y-h);
	    glVertex2i(c->x-w, c->y+h);
	    glVertex2i(c->x+w, c->y+h);
	    glVertex2i(c->x+w, c->y-h);
	    glEnd();
	    glPopMatrix();

	}


	//fonts
	Rect r;
	r.bot = WINDOW_HEIGHT -50;
	r.left = 10;
	r.center = 0;
	ggprint16(&r, 0, 0x00ffffff, "WATERFALL MODEL");

	char words[5][20] = {"REQUIREMENTS" , "DESIGN" , "CODING" , "TESTING" , "MAINTENANCE"};
	
	for(int i=0; i<5; i++)
	{
	    r.bot = game->box[i].center.y-15;
	    r.left = game->box[i].center.x-80;
	    ggprint16(&r, 0, 0x00000000, words[i]);
	} 


	r.bot = 20;
	r.left = 440;
	ggprint16(&r, 0, 0x00ffffff, "POT O' GOLD");
}



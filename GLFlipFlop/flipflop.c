/* flipflop, Copyright (c) 2003 Kevin Ogden <kogden1@hotmail.com>
 *                     (c) 2006 Sergio Guti�rrez "Sergut" <sergut@gmail.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *
 *
 * 2003 Kevin Odgen                  First version
 * 2006 Sergio Guti�rrez "Sergut"    Made several parameters dynamic and selectable
 *                                   from the command line: size of the board, 
 *                                   rotation speed and number of free squares; also
 *                                   added the "sticks" mode.
 *
 */

#define DEF_FLIPFLOP_MODE  "tiles" /* Default mode (options: "tiles", "sticks") */
#define DEF_BOARD_X_SIZE   "9"     /* Default width of the board */
#define DEF_BOARD_Y_SIZE   "9"     /* Default length of the board */

#define DEF_BOARD_SIZE     "0"     /* "0" means "no value selected by user". It is changed */ 
#define DEF_NUMSQUARES     "0"     /* in function init_flipflop() to its correct value (that */ 
#define DEF_FREESQUARES    "0"     /* is a function of the size of the board and the mode)*/

#define DEF_SPIN           "0.1"   /* Default angular velocity: PI/10 rads/s    */

#define DEF_STICK_THICK   54       /* Thickness for the sticks mode (over 100)  */
#define DEF_STICK_RATIO   80       /* Ratio of sticks/total squares (over 100)  */
#define DEF_TILE_THICK     4       /* Thickness for the tiles mode (over 100)   */
#define DEF_TILE_RATIO    95       /* Ratio of tiles/total squares (over 100)   */

#ifdef USE_GL

#include "gltrackball.h"
#include "yarandom.h"

#include "blanker.h"
#include <GL/gl.h>
#include <math.h> /* for M_PI */
#include <proto/exec.h>

#undef countof
#define countof(x) (sizeof((x))/sizeof((*x)))

/*static XrmOptionDescRec opts[] = {
    {"-sticks",         ".mode",            XrmoptionNoArg,  "sticks"},
    {"-tiles",          ".mode",            XrmoptionNoArg,  "tiles" },
    {"-mode",           ".mode",            XrmoptionSepArg, 0       },
    {"-size",           ".size",            XrmoptionSepArg, 0       },
    {"-size-x",         ".size_x",          XrmoptionSepArg, 0       },
    {"-size-y",         ".size_y",          XrmoptionSepArg, 0       },
    {"-count",          ".numsquares",      XrmoptionSepArg, 0       },
    {"-free",           ".freesquares",     XrmoptionSepArg, 0       },
    {"-spin",           ".spin",            XrmoptionSepArg, 0       },
};*/

static int wire = 0, clearbits = 0;
static int board_x_size = 9, board_y_size = 9, board_avg_size = 0;
static int numsquares, freesquares;
static float half_thick;
static float spin = 0.1;
static char* flipflopmode_str="tiles";

/*static argtype vars[] = {
    { &flipflopmode_str, "mode",        "Mode",     DEF_FLIPFLOP_MODE,  t_String},
    { &board_avg_size,   "size",        "Integer",  DEF_BOARD_SIZE,     t_Int},
    { &board_x_size,     "size_x",      "Integer",  DEF_BOARD_X_SIZE,   t_Int},
    { &board_y_size,     "size_y",      "Integer",  DEF_BOARD_Y_SIZE,   t_Int},
    { &numsquares,       "numsquares",  "Integer",  DEF_NUMSQUARES,     t_Int},
    { &freesquares,      "freesquares", "Integer",  DEF_NUMSQUARES,     t_Int},
    { &spin,             "spin",        "Float",    DEF_SPIN,           t_Float},
};

ENTRYPOINT ModeSpecOpt flipflop_opts = {countof(opts), opts, countof(vars), vars, NULL};*/

/*#ifdef USE_MODULES
ModStruct   flipflop_description =
    {"flipflop", "init_flipflop", "draw_flipflop", "release_flipflop",
     "draw_flipflop", "init_flipflop", NULL, &flipflop_opts,
     1000, 1, 2, 1, 4, 1.0, "",
     "Flipflop", 0, NULL};

#endif*/ /* USE_MODULES */

typedef struct {
    /* array specifying which squares are where (to avoid collisions) */
    /* -1 means empty otherwise integer represents square index 0 - n-1 */
	/* occupied[x*board_y_size+y] is the tile [x][y] (i.e. that starts at column x and row y)*/
    int *occupied; /* size: size_x * size_y */
    /* an array of xpositions of the squares */
    int *xpos; /* size: numsquares */
    /* array of y positions of the squares */
    int *ypos; /* size: numsquares */
    /* integer representing the direction of movement of a square */
    int *direction; /* 0 not, 1 x+, 2 y+, 3 x-, 4 y-*/ /* size: numsquares */
    /* angle of moving square (during a flip) */
    float *angle; /* size: numsquares */
    /* array of colors for a square (RGB) */
    /* eg. color[ 3*3 + 0 ] is the red   component of square 3 */
    /* eg. color[ 4*3 + 1 ] is the green component of square 4 */
    /* eg. color[ 5*3 + 2 ] is the blue  component of square 5 */
    /*            ^-- n is the number of square */
    float *color; /* size: numsquares * 3 */
} randsheet; 	

typedef struct {
	//GLXContext *glx_context;
	//Window window;
    trackball_state *trackball;
	BOOL button_down_p;

	randsheet *sheet;

    float theta;      /* angle of rotation of the board                */
    float flipspeed;  /* amount of flip;  1 is a entire flip           */
    float reldist;    /* relative distace of camera from center        */
    float energy;     /* likelyhood that a square will attempt to move */

} Flipflopcreen;

static Flipflopcreen qs;

static void randsheet_create( randsheet *rs );
static void randsheet_initialize( randsheet *rs );
static void randsheet_free( randsheet *rs );
static int  randsheet_new_move( randsheet* rs );
static void randsheet_move( randsheet *rs, float rot );
static void randsheet_draw( randsheet *rs );
static void setup_lights(void);
static void drawBoard(Flipflopcreen *);
static void display(Flipflopcreen *c);
static void draw_sheet(void);


/* configure lighting */
static void
setup_lights(void)
{
    /*   GLfloat position0[] = { board_avg_size*0.5, board_avg_size*0.1, board_avg_size*0.5, 1.0 }; */

    /*   GLfloat position0[] = { -board_avg_size*0.5, 0.2*board_avg_size, -board_avg_size*0.5, 1.0 }; */
    GLfloat position0[4];
    position0[0] = 0;
    position0[1] = board_avg_size*0.3;
    position0[2] = 0;
    position0[3] = 1;

    if (wire) return;

    glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT0, GL_POSITION, position0);
    glEnable(GL_LIGHT0);
}

/*ENTRYPOINT Bool
flipflop_handle_event (ModeInfo *mi, XEvent *event)
{
    Flipflopcreen *c = &qs[MI_SCREEN(mi)];

    if (event->xany.type == ButtonPress &&
        event->xbutton.button == Button1)
        {
            c->button_down_p = True;
            gltrackball_start (c->trackball,
                               event->xbutton.x, event->xbutton.y,
                               MI_WIDTH (mi), MI_HEIGHT (mi));
            return True;
        }
    else if (event->xany.type == ButtonRelease &&
             event->xbutton.button == Button1)
        {
            c->button_down_p = False;
            return True;
        }
    else if (event->xany.type == ButtonPress &&
             (event->xbutton.button == Button4 ||
              event->xbutton.button == Button5))
        {
            gltrackball_mousewheel (c->trackball, event->xbutton.button, 5,
                                    !event->xbutton.state);
            return True;
        }
    else if (event->xany.type == MotionNotify &&
             c->button_down_p)
        {
            gltrackball_track (c->trackball,
                               event->xmotion.x, event->xmotion.y,
                               MI_WIDTH (mi), MI_HEIGHT (mi));
            return True;
        }

    return False;
}*/

/* draw board */
static void
drawBoard(Flipflopcreen *c)
{
    int i;
    for( i=0; i < (c->energy) ; i++ ) {
        randsheet_new_move( c->sheet );
	}
    randsheet_move( c->sheet, c->flipspeed * 3.14159 );
    randsheet_draw( c->sheet );
}


static void
display(Flipflopcreen *c)
{
    GLfloat amb[] = { 0.8, 0.8, 0.8, 1.0 };

    glClear(clearbits);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.2);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.15/board_avg_size );
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.15/board_avg_size );
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);

    /** setup perspectif */
    glTranslatef(0.0, 0.0, -c->reldist*board_avg_size);
    glRotatef(22.5, 1.0, 0.0, 0.0);  
    gltrackball_rotate (c->trackball);
    glRotatef(c->theta*100, 0.0, 1.0, 0.0);
    glTranslatef(-0.5*board_x_size, 0.0, -0.5*board_y_size); /* Center the board */

    drawBoard(c);

    if (!c->button_down_p) {
        c->theta += .01 * spin;
	}

}

void reshape_flipflop(int width, int height)
{
    GLfloat h = (GLfloat) height / (GLfloat) width;
    glViewport(0,0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, 1/h, 1.0, 300.0);
    glMatrixMode(GL_MODELVIEW);
}

void init_flipflop(struct BlankerData* bd)
{
    Flipflopcreen *c;

	/* Set all constants to their correct values */
	if (board_avg_size != 0) {  /* general size specified by user */
		board_x_size = board_avg_size;
		board_y_size = board_avg_size;
	} else {
		board_avg_size = (board_x_size + board_y_size) / 2;
	}
	if ((numsquares == 0) && (freesquares != 0)) {
		numsquares = board_x_size * board_y_size - freesquares; 
	}
	if (strcmp(flipflopmode_str, "tiles")) {
		half_thick = 1.0 * DEF_STICK_THICK / 100.0; 
		if (numsquares == 0) {  /* No value defined by user */
			numsquares = board_x_size * board_y_size * DEF_STICK_RATIO / 100;
		}
	} else {
		half_thick = 1.0 * DEF_TILE_THICK / 100.0; 
		if (numsquares == 0) {  /* No value defined by user */
			numsquares = board_x_size * board_y_size * DEF_TILE_RATIO/ 100;;
		}
	}
	if (board_avg_size < 2) {
//		  fprintf (stderr,"%s: the board must be at least 2x2.\n", progname);
		exit(1);
	}
	if ((board_x_size < 1) || (board_y_size < 1) ||	(numsquares < 1)) {
//		  fprintf (stderr,"%s: the number of elements ('-count') and the dimensions of the board ('-size-x', '-size-y') must be positive integers.\n", progname);
		exit(1);
	}
	if (board_x_size * board_y_size <= numsquares) {
//		  fprintf (stderr,"%s: the number of elements ('-count') that you specified is too big \n for the dimensions of the board ('-size-x', '-size-y'). Nothing will move.\n", progname);
	}

	wire = FALSE;

	c = &qs;
    c->trackball = gltrackball_init ();

    c->flipspeed = 0.03;
    c->reldist = 1;
    c->energy = 40;

	/* At this point, all the constants have already been set, */
	/* so we can create the board */
	c->sheet = (randsheet*) IExec->AllocVec(sizeof(randsheet), MEMF_PRIVATE);
    randsheet_create( c->sheet ); 

    glClearColor(0.0, 0.0, 0.0, 0.0);

    clearbits = GL_COLOR_BUFFER_BIT;

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    setup_lights();

    glEnable(GL_DEPTH_TEST);
    clearbits |= GL_DEPTH_BUFFER_BIT;
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    randsheet_initialize( c->sheet );
}

void draw_flipflop()
{
	Flipflopcreen *c = &qs;

    display(c);

    glFinish();
}

void release_flipflop()
{
	Flipflopcreen *c = &qs;
	if (c->sheet) {
		randsheet_free(c->sheet);
		IExec->FreeVec(c->sheet);
		c->sheet = 0;
	}
}

/*** ADDED RANDSHEET FUNCTIONS ***/

static void
draw_sheet(void)
{
    glBegin( wire ? GL_LINE_LOOP : GL_QUADS );

    glNormal3f( 0, -1, 0 );
    glVertex3f( half_thick,  -half_thick,  half_thick );
    glVertex3f( 1-half_thick,   -half_thick, half_thick );
    glVertex3f( 1-half_thick, -half_thick,  1-half_thick);
    glVertex3f( half_thick, -half_thick, 1-half_thick );

    if (wire) { glEnd(); glBegin (GL_LINE_LOOP); }

    /* back */
    glNormal3f( 0, 1, 0 );
    glVertex3f( half_thick, half_thick, 1-half_thick );
    glVertex3f( 1-half_thick, half_thick,  1-half_thick);
    glVertex3f( 1-half_thick,   half_thick, half_thick );
    glVertex3f( half_thick,  half_thick,  half_thick );

    if (wire) { glEnd(); return; }

    /* 4 edges!!! weee.... */
    glNormal3f( 0, 0, -1 );
    glVertex3f( half_thick, half_thick, half_thick );
    glVertex3f( 1-half_thick, half_thick, half_thick );
    glVertex3f( 1-half_thick, -half_thick, half_thick );
    glVertex3f( half_thick, -half_thick, half_thick );
    glNormal3f( 0, 0, 1 );
    glVertex3f( half_thick, half_thick, 1-half_thick );
    glVertex3f( half_thick, -half_thick, 1-half_thick );
    glVertex3f( 1-half_thick, -half_thick, 1-half_thick );
    glVertex3f( 1-half_thick, half_thick, 1-half_thick );
    glNormal3f( 1, 0, 0 );
    glVertex3f( 1-half_thick, half_thick, 1-half_thick );
    glVertex3f( 1-half_thick, -half_thick, 1-half_thick );
    glVertex3f( 1-half_thick, -half_thick, half_thick );
    glVertex3f( 1-half_thick, half_thick, half_thick );
    glNormal3f( -1, 0, 0 );
    glVertex3f( half_thick, half_thick, 1-half_thick );
    glVertex3f( half_thick, half_thick, half_thick );
    glVertex3f( half_thick, -half_thick, half_thick );
    glVertex3f( half_thick, -half_thick, 1-half_thick );
    glEnd();
}

/* Reserve memory for the randsheet */
static void
randsheet_create( randsheet *rs )
{
	rs -> occupied  = (int*) IExec->AllocVec(board_x_size*board_y_size * sizeof(int), MEMF_PRIVATE);
	rs -> xpos      = (int*) IExec->AllocVec(numsquares * sizeof(int), MEMF_PRIVATE);
	rs -> ypos      = (int*) IExec->AllocVec(numsquares * sizeof(int), MEMF_PRIVATE);
	rs -> direction = (int*) IExec->AllocVec(numsquares * sizeof(int), MEMF_PRIVATE);
	rs -> angle     = (float*) IExec->AllocVec(numsquares * sizeof(float), MEMF_PRIVATE);
	rs -> color     = (float*) IExec->AllocVec(numsquares*3 * sizeof(float), MEMF_PRIVATE);
}

/* Free reserved memory for the randsheet */
static void
randsheet_free( randsheet *rs )
{
	IExec->FreeVec(rs->occupied);
	IExec->FreeVec(rs->xpos);
	IExec->FreeVec(rs->ypos);
	IExec->FreeVec(rs->direction);
	IExec->FreeVec(rs->angle);
	IExec->FreeVec(rs->color);
}

static void
randsheet_initialize( randsheet *rs )
{
    int i, j, index;
    index = 0;
    /* put the moving sheets on the board */
    for( i = 0; i < board_x_size; i++ )
        {
            for( j = 0; j < board_y_size; j++ )
                {
                    /* initially fill up a corner with the moving squares */
                    if( index < numsquares )
                        {
                            rs->occupied[ i * board_y_size + j ] = index;
                            rs->xpos[ index ] = i;
                            rs->ypos[ index ] = j;
                            /* have the square colors start out as a pattern */
                            rs->color[ index*3 + 0 ] = ((i+j)%3 == 0)||((i+j+1)%3 == 0);
                            rs->color[ index*3 + 1 ] = ((i+j+1)%3 == 0);
                            rs->color[ index*3 + 2 ] = ((i+j+2)%3 == 0);
                            index++;
                        }
                    /* leave everything else empty*/
                    else
                        {
                            rs->occupied[ i * board_y_size + j ] = -1;
                        }
                }
        }
    /* initially everything is at rest */
    for( i=0; i<numsquares; i++ )
        {
            rs->direction[ i ] = 0;
            rs->angle[ i ] = 0;
        }
}

/* Pick and random square and direction and try to move it. */
/* May not actually move anything, just attempt a random move. */
/* Returns true if move was sucessful. */
/* This could probably be implemented faster in a dequeue */
/* to avoid trying to move a square which is already moving */
/* but speed is most likely bottlenecked by rendering anyway... */
static int
randsheet_new_move( randsheet* rs )
{
    int i, j;
    int num, dir;
    /* pick a random square */
    num = random( ) % numsquares;
    i = rs->xpos[ num ];
    j = rs->ypos[ num ];
    /* pick a random direction */
    dir = ( random( )% 4 ) + 1;

    if( rs->direction[ num ] == 0 )
        {
            switch( dir )
                {
                case 1:
                    /* move up in x */
                    if( ( i + 1 ) < board_x_size )
                        {
                            if( rs->occupied[ (i + 1) * board_y_size + j ] == -1 )
                                {
                                    rs->direction[ num ] = dir;
                                    rs->occupied[ (i + 1) * board_y_size + j ] = num;
                                    rs->occupied[ i * board_y_size + j ] = -1;
                                    return 1;
                                }
                        }
                    return 0;
                    break;
                case 2:
                    /* move up in y */
                    if( ( j + 1 ) < board_y_size )
                        {
                            if( rs->occupied[ i * board_y_size + (j + 1) ] == -1 )
                                {
                                    rs->direction[ num ] = dir;
                                    rs->occupied[ i * board_y_size + (j + 1) ] = num;
                                    rs->occupied[ i * board_y_size + j ] = -1;
                                    return 1;
                                }
                        }
                    return 0;
                    break;
                case 3:
                    /* move down in x */
                    if( ( i - 1 ) >= 0 )
                        {
                            if( rs->occupied[ (i - 1) * board_y_size + j ] == -1 )
                                {
                                    rs->direction[ num ] = dir;
                                    rs->occupied[ (i - 1) * board_y_size + j ] = num;
                                    rs->occupied[ i * board_y_size + j ] = -1;
                                    return 1;
                                }
                        }
                    return 0;
                    break;
                case 4:
                    /* move down in y */
                    if( ( j - 1 ) >= 0 )
                        {
                            if( rs->occupied[ i * board_y_size + (j - 1) ] == -1 )
                                {
                                    rs->direction[ num ] = dir;
                                    rs->occupied[ i * board_y_size + (j - 1) ] = num;
                                    rs->occupied[ i * board_y_size + j ] = -1;
                                    return 1;
                                }
                        }
                    return 0;
                    break;
                default:
                    break;
                }
        }
    return 0;
}

/*   move a single frame.  */
/*   Pass in the angle in rads the square rotates in a frame. */
static void
randsheet_move( randsheet *rs, float rot )
{
    int i, j, index;
    for( index = 0 ; index < numsquares; index++ )
        {
            i = rs->xpos[ index ];
            j = rs->ypos[ index ];
            switch( rs->direction[ index ] )
                {
                case 0:
                    /* not moving */
                    break;
                case 1:
                    /* move up in x */
                    rs->angle[ index ] += rot;
                    /* check to see if we have finished moving */
                    if( rs->angle[ index ] >= M_PI  )
                        {
                            rs->xpos[ index ] += 1;
                            rs->direction[ index ] = 0;
                            rs->angle[ index ] = 0;
                        }
                    break;
                case 2:
                    /* move up in y */
                    rs->angle[ index ] += rot;
                    /* check to see if we have finished moving */
                    if( rs->angle[ index ] >= M_PI  )
                        {
                            rs->ypos[ index ] += 1;
                            rs->direction[ index ] = 0;
                            rs->angle[ index ] = 0;
                        }
                    break;
                case 3:
                    /* down in x */
                    rs->angle[ index ] += rot;
                    /* check to see if we have finished moving */
                    if( rs->angle[ index ] >= M_PI  )
                        {
                            rs->xpos[ index ] -= 1;
                            rs->direction[ index ] = 0;
                            rs->angle[ index ] = 0;
                        }
                    break;
                case 4:
                    /* up in x */
                    rs->angle[ index ] += rot;
                    /* check to see if we have finished moving */
                    if( rs->angle[ index ] >= M_PI  )
                        {
                            rs->ypos[ index ] -= 1;
                            rs->direction[ index ] = 0;
                            rs->angle[ index ] = 0;
                        }
                    break;
                default:
                    break;
                }
        }
}


/* draw all the moving squares  */
static void
randsheet_draw( randsheet *rs )
{
    int i, j;
    int index;
    /* for all moving squares ... */
    for( index = 0; index < numsquares; index++ )
        {
            /* set color */
            glColor3f( rs->color[ index*3 + 0 ],
                       rs->color[ index*3 + 1 ],
                       rs->color[ index*3 + 2 ] );
            /* find x and y position */
            i = rs->xpos[ index ];
            j = rs->ypos[ index ];
            glPushMatrix();
            switch( rs->direction[ index ] )
                {
                case 0:

                    /* not moving */
                    /* front */
                    glTranslatef( i, 0, j );
                    break;
                case 1:
                    glTranslatef( i+1, 0, j );
                    glRotatef( 180 - rs->angle[ index ]*180/M_PI, 0, 0, 1 );

                    break;
                case 2:
                    glTranslatef( i, 0, j+1 );
                    glRotatef( 180 - rs->angle[ index ]*180/M_PI, -1, 0, 0 );

                    break;
                case 3:
                    glTranslatef( i, 0, j );
                    glRotatef( rs->angle[ index ]*180/M_PI, 0, 0, 1 );
                    break;
                case 4:
                    glTranslatef( i, 0, j );
                    glRotatef( rs->angle[ index ]*180/M_PI, -1, 0, 0 );
                    break;
                default:
                    break;
                }
            draw_sheet();
            glPopMatrix();

        }
}

/**** END RANDSHEET_BAK FUNCTIONS ***/

#endif /* USE_GL */

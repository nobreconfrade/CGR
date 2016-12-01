/*

  main.c for glmaze

  (c) 2001 by Till Harbaum

 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>

#include "maze.h"

/* This is our SDL surface */
SDL_Surface *surface;

/* Ambient Light Values */
GLfloat LightAmbient[]  = { 0.2f, 0.2f, 0.2f, 1.0f };
/* Diffuse Light Values */
GLfloat LightDiffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
/* Light Position */
GLfloat LightPosition[] = { 0.0f, 0.0f, 1.0f, 1.0f };

GLuint texture[MAX_TEX]; /* Storage for 3 textures */

GLint me = MASTER;
PLAYER player[MAX_PLAYER];

int game_state = STATE_WAIT;  // game is in wait mode

int screen_width_tab[]  = { 640, 800, 1024, 1280, 0} ;
int screen_height_tab[] = { 480, 600,  768, 1024, 0} ;
int screen_width, screen_height;

void init_object(MAPPED_OBJECT *obj, float *pos) {

  obj->pos[OB_X] = pos[OB_X]; 
  obj->pos[OB_Y] = pos[OB_Y]; 
  obj->pos[OB_Z] = pos[OB_Z];

  obj->x_vec[OB_X] = -1.0; obj->x_vec[OB_Y] =  0.0; obj->x_vec[OB_Z] = 0.0; 
  obj->y_vec[OB_X] =  0.0; obj->y_vec[OB_Y] = -1.0; obj->y_vec[OB_Z] = 0.0; 
  obj->z_vec[OB_X] =  0.0; obj->z_vec[OB_Y] =  0.0; obj->z_vec[OB_Z] = pos[3];
}

void rotate_object(MAPPED_OBJECT *obj, int axis, float angle) {
  float  tmp;
  float  dsin = sin(angle * PIOVER180);
  float  dcos = cos(angle * PIOVER180);

  if(axis == OB_X) {

    tmp              = dcos * obj->x_vec[OB_Z] - dsin * obj->x_vec[OB_Y];
    obj->x_vec[OB_Y] = dsin * obj->x_vec[OB_Z] + dcos * obj->x_vec[OB_Y];
    obj->x_vec[OB_Z] = tmp;

    tmp              = dcos * obj->y_vec[OB_Z] - dsin * obj->y_vec[OB_Y];
    obj->y_vec[OB_Y] = dsin * obj->y_vec[OB_Z] + dcos * obj->y_vec[OB_Y];
    obj->y_vec[OB_Z] = tmp;

    tmp              = dcos * obj->z_vec[OB_Z] - dsin * obj->z_vec[OB_Y];
    obj->z_vec[OB_Y] = dsin * obj->z_vec[OB_Z] + dcos * obj->z_vec[OB_Y];
    obj->z_vec[OB_Z] = tmp;

  } else if(axis == OB_Y) {

    tmp              = dcos * obj->x_vec[OB_X] - dsin * obj->x_vec[OB_Z];
    obj->x_vec[OB_Z] = dsin * obj->x_vec[OB_X] + dcos * obj->x_vec[OB_Z];
    obj->x_vec[OB_X] = tmp;

    tmp              = dcos * obj->y_vec[OB_X] - dsin * obj->y_vec[OB_Z];
    obj->y_vec[OB_Z] = dsin * obj->y_vec[OB_X] + dcos * obj->y_vec[OB_Z];
    obj->y_vec[OB_X] = tmp;

    tmp              = dcos * obj->z_vec[OB_X] - dsin * obj->z_vec[OB_Z];
    obj->z_vec[OB_Z] = dsin * obj->z_vec[OB_X] + dcos * obj->z_vec[OB_Z];
    obj->z_vec[OB_X] = tmp;

  } else {

    tmp              = dcos * obj->x_vec[OB_X] - dsin * obj->x_vec[OB_Y];
    obj->x_vec[OB_Y] = dsin * obj->x_vec[OB_X] + dcos * obj->x_vec[OB_Y];
    obj->x_vec[OB_X] = tmp;

    tmp              = dcos * obj->y_vec[OB_X] - dsin * obj->y_vec[OB_Y];
    obj->y_vec[OB_Y] = dsin * obj->y_vec[OB_X] + dcos * obj->y_vec[OB_Y];
    obj->y_vec[OB_X] = tmp;

    tmp              = dcos * obj->z_vec[OB_X] - dsin * obj->z_vec[OB_Y];
    obj->z_vec[OB_Y] = dsin * obj->z_vec[OB_X] + dcos * obj->z_vec[OB_Y];
    obj->z_vec[OB_X] = tmp;
  }
}

/* function to release/destroy our resources and restoring the old desktop */
void Quit( int returnCode )
{
  SavePrefs();

  FreeControl();
  FreeShots();
  FreePlayer();

  FreeSound();
  
  /* clean up the window */
  SDL_Quit( );
  
  FreeNet();
  
  /* and exit appropriately */
  exit( returnCode );
}

/* function to reset our viewport after a window resize */
int resizeWindow( int width, int height ) {

  /* Height / width ration */
  GLfloat ratio;

  /* Protect against a divide by zero */
  if ( height == 0 ) height = 1;

  ratio = ( GLfloat )width / ( GLfloat )height;

  /* Setup our viewport. */
  glViewport( 0, 0, ( GLint )width, ( GLint )height );
  
  /* change to the projection matrix and set our viewing volume. */
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity( );

  /* Set our perspective */
  gluPerspective( 45.0f, ratio, 0.1f, 100.0f );
  
  /* Make sure we're chaning the model view and not the projection */
  glMatrixMode( GL_MODELVIEW );

  /* Reset The View */
  glLoadIdentity( );
  
  return( TRUE );
}

int check_step(MAPPED_OBJECT obj, float radius) {
  int x  = floor(obj.pos[OB_X]);
  int y  = floor(obj.pos[OB_Y]);
  int z  = floor(obj.pos[OB_Z]);

  float xi = obj.pos[OB_X] - floor(obj.pos[OB_X]);
  float yi = obj.pos[OB_Y] - floor(obj.pos[OB_Y]);
  float zi = obj.pos[OB_Z] - floor(obj.pos[OB_Z]);

#ifndef SQUARE_CHECK
  float sq_radius = SQ(radius);
  float sq_xi  = SQ(xi),     sq_yi  = SQ(yi),     sq_zi  = SQ(zi);
  float sq_1xi = SQ(1.0-xi), sq_1yi = SQ(1.0-yi), sq_1zi = SQ(1.0-zi);
#endif

  if((x>=0)&&(x<extend[OB_X])&&
     (y>=0)&&(y<extend[OB_Y])&&
     (z>=0)&&(z<extend[OB_Z])) {

    /* 6 direct neighbouring walls */
    if( wall[OB_X][x  ][z][y] && (xi < radius)) return 0;
    if( wall[OB_X][x+1][z][y] && (xi > (1.0-radius)  )) return 0;

    if( wall[OB_Y][y  ][z][x] && (yi < radius)) return 0;
    if( wall[OB_Y][y+1][z][x] && (yi > (1.0-radius)))   return 0;

    if( wall[OB_Z][z  ][y][x] && (zi < radius)) return 0;
    if( wall[OB_Z][z+1][y][x] && (zi > (1.0-radius)  )) return 0;

    /* 12 neighbouring borders */
#ifndef SQUARE_CHECK
    if(sq_xi + sq_yi < sq_radius) {
#else
    if((xi < radius) && (yi < radius)) {
#endif 
      if( wall[OB_X][x][z][y-1] ) return 0;
      if( wall[OB_Y][y][z][x-1] ) return 0;
    }

#ifndef SQUARE_CHECK
    if(sq_xi + sq_1yi < sq_radius) {
#else
    if((xi < radius) && (yi > (1.0-radius))) {
#endif 
      if( wall[OB_X][x][z][y+1] ) return 0;
      if( wall[OB_Y][y+1][z][x-1] ) return 0;
    }

#ifndef SQUARE_CHECK
    if(sq_1xi + sq_yi < sq_radius) {
#else
    if((xi > (1.0-radius))   && (yi < radius)) {
#endif 
      if( wall[OB_X][x+1][z][y-1] ) return 0;
      if( wall[OB_Y][y][z][x+1] ) return 0;
    }

#ifndef SQUARE_CHECK
    if(sq_1xi + sq_1yi < sq_radius) {
#else
    if((xi > (1.0-radius))   && (yi > (1.0-radius))) {
#endif 
      if( wall[OB_X][x+1][z][y+1] ) return 0;
      if( wall[OB_Y][y+1][z][x+1] ) return 0;
    }


#ifndef SQUARE_CHECK
    if(sq_xi + sq_zi < sq_radius) {
#else
    if((xi < radius) && (zi < radius)) {
#endif 
      if( wall[OB_X][x][z-1][y] ) return 0;
      if( wall[OB_Z][z][y][x-1] ) return 0;
    }

#ifndef SQUARE_CHECK
    if(sq_xi + sq_1zi < sq_radius) {
#else
    if((xi < radius) && (zi > (1.0-radius))) {
#endif 
      if( wall[OB_X][x][z+1][y] ) return 0;
      if( wall[OB_Z][z+1][y][x-1] ) return 0;
    }

#ifndef SQUARE_CHECK
    if(sq_1xi + sq_zi < sq_radius) {
#else
    if((xi > (1.0-radius))   && (zi < radius)) {
#endif 
      if( wall[OB_X][x+1][z-1][y] ) return 0;
      if( wall[OB_Z][z][y][x+1] )   return 0;
    }

#ifndef SQUARE_CHECK
    if(sq_1xi + sq_1zi < sq_radius) {
#else
    if((xi > (1.0-radius))   && (zi > (1.0-radius))) {
#endif 
      if( wall[OB_X][x+1][z+1][y] ) return 0;
      if( wall[OB_Z][z+1][y][x+1] ) return 0;
    }


#ifndef SQUARE_CHECK
    if(sq_yi + sq_zi < sq_radius) {
#else
    if((yi < radius) && (zi < radius)) {
#endif 
      if( wall[OB_Y][y][z-1][x] ) return 0;
      if( wall[OB_Z][z][y-1][x] ) return 0;
    }

#ifndef SQUARE_CHECK
    if(sq_yi + sq_1zi < sq_radius) {
#else
    if((yi < radius) && (zi > (1.0-radius))) {
#endif 
      if( wall[OB_Y][y][z+1][x] ) return 0;
      if( wall[OB_Z][z+1][y-1][x] ) return 0;
    }

#ifndef SQUARE_CHECK
    if(sq_1yi + sq_zi < sq_radius) {
#else
    if((yi > (1.0-radius))   && (zi < radius)) {
#endif 
      if( wall[OB_Y][y+1][z-1][x] ) return 0;
      if( wall[OB_Z][z][y+1][x] ) return 0;
    }

#ifndef SQUARE_CHECK
    if(sq_1yi + sq_1zi < sq_radius) {
#else
    if((yi > (1.0-radius))   && (zi > (1.0-radius))) {
#endif 
      if( wall[OB_Y][y+1][z+1][x] ) return 0;
      if( wall[OB_Z][z+1][y+1][x] ) return 0;
    }

    /* 8 neighbouring edges */
#ifndef SQUARE_CHECK
    if(sq_xi + sq_yi + sq_zi < sq_radius) {
#else
    if((xi < radius) && (yi < radius) && (zi < radius)) {
#endif
      if(wall[OB_X][x][z-1][y-1]) return 0;
      if(wall[OB_Y][y][z-1][x-1]) return 0;
      if(wall[OB_Z][z][y-1][x-1]) return 0;
    }

#ifndef SQUARE_CHECK
    if(sq_1xi + sq_yi + sq_zi < sq_radius) {
#else
    if((xi > (1.0-radius))   && (yi < radius) && (zi < radius)) {
#endif
      if(wall[OB_X][x+1][z-1][y-1]) return 0;
      if(wall[OB_Y][y][z-1][x+1]) return 0;
      if(wall[OB_Z][z][y-1][x+1]) return 0;
    }

#ifndef SQUARE_CHECK
    if(sq_xi + sq_1yi + sq_zi < sq_radius) {
#else
    if((xi < radius) && (yi > (1.0-radius))   && (zi < radius)) {
#endif
      if(wall[OB_X][x][z-1][y+1]) return 0;
      if(wall[OB_Y][y+1][z-1][x-1]) return 0;
      if(wall[OB_Z][z][y+1][x-1]) return 0;
    }

#ifndef SQUARE_CHECK
    if(sq_xi + sq_yi + sq_1zi < sq_radius) {
#else
    if((xi < radius) && (yi < radius) && (zi > (1.0-radius))  ) {
#endif
      if(wall[OB_X][x][z+1][y-1]) return 0;
      if(wall[OB_Y][y][z+1][x-1]) return 0;
      if(wall[OB_Z][z+1][y-1][x-1]) return 0;
    }

#ifndef SQUARE_CHECK
    if(sq_1xi + sq_1yi + sq_zi < sq_radius) {
#else
    if((xi > (1.0-radius))   && (yi > (1.0-radius))   && (zi < radius)) {
#endif
      if(wall[OB_X][x+1][z-1][y+1]) return 0;
      if(wall[OB_Y][y+1][z-1][x+1]) return 0;
      if(wall[OB_Z][z][y+1][x+1]) return 0;
    }

#ifndef SQUARE_CHECK
    if(sq_xi + sq_1yi + sq_1zi < sq_radius) {
#else
    if((xi < radius) && (yi > (1.0-radius))   && (zi > (1.0-radius))  ) {
#endif
      if(wall[OB_X][x][z+1][y+1]) return 0;
      if(wall[OB_Y][y+1][z+1][x-1]) return 0;
      if(wall[OB_Z][z+1][y+1][x-1]) return 0;
    }

#ifndef SQUARE_CHECK
    if(sq_1xi + sq_yi + sq_1zi < sq_radius) {
#else
    if((xi > (1.0-radius))   && (yi < radius) && (zi > (1.0-radius))  ) {
#endif
      if(wall[OB_X][x+1][z+1][y-1]) return 0;
      if(wall[OB_Y][y][z+1][x+1]) return 0;
      if(wall[OB_Z][z+1][y-1][x+1]) return 0;
    }

#ifndef SQUARE_CHECK
    if(sq_1xi + sq_1yi + sq_1zi < sq_radius) {
#else
    if((xi > (1.0-radius))   && (yi > (1.0-radius))   && (zi > (1.0-radius))  ) {
#endif
      if(wall[OB_X][x+1][z+1][y+1]) return 0;
      if(wall[OB_Y][y+1][z+1][x+1]) return 0;
      if(wall[OB_Z][z+1][y+1][x+1]) return 0;
    }
  }

  return 1;
}

/* check if player collided with other player */
int player_collide(MAPPED_OBJECT map) { 
  int i;

  for(i=0;i<MAX_PLAYER;i++) {
    if((i!=me)&&(player[i].mapped)) {
      if( SQ(map.pos[OB_X]-player[i].map.pos[OB_X])+
	  SQ(map.pos[OB_Y]-player[i].map.pos[OB_Y])+
	  SQ(map.pos[OB_Z]-player[i].map.pos[OB_Z])
	  < SQ(2*PLAYER_RADIUS))
	return TRUE;
    }
  }
  return FALSE;
}

void add_step(float factor) {
  float f_sign = (factor<0)?-1.0:1.0;
  float f_abs = fabs(factor);

  while(f_abs > STEP_RES) {  
    player[me].map.pos[OB_X] -= f_sign * STEP_RES * player[me].map.x_vec[OB_Z];
    if(!check_step(player[me].map, PLAYER_RADIUS) || player_collide(player[me].map))
      player[me].map.pos[OB_X] += f_sign * STEP_RES * player[me].map.x_vec[OB_Z];

    player[me].map.pos[OB_Y] -= f_sign * STEP_RES * player[me].map.y_vec[OB_Z];
    if(!check_step(player[me].map, PLAYER_RADIUS) || player_collide(player[me].map))
      player[me].map.pos[OB_Y] += f_sign * STEP_RES * player[me].map.y_vec[OB_Z];

    player[me].map.pos[OB_Z] -= f_sign * STEP_RES * player[me].map.z_vec[OB_Z];
    if(!check_step(player[me].map, PLAYER_RADIUS) || player_collide(player[me].map))
      player[me].map.pos[OB_Z] += f_sign * STEP_RES * player[me].map.z_vec[OB_Z];

    f_abs -= STEP_RES;    
  }

  /* undo step */
  player[me].map.pos[OB_X] -= f_sign * f_abs * player[me].map.x_vec[OB_Z];
  if(!check_step(player[me].map, PLAYER_RADIUS) || player_collide(player[me].map))
    player[me].map.pos[OB_X] += f_sign * f_abs * player[me].map.x_vec[OB_Z];

  player[me].map.pos[OB_Y] -= f_sign * f_abs * player[me].map.y_vec[OB_Z];
  if(!check_step(player[me].map, PLAYER_RADIUS) || player_collide(player[me].map))
    player[me].map.pos[OB_Y] += f_sign * f_abs * player[me].map.y_vec[OB_Z];

  player[me].map.pos[OB_Z] -= f_sign * f_abs * player[me].map.z_vec[OB_Z];
  if(!check_step(player[me].map, PLAYER_RADIUS) || player_collide(player[me].map))
    player[me].map.pos[OB_Z] += f_sign * f_abs * player[me].map.z_vec[OB_Z];
}

/* grab/release mouse */
void mouse(int show) {
  SDL_ShowCursor( show );
}

 /* function to handle key press events */
void handleKeyPress( SDL_keysym *keysym, int pressed, int timeval, int videoFlags ) {
  static int cursor_reg = 0;

  if(keysym != NULL) {

    if (pressed) {
      if (( keysym->sym == SDLK_ESCAPE )||
	  ( keysym->sym == SDLK_SPACE )) {
	SoundPlay(SOUND_BUTTON, 0);

	if(game_state == STATE_ACTIVE) {
	  /* go into menu state (release mouse) */
	  mouse( TRUE );
	  game_state = STATE_PAUSED;
	} else {
	  /* only master can leave wait state */
	  if((me==MASTER)||(game_state == STATE_PAUSED)) {
	    if(me==MASTER) NetStart();

	    /* return to active state (grab mouse) */
	    mouse( FALSE );
	    game_state = STATE_ACTIVE;
	  }
	}
      }

      if ( keysym->sym == SDLK_RETURN ) {
	SoundPlay(SOUND_BUTTON, 0);

	if(game_state != STATE_ACTIVE) {
	  /* only master can leave wait state */
	  if((me==MASTER)||(game_state == STATE_PAUSED)) {
	    if(me==MASTER) NetStart();

	    /* return to active state (grab mouse) */
	    mouse( FALSE );
	    game_state = STATE_ACTIVE;
	  }
	}
      }

      if ( keysym->sym == SDLK_F1 ) {
	SoundPlay(SOUND_BUTTON, 0);

	SDL_WM_ToggleFullScreen( surface );
	prefs.fullscreen = !prefs.fullscreen;
      }

      if ( keysym->sym == SDLK_F2 ) {
	SoundPlay(SOUND_BUTTON, 0);

	prefs.mode++;
	if(screen_width_tab[prefs.mode]==0) prefs.mode=0;

	screen_width = screen_width_tab[prefs.mode];
	screen_height = screen_height_tab[prefs.mode];

	if(prefs.fullscreen) 	SDL_WM_ToggleFullScreen( surface );
	surface = SDL_SetVideoMode( screen_width, screen_height,
				    SCREEN_BPP, videoFlags );
	resizeWindow(screen_width, screen_height);
	if(prefs.fullscreen) 	SDL_WM_ToggleFullScreen( surface );

	if ( !surface ) {
	  fprintf( stderr, "Could not get a surface after resize: %s\n", 
		   SDL_GetError( ) );
	  Quit( 1 );
	}
      }

#ifdef SOUND
      if ( keysym->sym == SDLK_F3 ) {
	SoundPlay(SOUND_BUTTON, 0);
	prefs.sound = !prefs.sound;
      }

      if ( keysym->sym == SDLK_F4 ) {
	SoundPlay(SOUND_BUTTON, 0);
	prefs.music = !prefs.music;
	SoundMusic(); 
      }
#endif

      if ( keysym->sym == SDLK_F12) {
	SoundPlay(SOUND_BUTTON, 0);
        Quit( 0 );
      }
    }

    if ( keysym->sym == SDLK_RIGHT ) { 
      if(pressed) cursor_reg |=1; else cursor_reg &= ~1; }
    if ( keysym->sym == SDLK_LEFT )  { 
      if(pressed) cursor_reg |=2; else cursor_reg &= ~2; }
    
    if ( keysym->sym == SDLK_UP )    { 
      if(pressed) cursor_reg |=4; else cursor_reg &= ~4; }
    if ( keysym->sym == SDLK_DOWN )  { 
      if(pressed) cursor_reg |=8; else cursor_reg &= ~8; }
  }

  if(game_state == STATE_ACTIVE) {
    /* handle currently pressed keys */
    if((cursor_reg & 0x03) == 0x01) 
      rotate_object(&player[me].map, OB_Z, +0.2 * timeval);
    if((cursor_reg & 0x03) == 0x02) 
      rotate_object(&player[me].map, OB_Z, -0.2 * timeval);
    
    if((cursor_reg & 0x0c) == 0x04) 
      add_step( 0.002 * timeval);
    if((cursor_reg & 0x0c) == 0x08) 
      add_step(-0.002 * timeval);
  }
}

/* general OpenGL initialization function */
int initGL( float *start_pos ) {

  /* tell user something about his version of gl */
  printf("Renderer: %s Version %s\n", (char*)glGetString( GL_RENDERER),
	 (char*)glGetString( GL_VERSION));
  printf("Vendor:   %s\n", (char*)glGetString( GL_VENDOR ));

  /* finally load maze textures into gl */
  convert_textures();

  /* Enable Texture Mapping */
  glEnable( GL_TEXTURE_2D );
  
  /* init 2d and 3d stuff */
  InitControl();
  InitShots();
  InitPlayer();
  InitRadar();

  InitSound();
  SoundMusic();

  /* Set the title bar in environments that support it */
  SDL_WM_SetCaption("glMaze " VERSION " - (c) 2001 by Till Harbaum", NULL);
  
  /* Enable smooth shading */
  glShadeModel( GL_SMOOTH );
  
  /* Set the background black */
  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
  
  /* Depth buffer setup */
  glClearDepth( 1.0f );
  
  /* Enables Depth Testing */
  glEnable( GL_DEPTH_TEST );
  
  /* The Type Of Depth Test To Do */
  glDepthFunc( GL_LEQUAL );

  glShadeModel(GL_SMOOTH);	 // Enables Smooth Color Shading
  
  /* Really Nice Perspective Calculations */
  glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

  glLightfv( GL_LIGHT1, GL_AMBIENT, LightAmbient );
  glLightfv( GL_LIGHT1, GL_DIFFUSE, LightDiffuse );
  glLightfv( GL_LIGHT1, GL_POSITION, LightPosition );
  
  /* Enable Light One */
  glEnable( GL_LIGHT1 );
  glEnable(GL_LIGHTING);
  
  /* init universe xyz */
  init_object(&player[me].map, start_pos);
  
  return( TRUE );
}

/* Here goes our drawing code */
int drawGLScene( GLint timer )
{
  int x,y,z;
  GLfloat matrix[16];

  /* Clear The Screen And The Depth Buffer */
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  glLoadIdentity( );

  glShadeModel(GL_SMOOTH);			// Enables Smooth Color Shading
  
  /* Really Nice Perspective Calculations */
  glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

  /* Translate To players eyes */
  glTranslatef( 0, 0, 0.5 * PLAYER_RADIUS);

  /* Rotate Depending On Direction Player Is Facing */
  matrix[ 0] = player[me].map.x_vec[OB_X]; 
  matrix[ 1] = player[me].map.x_vec[OB_Y]; 
  matrix[ 2] = player[me].map.x_vec[OB_Z]; 
  matrix[ 3] = 0;

  matrix[ 4] = player[me].map.y_vec[OB_X]; 
  matrix[ 5] = player[me].map.y_vec[OB_Y]; 
  matrix[ 6] = player[me].map.y_vec[OB_Z]; 
  matrix[ 7] = 0;

  matrix[ 8] = player[me].map.z_vec[OB_X]; 
  matrix[ 9] = player[me].map.z_vec[OB_Y]; 
  matrix[10] = player[me].map.z_vec[OB_Z]; 
  matrix[11] = 0;

  matrix[12] = 0;                      
  matrix[13] = 0;              
  matrix[14] = 0;                      
  matrix[15] = 1;

  glMultMatrixf(matrix);

  glEnable(GL_LIGHTING);

  /* Translate The Scene Based On Player Position */
  glTranslatef( -player[me].map.pos[OB_X], 
		-player[me].map.pos[OB_Y], 
		-player[me].map.pos[OB_Z] );
  
  glDisable(GL_BLEND);                        /* Turn Blending Off */

  /* process all Z tiles (top and bottom faces) */
  for(y=0;y<32;y++) {
    for(x=0;x<32;x++) {
      for(z=0;z<32;z++) {
	if(wall[OB_Z][z][y][x]!=0) {

	  /* Bind the proper texture to the triangle */
	  glBindTexture( GL_TEXTURE_2D, texture[wall[OB_Z][z][y][x]-1] );
	  glBegin(GL_QUADS);

	  if(z<player[me].map.pos[OB_Z]) glNormal3f( 0, 0,  1.0);
	  else                           glNormal3f( 0, 0, -1.0);

	  glTexCoord2f(0.0f, 0.0f);  glVertex3f(   x,   y,   z);
	  glTexCoord2f(1.0f, 0.0f);  glVertex3f( x+1,   y,   z);
	  glTexCoord2f(1.0f, 1.0f);  glVertex3f( x+1, y+1,   z);
	  glTexCoord2f(0.0f, 1.0f);  glVertex3f(   x, y+1,   z);

	  glEnd( );
	}
      }
    }
  }

  /* process all Y tiles */
  for(y=0;y<32;y++) {
    for(x=0;x<32;x++) {
      for(z=0;z<32;z++) {
	if(wall[OB_Y][y][z][x]!=0) {
	  
	  /* Bind the proper texture to the triangle */
	  glBindTexture( GL_TEXTURE_2D, texture[wall[OB_Y][y][z][x]-1] );
	  glBegin(GL_QUADS);

	  if(y<player[me].map.pos[OB_Y]) glNormal3f( 0,  1.0, 0);
	  else                           glNormal3f( 0, -1.0, 0);

	  glTexCoord2f(0.0f, 0.0f);  glVertex3f( x+1,   y, z+1);
	  glTexCoord2f(1.0f, 0.0f);  glVertex3f(   x,   y, z+1);
	  glTexCoord2f(1.0f, 1.0f);  glVertex3f(   x,   y,   z);
	  glTexCoord2f(0.0f, 1.0f);  glVertex3f( x+1,   y,   z);
	  
	  glEnd( );
	}
      }
    }
  }

  /* process all X tiles */
  for(y=0;y<32;y++) {
    for(x=0;x<32;x++) {
      for(z=0;z<32;z++) {
	if(wall[OB_X][x][z][y]!=0) {
	  
	  /* Bind the proper texture to the triangle */
	  glBindTexture( GL_TEXTURE_2D, texture[wall[OB_X][x][z][y]-1] );
	  glBegin(GL_QUADS);
	  
	  if(x<player[me].map.pos[OB_X]) glNormal3f(  1.0, 0, 0);
	  else                           glNormal3f( -1.0, 0, 0);

	  glTexCoord2f(0.0f, 0.0f);  glVertex3f(   x, y+1,   z);
	  glTexCoord2f(0.0f, 1.0f);  glVertex3f(   x, y+1, z+1);
	  glTexCoord2f(1.0f, 1.0f);  glVertex3f(   x,   y, z+1);
	  glTexCoord2f(1.0f, 0.0f);  glVertex3f(   x,   y,   z);
	  
	  glEnd( );
	}
      }
    }
  }

  /* handle network */
  if(!NetHandle()) return FALSE;

  /* do some test stuff */
  DoPlayer();
  
  /* handle shots */
  DoShots(timer);

  /* draw 2d control stuff */
  DoControl();

  /* Draw it to the screen */
  SDL_GL_SwapBuffers( );

  return( TRUE );
}

GLfloat get_time(void) {
  static GLint timespec[FRAME_AVERAGE] = {0};
  GLint t, i, ret;

  /* not initialized */
  if(timespec[0]==0) 
    for(i=0;i<FRAME_AVERAGE;i++)
      timespec[i]=0;

  /* get current time spec */
  t = SDL_GetTicks();
  ret = t - timespec[FRAME_AVERAGE-1];

  /* shift timespec up */
  for(i=FRAME_AVERAGE-1;i>0;i--)
    timespec[i] = timespec[i-1];

  /* store current spec */
  timespec[0] = t;

  return (float)ret / FRAME_AVERAGE;
}

int main( int argc, char **argv )
{
  int videoFlags, i;
  int done = FALSE;
  SDL_Event event;
  const SDL_VideoInfo *videoInfo;
  GLfloat timer=0;
  float start_pos[4];

  LoadPrefs();

  /* load the maze */
  if ( load_maze(prefs.maze) )
    return FALSE;

  /* init network */
  if(argc <= 1) i = InitNet(NULL, start_pos);
  else          i = InitNet(argv[1], start_pos);

  if(!i) {
    fprintf(stderr, "Failed to initialize network, exiting.\n");
    Quit( 1 );
  }
  
  /* initialize SDL */
  if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE ) < 0 ) {
      fprintf( stderr, "Video initialization failed: %s\n",
	       SDL_GetError( ) );
      exit(1);
  }
  
  /* hide mouse if game is running */
  if(game_state == STATE_ACTIVE) mouse(FALSE);
  
  /* Fetch the video info */
  videoInfo = SDL_GetVideoInfo( );
  
  if ( !videoInfo ) {
    fprintf( stderr, "Video query failed: %s\n",
	     SDL_GetError( ) );
    exit(1);
  }
  
  /* the flags to pass to SDL_SetVideoMode */
  videoFlags  = SDL_OPENGL;          /* Enable OpenGL in SDL */
  videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering */
  videoFlags |= SDL_HWPALETTE;       /* Store the palette in hardware */
  
  /* This checks to see if surfaces can be stored in memory */
  if ( videoInfo->hw_available ) videoFlags |= SDL_HWSURFACE;
  else                           videoFlags |= SDL_SWSURFACE;
  
  /* This checks if hardware blits can be done */
  if ( videoInfo->blit_hw ) videoFlags |= SDL_HWACCEL;
  
  /* Sets up OpenGL double buffering */
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  
  /* get a SDL surface */
  screen_width  = screen_width_tab[prefs.mode];
  screen_height = screen_height_tab[prefs.mode];
  surface = SDL_SetVideoMode( screen_width, screen_height, SCREEN_BPP,
			      videoFlags );
  
  /* Verify there is a surface */
  if ( !surface ) {
    fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError( ) );
    Quit( 1 );
  }

  if(prefs.fullscreen)  SDL_WM_ToggleFullScreen( surface );

  /* initialize OpenGL */
  initGL( start_pos );
  
  /* resize the initial window */
  resizeWindow( screen_width, screen_height );
  
  /* wait for events */
  while ( !done ) {

    if(game_state == STATE_ACTIVE) {
      /* handle the events in the queue */
#ifdef SINGLE_TEST
      if(me==0)
#endif
	SDL_WarpMouse( screen_width/2, screen_height/2 );
    }
    
    while ( SDL_PollEvent( &event ) ) {
      switch( event.type ) {

      case SDL_VIDEORESIZE:
	/* handle resize event */
	surface = SDL_SetVideoMode( event.resize.w, event.resize.h,
				    SCREEN_BPP, videoFlags );
	if ( !surface ) {
	  fprintf( stderr, "Could not get a surface after resize: %s\n", 
		   SDL_GetError( ) );
	  Quit( 1 );
	}

	resizeWindow( event.resize.w, event.resize.h );
	break;

      case SDL_MOUSEBUTTONDOWN:
	/* A Mouse Button Was Pressed */
	if(game_state == STATE_ACTIVE) {
	  if(event.button.button == SDL_BUTTON_LEFT) {
	    FireShot();
	  }
	}
	break;

      case SDL_KEYDOWN:
	/* handle key presses */
	handleKeyPress( &event.key.keysym, TRUE, timer, videoFlags);
	break;

      case SDL_KEYUP:
	handleKeyPress( &event.key.keysym, FALSE, timer, videoFlags);
	break;

      case SDL_MOUSEMOTION:
	if((game_state == STATE_ACTIVE)&&(timer!=0)) {
	  rotate_object(&player[me].map, OB_X, 
			( screen_height/2 - event.motion.y ) * .2);
	  rotate_object(&player[me].map, OB_Y, 
			( screen_width/2 - event.motion.x ) * .2);
	}
	break;

      case SDL_QUIT:
	/* handle quit requests */
	done = TRUE;
	break;
      default:
	break;
      }
    }
    
    /* draw the scene */
    handleKeyPress(NULL, FALSE, timer, videoFlags);

    if(!drawGLScene( timer )) Quit( 1 );

    /* get timer for constant velocity stuff */
    timer = get_time();
  }
  
  /* clean ourselves up and exit */
  Quit( 0 );
  
  /* Should never get here */
  return( 0 );
}

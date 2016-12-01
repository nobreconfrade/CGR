#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdarg.h>
#include <string.h>

#include "SDL.h"
#include "maze.h"

GLuint glHits[5], glSlave, glMaster;
GLuint glFont, glCross, glPaused, glIcon[MAX_PLAYER];
GLuint glHealth, base;     /* Font Display List  */

void FreeControl(void) {
  /* Delete All 95 Font Display Lists */
  glDeleteLists( base, 95 );
}

/* Build Our Font Display List */
GLint BuildFont( GLvoid )
{
  int loop;
  SDL_Surface  *Image=NULL;

  puts("Loading font ...");

  /* Create The Texture */
  glGenTextures(1, &glFont );

  /* load font */
  if (!(Image=ExtSDL_LoadBMP("data/Font.bmp"))) return -1;

  /* Typical Texture Generation Using Data From The Bitmap */
  glBindTexture( GL_TEXTURE_2D, glFont);
  Image = torgb(Image);

  /* Generate The Texture */
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, Image->w,
		Image->h, 0, GL_RGB, GL_UNSIGNED_BYTE, Image->pixels );

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

  SDL_FreeSurface( Image );

  /* Creating 96 Display Lists */
  base = glGenLists( 96 );
  /* Bind Our Font Texture */
  glBindTexture( GL_TEXTURE_2D, glFont );
  
  /* Loop Through All 96 Lists */
  for ( loop = 0; loop < 96; loop++ ) {

      /* X Position Of Current Character */
      float cx = 1.0f - ( float )( loop % 16 ) / 16.0f;
      /* Y Position Of Current Character */
      float cy = 1.0f - 0.25 - ( float )( loop / 16 ) / 8.0f;

      /* Start Building A List */
      glNewList( base + ( 95 - loop ), GL_COMPILE );
      /* Use A Quad For Each Character */
      glBegin( GL_QUADS );
      /* Texture / Vertex Coord (Bottom Left) */
      glTexCoord2f( cx - 0.0625f, cy );
      glVertex2i( 0, 0 );
      /* Texutre / Vertex Coord (Bottom Right) */
      glTexCoord2f( cx, cy );
      glVertex2i( 15, 0 );
      /* Texture / Vertex Coord (Top Right) */
      glTexCoord2f( cx, cy - 0.120f );
      glVertex2i( 15, 15 );
      /* Texture / Vertex Coord (Top Left) */
      glTexCoord2f( cx - 0.0625f, cy - 0.120f );
      glVertex2i( 0, 15 );
      glEnd( );
      /* Move To The Right Of The Character */
      glTranslated( 10, 0, 0 );
      glEndList( );
    }

  return 0;
}

/* Where The Printing Happens */
GLvoid glPrint( GLint x, GLint y, const char *string, ... ) {
  char text[256]; /* Holds Our String             */
  va_list ap;     /* Pointer To List Of Arguments */
  
  /* If There's No Text */
  if ( string == NULL ) return;

  /* Parses The String For Variable */
  va_start( ap, string );
  /* And Converts Symbols To Actual Numbers */
  vsprintf( text, string, ap );
  va_end( ap );
  
  /* Select Our Font Texture */
  glBindTexture( GL_TEXTURE_2D, glFont );

  /* Store The Modelview Matrix */
  glPushMatrix( );
  /* Reset The Modelview Matrix */
  glLoadIdentity( );
  /* Position The Text (0,0 - Bottom Left) */
  glTranslated( x, y, 0 );
  /* Choose The Font Set */
  glListBase( base - 32 );
  /* Draws The Display List Text */
  glCallLists( strlen( text ), GL_UNSIGNED_BYTE, text );

  glPopMatrix( );
}

const char *menu[]={
  "Esc/Space - Toggle Menu / Start game",
  "F1        - Toggle Fullscreen",
  "F2        - Toggle Video Mode (%dx%d)    ",
#ifdef SOUND
  "F3        - Toggle Sound (%s)            ",
  "F4        - Toggle Music (%s)            ",
#endif
  "F12       - Quit Game",
  ""
};

#define CELL_WIDTH 10
#define CELL_HEIGHT 18

void draw_menu(void) {
  int x,y,i,j;
  char str[60];

  for(j=0,i=0;menu[i][0]!=0;i++)
    if(strlen(menu[i])>j) j=strlen(menu[i]);

  /* master or slave screen */
  if(game_state != STATE_PAUSED) i+=3; 

  x = (screen_width  - CELL_WIDTH*j)/2;
  y = (screen_height - CELL_HEIGHT*i)/2;

  /* dim background */
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor4f( 0, 0, 0, .8);
  glLoadIdentity( );
  glDisable( GL_TEXTURE_2D );
  glBegin(GL_QUADS);
  glVertex2d(   x-CELL_WIDTH,  screen_height-y+CELL_HEIGHT/2);
  glVertex2d(   screen_width-x+CELL_WIDTH, screen_height-y+CELL_HEIGHT/2);
  glVertex2d(   screen_width-x+CELL_WIDTH, screen_height-y-i*CELL_HEIGHT-CELL_HEIGHT/2);
  glVertex2d(   x-CELL_WIDTH, screen_height-y-i*CELL_HEIGHT-CELL_HEIGHT/2);
  glEnd(); 
  glEnable( GL_TEXTURE_2D );
  glColor4f( 1, 1, 1, 1);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  /* master or slave screen */
  if(game_state != STATE_PAUSED) {
    glColor4f( player_rgb[me][0], player_rgb[me][1], player_rgb[me][2], 1);

    if(me == MASTER)  strcpy(str, "Please start the game!");
    else              strcpy(str, "Waiting for master to start the game!");

    glPrint((screen_width-strlen(str)*CELL_WIDTH)/2,
	    screen_height-1-y-CELL_HEIGHT, str);

    /* there has been a winner */
    if(winner != -1) {
      glColor4f( player_rgb[winner][0], player_rgb[winner][1], player_rgb[winner][2], 1);
      sprintf(str, "The Winner is: %s", user_name[winner]);
      glPrint((screen_width-strlen(str)*CELL_WIDTH)/2,
	      screen_height-1-y-2*CELL_HEIGHT, str);
    }

    y+=3*CELL_HEIGHT;
    i-=3;
  }

  /* and draw the entries */
  glColor4f( 1, 1, 1, 1);
  for(j=0;j<i;j++) {
    if(j==2)       glPrint(x,screen_height-1-y-CELL_HEIGHT,menu[j],
			   screen_width_tab[prefs.mode],
			   screen_height_tab[prefs.mode]);
#ifdef SOUND
    else if(j==3)  glPrint(x,screen_height-1-y-CELL_HEIGHT,menu[j],
			   prefs.sound?"On":"Off");
    else if(j==4)  glPrint(x,screen_height-1-y-CELL_HEIGHT,menu[j],
			   prefs.music?"On":"Off");
#endif
    else           glPrint(x,screen_height-1-y-CELL_HEIGHT,menu[j]);
    y += CELL_HEIGHT;
  }
}

void draw_2d_status(void) {
  int i, x=16, y = screen_height-16;
  float health;

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glPushMatrix( );
  glLoadIdentity( );
  glTranslated( 0, 0, 0 );

  for(i=0;i<MAX_PLAYER;i++) {

    if(player[i].mapped || (i==me)) {
      /* calculate normalized (0..1) health */
      health = ((float)player[i].health)/MAX_HEALTH;

      /* draw health bar */
      glBindTexture(GL_TEXTURE_2D, glHealth);
      glBegin(GL_QUADS);
      glTexCoord2f(0.0f,1.0f);         glVertex2i( x,   y-32);
      glTexCoord2f(1.0f,1.0f);         glVertex2i( x+8, y-32);
      glTexCoord2f(1.0f,1.0f-health);  glVertex2i( x+8, y-(32*(1-health)));
      glTexCoord2f(0.0f,1.0f-health);  glVertex2i( x,   y-(32*(1-health)));
      glEnd();	      
    }

    if(player[i].mapped||(i==me)) {
      /* draw player logo */
      glBindTexture(GL_TEXTURE_2D, glIcon[i]);
      glBegin(GL_QUADS);
      glTexCoord2f(0.0f,1.0f);	glVertex2i(   x+12, y-31);
      glTexCoord2f(1.0f,1.0f);	glVertex2i(x+31+12, y-31);
      glTexCoord2f(1.0f,0.0f);	glVertex2i(x+31+12,    y);
      glTexCoord2f(0.0f,0.0f);	glVertex2i(   x+12,    y);
      glEnd();	
    }

    glBlendFunc(GL_SRC_ALPHA,GL_ONE);   // Set Blending For Translucency

    if(player[i].kills>0) {

      if(player[i].mapped || (i==me)) {
	glBindTexture(GL_TEXTURE_2D, 
		      glHits[((player[i].kills>5)?5:player[i].kills)-1]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f,1.0f);	glVertex2i( x+48,  y-15);
	glTexCoord2f(1.0f,1.0f);	glVertex2i( x+64,  y-15);
	glTexCoord2f(1.0f,0.0f);	glVertex2i( x+64,  y);
	glTexCoord2f(0.0f,0.0f);	glVertex2i( x+48,  y);
	glEnd();	

	if(player[i].kills>5) {
	  glBindTexture(GL_TEXTURE_2D, glHits[player[i].kills-6]);
	  glBegin(GL_QUADS);
	  glTexCoord2f(0.0f,1.0f);	glVertex2i( x+68,  y-15);
	  glTexCoord2f(1.0f,1.0f);	glVertex2i( x+84,  y-15);
	  glTexCoord2f(1.0f,0.0f);	glVertex2i( x+84,  y);
	  glTexCoord2f(0.0f,0.0f);	glVertex2i( x+68,  y);
	  glEnd();	
	}
      }
    }

    if(player[i].mapped || (i==me)) {

      glBlendFunc(GL_SRC_ALPHA,GL_ONE);   // Set Blending For Translucency
      /* print score */
      glPrint(x+44, y-34, "%d", player[i].score);
      
      /* print name */
      glColor4f( player_rgb[i][0], player_rgb[i][1], player_rgb[i][2], 1.0f);
      glPrint(x-4, y-50, user_name[i]);
      
      /* reset translucency and color */
      glColor4f( 1.0f, 1.0f, 1.0f, 1.0f);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    x+=100;
  }

  glPopMatrix( );
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void DoControl(void) {
  /* These are to calculate our fps */
  static GLint Frames  = 0;
  static GLfloat fps   = 0;
  static GLint T0      = 0;
  GLint t,y;

  glDisable( GL_DEPTH_TEST );
  glBlendFunc(GL_SRC_ALPHA,GL_ONE);   // Set Blending For Translucency
  glEnable(GL_BLEND);
  glDisable(GL_LIGHTING);
  glLoadIdentity();	

  /* draw crosshair/paused */
  if(game_state == STATE_ACTIVE) {
    glBindTexture(GL_TEXTURE_2D, glCross);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f,1.0f);	glVertex3f(-0.1, -0.1, -3.0f);
    glTexCoord2f(1.0f,1.0f);	glVertex3f( 0.1, -0.1, -3.0f);
    glTexCoord2f(1.0f,0.0f);	glVertex3f( 0.1,  0.1, -3.0f);
    glTexCoord2f(0.0f,0.0f);	glVertex3f(-0.1,  0.1, -3.0f);
    glEnd();	
  } 

  glMatrixMode( GL_PROJECTION );
  glPushMatrix( );
  glLoadIdentity( );
  glOrtho( 0, screen_width-1, 0, screen_height-1, -1, 1 );
  glMatrixMode( GL_MODELVIEW );

  /* draw pause/master/slave text */
  if(game_state != STATE_ACTIVE) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor3f( player_rgb[me][0], player_rgb[me][1], player_rgb[me][2]);

    if(game_state == STATE_PAUSED) {
      glBindTexture(GL_TEXTURE_2D, glPaused);
      y = screen_height*2/3;
    } else {
      if(me == 0)  glBindTexture(GL_TEXTURE_2D, glMaster);
      else         glBindTexture(GL_TEXTURE_2D, glSlave);
      y = screen_height*2/3 + 40;
    }

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f,1.0f);	glVertex2i((screen_width-256)/2, y-32);
    glTexCoord2f(1.0f,1.0f);	glVertex2i((screen_width+256)/2, y-32);
    glTexCoord2f(1.0f,0.0f);	glVertex2i((screen_width+256)/2, y+32);
    glTexCoord2f(0.0f,0.0f);	glVertex2i((screen_width-256)/2, y+32);
    glEnd();	

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  }

  glColor4f( 1.0f, 1.0f, 1.0f, 1.0f);
 
  /* Gather our frames per second */
  Frames++;
  t = SDL_GetTicks();
  if (t - T0 >= 1000) {
    fps = Frames / ((t - T0) / 1000.0);
    T0 = t; Frames = 0;
  }
  
  if(fps != 0) glPrint( 10, 10, "%.1f FPS", fps);

  draw_2d_status();

  DrawRadar();

  if(game_state != STATE_ACTIVE) draw_menu();
  
  glColor4f( 1.0f, 1.0f, 1.0f, 0.5f);

  /* Select The Projection Matrix */ 
  glMatrixMode( GL_PROJECTION );
  /* Restore The Old Projection Matrix */
  glPopMatrix( );
  /* Select The Modelview Matrix */
  glMatrixMode( GL_MODELVIEW );

  glEnable( GL_DEPTH_TEST );
}

int InitControl(){
  int i;
  char ext[]="ybgcmr", str[64];

  puts("Init controls ...");
  LoadTexture(&glCross, "data/crosshair.bmp", FALSE);
  LoadTexture(&glPaused, "data/paused.bmp", 0x80000000);
  LoadTexture(&glHealth, "data/bar.bmp", FALSE);
  LoadTexture(&glMaster, "data/master.bmp", 0x80000000);
  LoadTexture(&glSlave, "data/slave.bmp", 0x80000000);

  for(i=0;i<5;i++) {
    sprintf(str, "data/hit%d.bmp",i+1);
    LoadTexture(&glHits[i], str, FALSE);
  }

  printf("Init icons ");
  for(i=0;i<MAX_PLAYER;i++) {
    sprintf(str, "data/smiley_%c_icon.bmp", ext[i]);
    LoadTexture(&glIcon[i], str, 0x334c66);
    printf(".");
    fflush(stdout);
  }
  printf("\n");
  
  BuildFont();

  return 0;
}


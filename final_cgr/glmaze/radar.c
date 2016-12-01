/*
  radar.c - part of glmaze

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

GLint glRadar;

void InitRadar(void) {
  puts("Init radar ...");
  LoadTexture(&glRadar, "data/radar.bmp",  0x80000000);
}

#define RAD 51.0
#define Z_SCALE 5.0

void DrawRadar(void) {
  GLUquadricObj *q;
  int i;
  float x,y,z;
  int xm,ym,zm,yzm;
  static float scale = -1;

  /* recalc scale */
  if(scale == -1) {
    scale = RAD/sqrt(SQ(extend[OB_X])+SQ(extend[OB_Y])+SQ(extend[OB_Z]));
  }

  glDisable( GL_TEXTURE_2D );
  glLineWidth(1.0f);

  glTranslatef( screen_width/2, 64, 0 );

  q = gluNewQuadric( );
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  /* dim background */
  glColor4f( 0, 0, 0, .8);
  gluDisk (q, RAD, 0, 32, TRUE);

  /* light disc */
  glColor4f( 1, 1, 1, .1);
  glScalef( 1, 1/Z_SCALE, 1);          // change aspect ratio
  gluDisk (q, RAD, 0, 32, TRUE);
  glScalef( 1,  Z_SCALE, 1);          // restore ratio

  glColor4f( 1, 1, 1, 1);

  /* draw all radar echos */
  for(i=0;i<MAX_PLAYER;i++) {
    if((i!=me) && player[i].mapped) {
      x = player[i].map.pos[OB_X] - player[me].map.pos[OB_X];
      y = player[i].map.pos[OB_Y] - player[me].map.pos[OB_Y];
      z = player[i].map.pos[OB_Z] - player[me].map.pos[OB_Z];

      xm = scale * ( x * player[me].map.x_vec[OB_X] + 
	   y * player[me].map.y_vec[OB_X] + z * player[me].map.z_vec[OB_X]);

      ym = scale * ( x * player[me].map.x_vec[OB_Y] +
	   y * player[me].map.y_vec[OB_Y] + z * player[me].map.z_vec[OB_Y]);

      zm = -scale / Z_SCALE * ( x * player[me].map.x_vec[OB_Z] +
	   y * player[me].map.y_vec[OB_Z] + z * player[me].map.z_vec[OB_Z]);

      glColor4f(player_rgb[i][0],player_rgb[i][1],player_rgb[i][2],1.0);

      yzm = ym+zm;

      glBegin(GL_LINES);
      glVertex2i( xm, zm);
      glVertex2i( xm, yzm);
      glEnd();

      glBegin(GL_POLYGON);
      glVertex2i( xm,  yzm-3);
      glVertex2i( xm-3, yzm);
      glVertex2i( xm,  yzm+3);
      glVertex2i( xm+3, yzm);
      glEnd();

    }
  }

  /* draw decorations */
  glEnable( GL_TEXTURE_2D );
  glLoadIdentity( );
  glTranslated( 0, 0, 0 );

  /* use full color range */
  glColor3f( (player_rgb[me][0] == 1)?player_rgb[me][0]:0, 
	     (player_rgb[me][1] == 1)?player_rgb[me][1]:0, 
	     (player_rgb[me][2] == 1)?player_rgb[me][2]:0);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBindTexture(GL_TEXTURE_2D, glRadar);
  glBegin(GL_QUADS);
  glTexCoord2i(0.0f,1.0f);	glVertex2i((screen_width+256)/2,  0);
  glTexCoord2i(1.0f,1.0f);	glVertex2i((screen_width-256)/2,  0);
  glTexCoord2i(1.0f,0.0f);	glVertex2i((screen_width-256)/2, 128);
  glTexCoord2i(0.0f,0.0f);	glVertex2i((screen_width+256)/2, 128);
  glEnd();		

  glColor4f( 1, 1, 1, 1);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

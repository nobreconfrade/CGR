#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdarg.h>
#include <string.h>

#include "SDL.h"
#include "maze.h"

GLint glPlayer[MAX_PLAYER], glShield;
int global_hit_map;

void InitPlayer(void) {
  int i;
  char ext[]="ybgcmr", str[64];

  printf("Init players ");

  for(i=0;i<MAX_PLAYER;i++) {
    sprintf(str, "data/smiley_%c.bmp", ext[i]);
    LoadTexture(&glPlayer[i], str, FALSE);
    printf(".");
    fflush(stdout);
  }
  LoadTexture(&glShield, "data/shield.bmp", FALSE);
  printf("\n");
}

void FreePlayer(void) {
}

void DoPlayer(void) {
  GLUquadricObj *q;
  int i;
  GLfloat matrix[16];

  for(i=0;i<MAX_PLAYER;i++) {
    /* only handle mapped other players or myself if i am hit */
    if((player[i].mapped && (i!=me))||(player[i].hit && (i==me))) {
      
      glPushMatrix( );
      
      /* Create A New Quadrati */
      q = gluNewQuadric( );
      /* Generate Smooth Normals For The Quad */
      gluQuadricNormals( q, GL_SMOOTH );
      /* Enable Texture Coords For The Quad */
      gluQuadricTexture( q, GL_TRUE );
      
      /* Set Up Sphere Mapping */
      glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
      /* Set Up Sphere Mapping */
      glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP );
      
      glTranslatef( player[i].map.pos[OB_X],
		    player[i].map.pos[OB_Y],
		    player[i].map.pos[OB_Z]);
      
      /* use player normal vectors as rotation matrix */
      matrix[ 0] = player[i].map.x_vec[OB_X]; 
      matrix[ 1] = player[i].map.y_vec[OB_X]; 
      matrix[ 2] = player[i].map.z_vec[OB_X]; 
      matrix[ 3] = 0;

      matrix[ 4] = player[i].map.x_vec[OB_Y]; 
      matrix[ 5] = player[i].map.y_vec[OB_Y]; 
      matrix[ 6] = player[i].map.z_vec[OB_Y]; 
      matrix[ 7] = 0;

      matrix[ 8] = player[i].map.x_vec[OB_Z]; 
      matrix[ 9] = player[i].map.y_vec[OB_Z]; 
      matrix[10] = player[i].map.z_vec[OB_Z]; 
      matrix[11] = 0;
      
      matrix[12] = 0;                      
      matrix[13] = 0;              
      matrix[14] = 0;                      
      matrix[15] = 1;

      glMultMatrixf(matrix);

      glDisable(GL_BLEND);                 // Enable Blending 
      
      /* don't draw own sphere */
      if(i != me) {
	/* Draw First Sphere */
	glBindTexture( GL_TEXTURE_2D, glPlayer[i] );
	gluSphere( q, PLAYER_RADIUS, 32, 16 );
      }

      if(player[i].hit) {
	glDepthMask(GL_FALSE);              // disable writing to depth buf

	glBlendFunc(GL_SRC_ALPHA,GL_ONE);   // Set Translucency
	glEnable(GL_BLEND);                 // Enable Blending 
	
	glBindTexture( GL_TEXTURE_2D, glShield );
	gluSphere( q, PLAYER_RADIUS + 0.101 - 0.001 * player[i].hit, 32, 16 );

	glDepthMask(GL_TRUE);               // enable writing to depth buf
      }
      
      glPopMatrix( );
    }
  }
}


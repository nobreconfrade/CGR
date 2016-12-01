/*
  shot.c - part of glmaze

  (c) 2001 by Till Harbaum
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdarg.h>
#include <string.h>

#include "SDL.h"
#include "maze.h"

GLint glShot;

SHOT shot[MAX_SHOTS];                      /* our own shots       */
O_SHOT o_shot[(MAX_PLAYER-1)*MAX_SHOTS];   /* other peoples shots */

int local_hit_map;                         /* save info about own hits */

const  float player_rgb[MAX_PLAYER][3]={
  { 1.0, 1.0, 0.3 }, { 0.3, 0.3, 1.0 }, { 0.3, 1.0, 0.3 },
  { 0.3, 1.0, 1.0 }, { 1.0, 0.3, 1.0 }, { 1.0, 0.3, 0.3 } };

SDL_Surface  *torgb(SDL_Surface  *s) {
  SDL_Surface  *cs, *tmp;

  /* i need an SDL_PixelFormat, for RGB 24 bit format, struct */
  tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, 0, 0, 24, 
			     0x000000ff, 0x0000ff00, 0x00ff0000, 
			     0x00000000);

  /* convert given surface to rgb format */
  cs = SDL_ConvertSurface(s, tmp -> format, SDL_SWSURFACE); 
  SDL_FreeSurface(s);
  SDL_FreeSurface(tmp);
  return cs;
}

/* paths to search for glMaze files */
char *gm_path[]={
  "/usr/games/glMaze/",
  "/usr/games/glmaze/",
  "/usr/local/games/glMaze/",
  "/usr/local/games/glmaze/",
  "./", 
  ""
};

FILE *Extfopen(const char *name, const char *mode) {
  FILE *ret;  
  int i;
  char str[128];

  /* scan all paths */
  for(i=0;gm_path[i][0]!=0;i++) {
    strcpy(str, gm_path[i]);
    strcat(str, name);
    
    /* first look at current directory */
    if ((ret = fopen( str, mode ))) {
      return ret;
    }
  }
  
  return NULL;
}

SDL_Surface *ExtSDL_LoadBMP( char *name ) {
  SDL_Surface *ret;
  int i;
  char str[128];

  /* scan all paths */
  for(i=0;gm_path[i][0]!=0;i++) {
    strcpy(str, gm_path[i]);
    strcat(str, name);
    
    /* first look at current directory */
    if ((ret = SDL_LoadBMP( str ))) {
      return ret;
    }
  }
  
  return NULL;
}

int LoadTexture(GLint *tex, char *name, int mode) {
  SDL_Surface *TextureImage; 
  char *rgba, *rgb;
  int i;

  if (!(TextureImage = ExtSDL_LoadBMP( name ) )) {
    printf("loading of texture %s failed ...\n", name);
    return 1;
  }
  
  /* Create The Texture */
  glGenTextures(1, tex );
  
  /* Typical Texture Generation Using Data From The Bitmap */
  glBindTexture( GL_TEXTURE_2D, *tex );
  
  TextureImage = torgb(TextureImage);

  /* generate alpha channel */
  if(mode) {
    rgba = malloc(4*TextureImage->w*TextureImage->h);
    rgb = (char*)TextureImage->pixels;

    for(i=0;i<TextureImage->w*TextureImage->h;i++) {
      rgba[4*i+0] = rgb[3*i+0];
      rgba[4*i+1] = rgb[3*i+1];
      rgba[4*i+2] = rgb[3*i+2];

      if((rgb[3*i+2] == (mode&0xff)>>0) &&
	 (rgb[3*i+1] == (mode&0xff00)>>8) &&
	 (rgb[3*i+0] == (mode&0xff0000)>>16))
	rgba[4*i+3] = 0;
      else
	rgba[4*i+3] = 255;
    }

    /* Generate The Texture */
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, TextureImage->w,
		  TextureImage->h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		  rgba );
    
#ifdef FILTER    
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, TextureImage->w, TextureImage->h, GL_RGBA, 
		      GL_UNSIGNED_BYTE, rgba ); 
#else
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
#endif

    free(rgba);
  } else {  
    /* Generate The Texture */
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, TextureImage->w,
		  TextureImage->h, 0, GL_RGB, GL_UNSIGNED_BYTE,
		  TextureImage->pixels );
    
#ifdef FILTER    
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, TextureImage->w, TextureImage->h, GL_RGB, 
		      GL_UNSIGNED_BYTE, TextureImage->pixels ); 
#else
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
#endif
  }

  /* Linear Filtering */
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  
  SDL_FreeSurface( TextureImage );

  return 0;
}

void InitShots(void) {
  int i,j;

  puts("Init shots ...");
  LoadTexture(&glShot, "data/Star.bmp", FALSE);

  /* nothing hit yet */
  local_hit_map = 0;

  /* erase own shots */
  for(i=0;i<MAX_SHOTS;i++) 
    shot[i].mapped = FALSE;

  /* erase network player shots */
  for(j=0;j<(MAX_PLAYER-1)*MAX_SHOTS;j++)
    o_shot[j].mapped = FALSE;
}

void FreeShots(void) {
}

void FireShot(void) {
  int i;

  for(i=0;i<MAX_SHOTS;i++) {
    if(!shot[i].mapped){
      SoundPlay(SOUND_SHOT, 1+i);

      /* enable one shot */
      shot[i].mapped = TRUE;
      shot[i].map.pos[OB_X] = player[me].map.pos[OB_X] - 
	PLAYER_RADIUS * player[me].map.x_vec[OB_Z];
      shot[i].map.pos[OB_Y] = player[me].map.pos[OB_Y] - 
	PLAYER_RADIUS * player[me].map.y_vec[OB_Z];
      shot[i].map.pos[OB_Z] = player[me].map.pos[OB_Z] - 
	PLAYER_RADIUS * player[me].map.z_vec[OB_Z];

      shot[i].map.x_vec[OB_Z] = -player[me].map.x_vec[OB_Z];
      shot[i].map.y_vec[OB_Z] = -player[me].map.y_vec[OB_Z];
      shot[i].map.z_vec[OB_Z] = -player[me].map.z_vec[OB_Z];
      shot[i].dist = 10.0; 

      break;
    }
  }
}

int shot_compare(const void *a, const void *b) {
  O_SHOT *shot_a = (O_SHOT*)a;
  O_SHOT *shot_b = (O_SHOT*)b;

  return( ( SQ(shot_a->pos[OB_X] - player[0].map.pos[OB_X]) +
	    SQ(shot_a->pos[OB_Y] - player[0].map.pos[OB_Y]) +
	    SQ(shot_a->pos[OB_Z] - player[0].map.pos[OB_Z])) <
	  ( SQ(shot_b->pos[OB_X] - player[0].map.pos[OB_X]) +
	    SQ(shot_b->pos[OB_Y] - player[0].map.pos[OB_Y]) +
	    SQ(shot_b->pos[OB_Z] - player[0].map.pos[OB_Z])) );	    
}

void DoShots(GLint timer) {
  float scale = SHOT_RADIUS;
  float step = 0.005 * timer, k;
  int i,j;
  O_SHOT xshot[MAX_PLAYER*MAX_SHOTS];

  /* get other players shots */
  for(j=0,i=0;i<(MAX_PLAYER-1)*MAX_SHOTS;i++) {
    if(o_shot[i].mapped) {
      xshot[j].mapped    = TRUE;
      xshot[j].color     = o_shot[i].color;
      xshot[j].pos[OB_X] = o_shot[i].pos[OB_X];
      xshot[j].pos[OB_Y] = o_shot[i].pos[OB_Y];
      xshot[j].pos[OB_Z] = o_shot[i].pos[OB_Z];
      j++;
    }
  }

  /* add own shots */
  for(i=0;i<MAX_SHOTS;i++) {
    if(shot[i].mapped) {
      xshot[j].color = me;
      xshot[j].mapped    = TRUE;
      xshot[j].pos[OB_X] = shot[i].map.pos[OB_X];
      xshot[j].pos[OB_Y] = shot[i].map.pos[OB_Y];
      xshot[j].pos[OB_Z] = shot[i].map.pos[OB_Z];
      j++;
    }
  }
  
  /* sort transparent objects regarding depth */
  qsort(xshot, j, sizeof(O_SHOT), shot_compare);

  glBlendFunc(GL_SRC_ALPHA,GL_ONE);   // Set Translucency
  glEnable(GL_BLEND);                 // Enable Blending 
  glDisable(GL_LIGHTING);

  for(i=0;i<j;i++) {
    if(xshot[i].mapped) {
      glColor4f( player_rgb[xshot[i].color][0], 
		 player_rgb[xshot[i].color][1], 
		 player_rgb[xshot[i].color][2], 1.0f);

      glBindTexture( GL_TEXTURE_2D,  glShot);
      glBegin(GL_QUADS);

      /* normal faces player for max brightness */
      glNormal3f( player[me].map.x_vec[OB_Z], 
		  player[me].map.y_vec[OB_Z], 
		  player[me].map.z_vec[OB_Z]);
      
      glTexCoord2f(0.0f, 0.0f);  
      glVertex3f( xshot[i].pos[OB_X] - 
		  player[me].map.x_vec[OB_X] * scale - 
		  player[me].map.x_vec[OB_Y] * scale, 
		  xshot[i].pos[OB_Y] - 
		  player[me].map.y_vec[OB_X] * scale - 
		  player[me].map.y_vec[OB_Y] * scale, 
		  xshot[i].pos[OB_Z] - 
		  player[me].map.z_vec[OB_X] * scale - 
		  player[me].map.z_vec[OB_Y] * scale);
	
      glTexCoord2f(0.0f, 1.0f);  
      glVertex3f( xshot[i].pos[OB_X] - 
		  player[me].map.x_vec[OB_X] * scale + 
		  player[me].map.x_vec[OB_Y] * scale, 
		  xshot[i].pos[OB_Y] - 
		  player[me].map.y_vec[OB_X] * scale + 
		  player[me].map.y_vec[OB_Y] * scale, 
		  xshot[i].pos[OB_Z] - 
		  player[me].map.z_vec[OB_X] * scale + 
		  player[me].map.z_vec[OB_Y] * scale);
	
      glTexCoord2f(1.0f, 1.0f);  
      glVertex3f( xshot[i].pos[OB_X] + 
		  player[me].map.x_vec[OB_X] * scale + 
		  player[me].map.x_vec[OB_Y] * scale, 
		  xshot[i].pos[OB_Y] + 
		  player[me].map.y_vec[OB_X] * scale + 
		  player[me].map.y_vec[OB_Y] * scale, 
		  xshot[i].pos[OB_Z] + 
		  player[me].map.z_vec[OB_X] * scale + 
		  player[me].map.z_vec[OB_Y] * scale);
	
      glTexCoord2f(1.0f, 0.0f);  
      glVertex3f( xshot[i].pos[OB_X] + 
		  player[me].map.x_vec[OB_X] * scale - 
		  player[me].map.x_vec[OB_Y] * scale, 
		  xshot[i].pos[OB_Y] + 
		  player[me].map.y_vec[OB_X] * scale - 
		  player[me].map.y_vec[OB_Y] * scale, 
		  xshot[i].pos[OB_Z] + 
		  player[me].map.z_vec[OB_X] * scale - 
		  player[me].map.z_vec[OB_Y] * scale);
      
      glEnd( );
    }
  }

  /* process own shots */
  for(i=0;i<MAX_SHOTS;i++) {
    k = step;

    if(shot[i].mapped) {
      while(k > STEP_RES) {  

        shot[i].map.pos[OB_X] += STEP_RES * shot[i].map.x_vec[OB_Z];
        shot[i].map.pos[OB_Y] += STEP_RES * shot[i].map.y_vec[OB_Z];
        shot[i].map.pos[OB_Z] += STEP_RES * shot[i].map.z_vec[OB_Z];
        shot[i].dist -= STEP_RES;
        k -= STEP_RES;

	/* check if shot hit something or */
	/* shot expires without hitting anything */
	if(!check_step(shot[i].map, SHOT_COLL_RADIUS)) {
	  SoundPlay(SOUND_WALL, i+i);
	  shot[i].mapped = FALSE;
	}

        if(shot[i].dist<0.0)
	    shot[i].mapped = FALSE;

	CheckShot();
      }

      if(k > 0.0) {  

        shot[i].map.pos[OB_X] += k * shot[i].map.x_vec[OB_Z];
        shot[i].map.pos[OB_Y] += k * shot[i].map.y_vec[OB_Z];
        shot[i].map.pos[OB_Z] += k * shot[i].map.z_vec[OB_Z];
        shot[i].dist -= k;
	
	/* check if shot hit something or */
	/* shot expires without hitting anything */
	if(!check_step(shot[i].map, SHOT_COLL_RADIUS)) {
	  SoundPlay(SOUND_WALL, i+1);
	  shot[i].mapped = FALSE;
	}

        if(shot[i].dist<0.0)
	    shot[i].mapped = FALSE;

	CheckShot();
      }
    }
  }

  /* reset color */
  glColor4f( 1.0f, 1.0f, 1.0f, 1.0f);
}

void CheckShot(void) {
  int scnt, pcnt;

  for(scnt=0;scnt<MAX_SHOTS;scnt++) {
    if(shot[scnt].mapped) {	

      for(pcnt=0;pcnt<MAX_PLAYER;pcnt++) {

	if(player[pcnt].mapped) {	
	  if( SQ(shot[scnt].map.pos[OB_X]-player[pcnt].map.pos[OB_X])+
	      SQ(shot[scnt].map.pos[OB_Y]-player[pcnt].map.pos[OB_Y])+
	      SQ(shot[scnt].map.pos[OB_Z]-player[pcnt].map.pos[OB_Z])
	      < SQ(SHOT_RADIUS+PLAYER_RADIUS)) {
	    
            SoundPlay(SOUND_HIT, scnt+1);

	    shot[scnt].mapped = FALSE;   /* remove shot */
	    local_hit_map |= (1<<pcnt);  /* player is hit */
	  }
	}
      }
    }
  }
}

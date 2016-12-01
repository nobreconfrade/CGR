#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <unistd.h>

#include "SDL.h"
#include "maze.h"

/* pointer to wall info */
int extend[3]={0,0,-1};
int wall[3][MAX_EXT][MAX_EXT][MAX_EXT];
int num_textures=0;

SDL_Surface *TextureImage[MAX_TEX]; 

int convert_textures(void) {
  /* Status indicator */
  int loop;
    
  /* Create The Texture */
  glGenTextures(num_textures, &texture[0] );
  
  /* Load in textures */
  for ( loop = 0; loop < num_textures; loop++ ) {

    /* Typical Texture Generation Using Data From The Bitmap */
    glBindTexture( GL_TEXTURE_2D, texture[loop] );
    
    TextureImage[loop] = torgb(TextureImage[loop]);

    /* Generate The Texture */
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, TextureImage[loop]->w,
		  TextureImage[loop]->h, 0, GL_RGB,
		  GL_UNSIGNED_BYTE,
		  TextureImage[loop]->pixels );

#ifdef FILTER    
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, TextureImage[loop]->w, TextureImage[loop]->h, GL_RGB, 
		      GL_UNSIGNED_BYTE, TextureImage[loop]->pixels ); 
#else
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
#endif
    /* Linear Filtering */
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  }

  /* Free up any memory we may have used */
  for ( loop = 0; loop < num_textures; loop++ )
    if ( TextureImage[loop] )
      SDL_FreeSurface( TextureImage[loop] );
  
  return 0;
}


/* function to load in bitmap as a GL texture */
int load_texture(char *name, int tex) {

  /* Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit */
  if (!(TextureImage[tex] = ExtSDL_LoadBMP( name ) )) {
    printf("loading of texture \"%s\" failed ...\n", name);
    return 1;
  }

  return 0;
}

int load_maze(char *filename) {
  FILE *fp;
  char msg[128], *p;
  int i,known_textures[256],line=0, linelen=-1;
  int j,x,y,z;
  int box[256][6];
  int last_was_blank = 1;

  /* init variables */
  for(i=0;i<256;i++) known_textures[i]=-1;
  for(i=0;i<256;i++) box[i][0]=-1;

  for(i=OB_X;i<=OB_Z;i++)
    for(z=0;z<MAX_EXT;z++)
      for(y=0;y<MAX_EXT;y++)
	for(x=0;x<MAX_EXT;x++)
	  wall[i][z][y][x]=0;

  printf("Loading %s ...\n", filename);
  fp = Extfopen(filename, "r");
  if(fp == NULL) {
    printf("Unable to open file!\n");
    return -1;
  }

  /* read whole file */
  while(fgets(msg, 128, fp) != 0) {
    line++;
    p=msg;

    /* skip leading white spaces */
    while((*p==' ')||(*p=='\t')) p++;

    if((*p!='\0')&&(*p!='\n')&&(*p!=';')) {
      if(last_was_blank)
	extend[OB_Y]=0;

      if(strncmp(p,"box[",4)==0) {

	if(strncmp(p+7, "none", 4)==0)
	  box[(int)p[4]][0]=0;
	else {
	  for(i=0;i<6;i++) box[(int)p[4]][i]=p[8+i];
	}
      } else if(strncmp(p,"texture[",8)==0) {
	if(known_textures[(int)p[8]] != -1) 
	  printf("Warning, texture [%c] redefined in line %d\n", p[8], line);

	if(strncmp(&p[11], "none",4)==0) {
	  known_textures[(int)p[8]] = 0;
	} else {
	  /* mark texture as used */
	  known_textures[(int)p[8]] = num_textures+1;

	  while((*p!='\"')&&(*p!='\0')) p++;
	  while(p[strlen(p)-1]!='"') p[strlen(p)-1]=0;
	  p[strlen(p)-1]=0; p++;

	  load_texture(p, num_textures++);
	}
      } else {
	/* remove trailing white spaces */
	while(isblank(p[strlen(p)-1])||(p[strlen(p)-1]=='\n')) 
	  p[strlen(p)-1]=0;

	if(linelen == -1) linelen = strlen(p);
	extend[OB_X] = linelen;
	if(last_was_blank) extend[OB_Z]++;
	last_was_blank = 0;

	for(j=0;j<strlen(p);j++) {
	  if(box[(int)p[j]][0] != 0) {
	    // printf("wall at %d %d %d\n", j, extend[OB_Y], extend[OB_Z]);

	    /* only set walls, that or not already set   */
	    /* if it is already set, remove it, since it */
	    /* is now an invisible inner wall between    */
	    /* two boxes                                 */

	    /* left and right face */
	    if(!wall[OB_X][j+0][extend[OB_Z]][extend[OB_Y]])
	      wall[OB_X][j+0][extend[OB_Z]][extend[OB_Y]] = 
		known_textures[box[(int)p[j]][1]];
	    else
	      wall[OB_X][j+0][extend[OB_Z]][extend[OB_Y]] = 0;

	    if(!wall[OB_X][j+1][extend[OB_Z]][extend[OB_Y]])
	      wall[OB_X][j+1][extend[OB_Z]][extend[OB_Y]] = 
		known_textures[box[(int)p[j]][3]];
	    else
	      wall[OB_X][j+1][extend[OB_Z]][extend[OB_Y]] = 0;

	    /* front and rear face */
	    if(!wall[OB_Y][extend[OB_Y]+0][extend[OB_Z]][j])
	      wall[OB_Y][extend[OB_Y]+0][extend[OB_Z]][j] =
		known_textures[box[(int)p[j]][2]];
	    else
	      wall[OB_Y][extend[OB_Y]+0][extend[OB_Z]][j] = 0;

	    if(!wall[OB_Y][extend[OB_Y]+1][extend[OB_Z]][j])
	      wall[OB_Y][extend[OB_Y]+1][extend[OB_Z]][j] =
		known_textures[box[(int)p[j]][4]];
	    else
	      wall[OB_Y][extend[OB_Y]+1][extend[OB_Z]][j] = 0;

	    /* top and bottom face */
	    if(!wall[OB_Z][extend[OB_Z]+0][extend[OB_Y]][j])
	      wall[OB_Z][extend[OB_Z]+0][extend[OB_Y]][j] =
		known_textures[box[(int)p[j]][0]];
	    else
	      wall[OB_Z][extend[OB_Z]+0][extend[OB_Y]][j] = 0;

	    if(!wall[OB_Z][extend[OB_Z]+1][extend[OB_Y]][j])
	      wall[OB_Z][extend[OB_Z]+1][extend[OB_Y]][j] =
		known_textures[box[(int)p[j]][5]];
	    else
	      wall[OB_Z][extend[OB_Z]+1][extend[OB_Y]][j] = 0;
	  }
	}
	  
	extend[OB_Y]++;
      }
    } else {
      last_was_blank = 1;
    }
  }

  extend[OB_Z]+=1;
  // printf("size of maze: %dx%dx%d\n", extend[OB_X],extend[OB_Y],extend[OB_Z]);

  /* create borders */
  for(y=0;y<extend[OB_Y];y++) {
    for(z=0;z<extend[OB_Z];z++) {
      if(!wall[OB_X][0][z][y])            wall[OB_X][0][z][y]=1;   
      if(!wall[OB_X][extend[OB_X]][z][y]) wall[OB_X][extend[OB_X]][z][y]=1;
    }
  }

  for(x=0;x<extend[OB_X];x++) {
    for(z=0;z<extend[OB_Z];z++) {
      if(!wall[OB_Y][0][z][x])            wall[OB_Y][0][z][x]=1;   
      if(!wall[OB_Y][extend[OB_Y]][z][x]) wall[OB_Y][extend[OB_Y]][z][x]=1;
    }
  }

  for(x=0;x<extend[OB_X];x++) {
    for(y=0;y<extend[OB_Y];y++) {
      if(!wall[OB_Z][0][y][x])            wall[OB_Z][0][y][x]=1;   
      if(!wall[OB_Z][extend[OB_Z]][y][x]) wall[OB_Z][extend[OB_Z]][y][x]=1;
    }
  }

  fclose(fp);

  return 0;
}

#ifdef TEST
int main(int argc, char **argv) {
  if(argc != 2) {
    printf("usage: %s filename\n", argv[0]);
    exit(1);
  }

  load_maze(argv[1]);
}
#endif

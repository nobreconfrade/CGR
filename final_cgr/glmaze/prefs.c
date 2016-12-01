/*

  prefs.c - part of glmaze

  (c) 2001 by Till Harbaum

 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdarg.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "SDL.h"
#include "maze.h"

PREFS prefs;

#define PREF_FILE ".glmaze"

void set_defaults(void) {
  strcpy(prefs.maze, "levels/maze.maz");
  strcpy(prefs.name, get_player_name());
  prefs.mode = 0;     /* 640 x 480 */
  prefs.fullscreen = FALSE;
  prefs.sound = prefs.music = TRUE;
}

char *get_fname(void) {
  struct passwd *pw;
  static char fname[128];

  /* locate user's home directory */
  if(!(pw=getpwuid(getuid()))) { 
    printf("Who are you? I can't find you in the passwd database!\n");
    return NULL;
  }

  sprintf(fname, "%s/%s", pw->pw_dir, PREF_FILE);

  return fname;
}

void LoadPrefs(void) {
  FILE *fp;
  char *fname;
  char msg[128], id[32], val[128], *p;

  /* load default preferences */
  set_defaults();

  if((fname = get_fname()) == NULL) return;

  if((fp = fopen(fname, "r"))==NULL) {
    printf("No config file \"%s\" found, using defaults.\n", fname);
    return;
  }

  while(fgets(msg, 128, fp) != 0) {
    if(msg[0] != ';') {
      if(sscanf(msg, "%[^=]=%s", id, val)==2) {
	if(strcmp(id, "MAZE")==0) {
	  if((p = strchr(msg, '\"')) != NULL) {
	    p++; if(strrchr(msg, '\"') != NULL) *strrchr(msg, '\"') = 0;
	    strcpy(prefs.maze, p);
	  }
	} else if(strcmp(id, "NAME")==0) {
	  if((p = strchr(msg, '\"')) != NULL) {
	    p++; if(strrchr(msg, '\"') != NULL) *strrchr(msg, '\"') = 0;

	    if(strlen(p)>8) p[8]=0;
	    strcpy(prefs.name, p);
	  }
	} else if(strcmp(id, "MODE")==0) {
	  prefs.mode = atoi(val);
	} else if(strcmp(id, "FULLSCREEN")==0) {
	  prefs.fullscreen = atoi(val);
	} else if(strcmp(id, "MUSIC")==0) {
	  prefs.music = atoi(val);
	} else if(strcmp(id, "SOUND")==0) {
	  prefs.sound = atoi(val);
	}
      }
    }
  }
  
  fclose(fp);
}

void SavePrefs(void) {
  FILE *fp;
  char *fname;

  if((fname = get_fname()) == NULL) return;

  if((fp = fopen(fname, "w"))==NULL) {
    printf("Unable to write config file \"%s\".\n", fname);
    return;
  }

  /* write config */
  fprintf(fp, ";; .glmaze config file\n");
  fprintf(fp, "MAZE=\"%s\"\n", prefs.maze);
  fprintf(fp, "NAME=\"%s\"\n", prefs.name);
  fprintf(fp, "MODE=%d\n", prefs.mode);
  fprintf(fp, "FULLSCREEN=%d\n", prefs.fullscreen);
  fprintf(fp, "MUSIC=%d\n", prefs.music);
  fprintf(fp, "SOUND=%d\n", prefs.sound);
  
  fclose(fp);
}




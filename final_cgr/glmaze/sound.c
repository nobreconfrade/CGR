/*

  sound.c for glmaze

  (c) 2001 by Till Harbaum

 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>

#include "maze.h"

#ifdef SOUND
#include <SDL/SDL_mixer.h>
Mix_Chunk *sound[5];
Mix_Music *music; 
#endif

#ifdef SOUND
static Mix_Chunk *load_sound(char * name) {
  extern char *gm_path[];
  Mix_Chunk * sound;
  int i;
  char str[128];

  /* scan all paths */
  for(i=0;gm_path[i][0]!=0;i++) {
    strcpy(str, gm_path[i]);
    strcat(str, name);
    
    /* first look at current directory */
    if((sound = Mix_LoadWAV(str)))
      return sound;
  }

  return NULL;
}

static Mix_Music *load_music(char * name) {
  extern char *gm_path[];
  Mix_Music *music;
  int i;
  char str[128];

  /* scan all paths */
  for(i=0;gm_path[i][0]!=0;i++) {
    strcpy(str, gm_path[i]);
    strcat(str, name);
    
    /* first look at current directory */
    if((music = Mix_LoadMUS(str)))
      return music;
  }

  return NULL;
}

#endif

void InitSound(void) {
#ifdef SOUND
  printf("Init sound ...\n");

  if ( SDL_Init(SDL_INIT_AUDIO) < 0 ) {
    fprintf(stderr, "Unable to initialize SDL Audio: %s\n", SDL_GetError());
    exit(1);
  }

  if (Mix_OpenAudio(44100, AUDIO_S16, 2, 512) < 0) {
    fprintf(stderr, "Error: %s\n", SDL_GetError());
  }

  music = load_music("sounds/music.mod");
  sound[SOUND_SHOT]   = load_sound("sounds/shot.wav");
  sound[SOUND_BUTTON] = load_sound("sounds/button.wav");
  sound[SOUND_WALL]   = load_sound("sounds/wall.wav");
  sound[SOUND_HIT]    = load_sound("sounds/hit.wav");
  sound[SOUND_KILL]   = load_sound("sounds/kill.wav");
#endif
}

void SoundPlay(int sound_no, int channel) {
#ifdef SOUND
  if(prefs.sound)
    Mix_PlayChannel(-1, sound[sound_no], 0);
#endif
}

void SoundMusic(void) {
#ifdef SOUND
  if(prefs.music) Mix_PlayMusic(music,-1);
  else            Mix_HaltMusic();
#endif
}

void FreeSound(void) {
#ifdef SOUND
  Mix_HaltChannel(-1);
  Mix_CloseAudio();
#endif
}

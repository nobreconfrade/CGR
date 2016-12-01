/*

  maze.h  -  part of glmaze

  (c) 2001 by Till Harbaum

*/

#include <GL/gl.h>
#include <GL/glu.h>

#include "SDL.h"

#undef SINGLE_TEST    // define this to test the game with one display

#define MAX_TEX 32
#define MAX_EXT 32

#define MAX_NAMELEN 8  // max user name len

#define FILTER   // use mip mapping

#define OB_X 0
#define OB_Y 1
#define OB_Z 2

#define MAX_HEALTH    16

#define FRAME_AVERAGE 10  /* average timer over n frames */

/* collision check assumes object are cubes (spheres otherwise) */
#undef SQUARE_CHECK

/* object sizes */
#define PLAYER_RADIUS      0.3
#define SHOT_RADIUS        0.1
#define SHOT_COLL_RADIUS   0.03

/* game state */
#define STATE_ACTIVE 0   /* game is running               */
#define STATE_PAUSED 1   /* player is watching the menu   */
#define STATE_WAIT   2   /* menu before first round       */

/* Set up some booleans */
#define TRUE  1
#define FALSE 0

#ifdef SINGLE_TEST
#define MAX_KILLS 2
#else
#define MAX_KILLS 10
#endif

/* preferences struct */
typedef struct {
  char maze[128];   /* filename of maze to load */
  char name[9];     /* player name */
  int  mode;        /* videomode */
  int  fullscreen;  /* fullscreen flag */
  int  sound;       /* sound flag */
  int  music;       /* music flag */
} PREFS;

/* info needed to determine position of object */
typedef struct {
  float pos[3];
  float x_vec[3], y_vec[3], z_vec[3];
} MAPPED_OBJECT;

#define MAX_SHOTS  8
#define MAX_PLAYER 6

/* shot info */
typedef struct {
  int mapped;
  MAPPED_OBJECT map;
  float dist;
} SHOT;

/* other players shot info (no vectors) */
typedef struct {
  int mapped;
  float pos[3];
  int color;
} O_SHOT;

/* other player info */
typedef struct {
  unsigned char mapped, hit, health, kills;
  int score;
  MAPPED_OBJECT map;
} PLAYER;

/* player itself info */
typedef struct {
  unsigned char hit, health, kills;
  int score;
} SPLAYER;

/* just one position */
typedef struct {
  float pos[3];
} POS;

/* just one position with color */
typedef struct {
  float pos[3];
  int color;
} CPOS;

#define SQ(a) ((a)*(a))
#define DIST2(a,b)   (SQ(a)+SQ(b))
#define DIST3(a,b,c) (SQ(a)+SQ(b)+SQ(c))

#define MASTER 0

#define STEP_RES 0.01

/* constant used for converting to radians */
#define PIOVER180 0.0174532925f

#define SCREEN_BPP 16

/* sounds ... */
#define SOUND_SHOT   0
#define SOUND_BUTTON 1
#define SOUND_WALL   2
#define SOUND_HIT    3
#define SOUND_KILL   4

/* main.c */
extern int screen_width, screen_height;
extern int screen_width_tab[], screen_height_tab[], screen_mode;
extern int me, game_state;
extern PLAYER player[];
extern GLuint texture[MAX_TEX]; /* Storage for textures */
extern int check_step(MAPPED_OBJECT obj, float radius);
extern void rotate_object(MAPPED_OBJECT *obj, int axis, float angle);
extern void init_object(MAPPED_OBJECT *obj, float *pos);
extern void mouse(int show);

/* load_maze.c */
extern int load_maze(char *filename);
extern int extend[3];
extern int wall[3][MAX_EXT][MAX_EXT][MAX_EXT];
extern int convert_textures(void);

/* controls.c */
extern void DoControl(void);
extern int InitControl();
extern void FreeControl(void);

/* shot.c */
extern O_SHOT o_shot[];
extern SHOT shot[];
extern const  float player_rgb[MAX_PLAYER][3];
extern int  local_hit_map;
extern void InitShots(void);
extern void FreeShots(void);
extern void DoShots(GLint timer);
extern void FireShot(void);
extern void CheckShot(void);

extern SDL_Surface  *torgb(SDL_Surface  *s);
extern int  LoadTexture(GLint *tex, char *name, int mode);
extern SDL_Surface *ExtSDL_LoadBMP( char *name );
extern FILE *Extfopen(const char *name, const char *mode);

/* player.c */
extern GLint glPlayer[MAX_PLAYER];
extern void InitPlayer(void);
extern void FreePlayer(void);
extern void DoPlayer(void);

/* net.c */
extern int  InitNet(char *arg, float *start);
extern void FreeNet(void);
extern int  NetHandle(void);
extern char user_name[MAX_PLAYER][MAX_NAMELEN+1];
extern void NetStart(void);
extern int  winner;

/* username.c */
extern char *get_player_name(void);

/* radar.c */
extern void InitRadar(void);
extern void DrawRadar(void);

/* prefs.c */
extern PREFS prefs;
extern void LoadPrefs(void);
extern void SavePrefs(void);

/* sound.c */
extern void InitSound(void);
extern void FreeSound(void);
extern void SoundPlay(int type, int channel);
extern void SoundMusic(void);

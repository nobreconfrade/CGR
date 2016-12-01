/*
  net.c - part of glmaze

  (c) 2001 by Till Harbaum

 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>

#include "maze.h"

#define PORT 30571

int winner=-1;

pid_t pid;
int client_sock;
int server_running = FALSE;
char player_mask=0;

/* server stuff ... */
float srv_hit[MAX_PLAYER];
float srv_shield[MAX_PLAYER];
int   srv_score[MAX_PLAYER];
int   srv_kills[MAX_PLAYER];
int   srv_health[MAX_PLAYER];
int   srv_run = FALSE;

char  user_name[MAX_PLAYER][MAX_NAMELEN+1];

/* server state table for each shot */
O_SHOT srv_shot[MAX_PLAYER][MAX_SHOTS];

void NetSend(int socket, char *msg, unsigned char len) {
  write(socket, &len, 1);           /* send length of message */
  write(socket, msg, len);          /* send message           */
}

int NetReceive(int socket, char *buffer, int len) {
  unsigned char rlen;
  int r;
    
  /* read length byte */
  r = read(socket, &rlen, 1);

  if(r<0) perror("read");
  if(r<=0) return r;

  /* this should never happen */
  if(rlen > len) {
    printf("receive: buffer overflow\n");
    exit(1);
  }

  len = rlen; /* save length */

  do {
    r = read(socket, buffer, rlen);

    if(r < 0) perror("read");
    else { 
      /* successfully read some bytes */
      buffer += r;
      rlen -= r;
    }
  } while((rlen>0)&&(r>0));

  if(r <= 0) return r;

  return len;
}

void sighandler(int sig) {
  puts("Server received SIGTERM, exiting.");
  exit(1);
}

void get_new_position(float *ret) {
  int j,i,n,best;
  float bdist, dist, nd;
  POS new;
  int start_pos[][3] = { 
    {0,0,0}, {0,0,1}, {0,1,0}, {0,1,1},
    {1,0,0}, {1,0,1}, {1,1,0}, {1,1,1}};

  /* scan all 8 corners */
  for(bdist=0,best=0,j=0;j<8;j++) {
    dist = 10000.0;

    for(n=0;n<3;n++) {
      if(start_pos[j][n]) new.pos[n] = extend[n] - 0.5;
      else                new.pos[n] = 0.5;
    }

    /* check distance to nearest other player */
    for(i=0;i<MAX_PLAYER;i++) {
      if(player[i].mapped) {	
	nd = ( SQ(new.pos[OB_X]-player[i].map.pos[OB_X])+
	       SQ(new.pos[OB_Y]-player[i].map.pos[OB_Y])+
	       SQ(new.pos[OB_Z]-player[i].map.pos[OB_Z]));
	
	if(nd < dist) dist = nd;
      }
    }

    /* is this position better than the last one? */
    if(dist>bdist) {
      bdist = dist;
      best = j;
    }
  }

  /* return best start position */
  for(n=0;n<3;n++) {
    if(start_pos[best][n]) ret[n] = extend[n] - 0.5;
    else                   ret[n] = 0.5;
  }

  /* make sure players looks into maze */
  ret[3] = (start_pos[best][2])?1.0:-1.0;
}

void check_hit_players(void) {
    struct timeval tv;
  long long current;
  float delay;
  static long long last_check=-1;
  int i;

  gettimeofday(&tv, NULL);
  current = ((long long)tv.tv_sec * 1000000) + tv.tv_usec;

  /* process hits */
  if(last_check > 0) {
    delay = current - last_check;

    /* update hit state of all players */
    for(i=0;i<MAX_PLAYER;i++) {
      if(srv_hit[i]    > 0.0) srv_hit[i]    -= (delay/100000.0);
      if(srv_shield[i] > 0.0) srv_shield[i] -= (delay/1000000.0);
    }
  }

  /* save time of last time check */
  last_check = current;
}

void restart_player(int no, int socket) {
  float restart[4];
  char str[128];

  player[no].mapped = FALSE;      /* hide player */
  srv_health[no]   = MAX_HEALTH;  /* revive player */

  /* get a new position for this player and tell it */
  get_new_position(restart);
  strcpy(str, "RST");
  memcpy(str+3, restart, sizeof(restart));
  NetSend(socket, str, 3+sizeof(restart));
}

void server(void) {
  int listen_sock, cs, csock[MAX_PLAYER], len;
  struct sockaddr_in sin, from;
  int i, n, m, j, quit=FALSE;
  fd_set rfds;
  struct timeval tv;
  char msg[1024];
  POS *pos;
  CPOS other_shot[(MAX_PLAYER-1) * MAX_SHOTS];
  float start_pos[4];
  SPLAYER self;

  /* erase shot/server structures */
  for(i=0;i<MAX_PLAYER;i++) {
    csock[i]=-1;
    player[i].mapped = FALSE;
    srv_hit[i]       = 0.0;
    srv_shield[i]    = 0.0;
    srv_score[i]     = 0;
    srv_kills[i]     = 0;
    srv_health[i]    = MAX_HEALTH;
    user_name[i][0]  = 0;

    for(j=0;j<MAX_SHOTS;j++)
      srv_shot[i][j].mapped = FALSE;
  }

  signal(SIGTERM, sighandler);

  sin.sin_family=AF_INET;
  sin.sin_port=htons(PORT);
  sin.sin_addr.s_addr=htonl(INADDR_ANY);

  /* open listen socket */
  listen_sock = socket(PF_INET, SOCK_STREAM, 0);
  if(listen_sock < 0) {
    perror("server: opening listen socket");
    exit(1);
  }

  /* bind to address */
  if(bind(listen_sock, (struct sockaddr *)&sin, sizeof(sin))<0) {
    perror("bind listen socket");
    exit(1);
  }
  
  if(listen(listen_sock, MAX_PLAYER)<0) {
    perror("server: listen");
    exit(1);
  }

  while(!quit) {
    /* set timeout to 10 sec */
    tv.tv_usec = 0;
    tv.tv_sec = 10;

    FD_ZERO(&rfds);
    for(cs=-1,i=MAX_PLAYER-1;i>=0;i--) {
      if(csock[i] != -1) FD_SET(csock[i], &rfds);
      else               cs = i;           
    } 

    /* still free socket buffers? */
    if(cs != -1) FD_SET(listen_sock, &rfds);

    i = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);
    if(i < 0) perror("select");
    else if(i == 0) {
      puts("server: timeout, exiting");
      quit = TRUE;
    } else {
      check_hit_players();

      /* a new client ... */
      if(FD_ISSET(listen_sock, &rfds)) {
	len = sizeof(from);
	if ((csock[cs] = accept(listen_sock, (struct sockaddr*)&from, &len)) < 0) {
	  perror("server exiting: accept");
	  exit(1);
        }

	printf("server: new client %d\n", cs);
      } else {
	for(i=0;i<MAX_PLAYER;i++) {
	  if(csock[i] != -1) {
	    if(FD_ISSET(csock[i], &rfds)) {
	      if(NetReceive(csock[i], msg, 1024)>0) {
		/* command used to get client id */
		if(strncmp(msg, "GET", 3) == 0) {

		  /* get a free start position */
		  get_new_position(start_pos);

		  /* reset all kind of stuff */
		  srv_hit[i]       = 0.0;
		  srv_score[i]     = 0;
		  srv_kills[i]     = 0;
		  srv_health[i]    = MAX_HEALTH;

		  /* synchonous reply */
		  msg[0]=i; memcpy(msg+1, start_pos, sizeof(start_pos));
		  NetSend(csock[i], msg, 1+sizeof(start_pos));

		  /* game running? let player join */
		  if(srv_run) NetSend(csock[i], "START", 5);

		  /* command to trigger server timeout */
		} else if(strncmp(msg, "PING", 4) == 0) {
		  /* just do nothing ... */ 

		  /* master client wants to start game */
		} else if(strncmp(msg, "START", 5) == 0) {
		  srv_run = TRUE;

		  /* forward this to all clients */
		  for(n=0;n<MAX_PLAYER;n++) {
		    /* reset all kind of stuff */
		    srv_hit[n]       = 0.0;
		    srv_score[n]     = 0;
		    srv_kills[n]     = 0;
		    srv_health[n]    = MAX_HEALTH;
		      
		    if((n!=i)&&(player[n].mapped)) {
		      for(j=0;j<MAX_SHOTS;j++)
			srv_shot[i][j].mapped = FALSE;
		      NetSend(csock[n], "START", 5);
		    }
		  }
		  
		  /* player position */
		} else if(strncmp(msg, "POS", 3) == 0) {
		  /* save hit state from being overwritten */
		  memcpy(&player[i], msg+3, sizeof(PLAYER));

		  /* rotate player image to face right direction */
		  rotate_object(&player[i].map, OB_X, 90.0);

		  /* mark this player as valid */
		  player[i].mapped = TRUE;
		  player_mask |= (1<<i);

		  /* send mask */
		  sprintf(msg, "MAP%c", player_mask);
		  NetSend(csock[i], msg, 4);

		  for(n=0;n<MAX_PLAYER;n++) {
		    if((player[n].mapped)&&(n!=i)) { 

		      /* set hit state */
		      if(srv_hit[n] > 0.0) player[n].hit = 100 * srv_hit[n];
		      else                 player[n].hit = 0;

		      /* set health, kills and score */
		      player[n].health = srv_health[n];
		      player[n].kills  = srv_kills[n];
		      player[n].score  = srv_score[n];

		      /* send position of all other players */ 
		      sprintf(msg, "PLR%c", n);
		      memcpy(msg+4, &player[n], sizeof(PLAYER));
		      NetSend(csock[i], msg, 4+sizeof(PLAYER));
		    }
		    
		    /* the player itself only gets hit state, health etc */
		    if(n==i) {
		      sprintf(msg, "PLR%c", n);

		      if(srv_hit[n] > 0) self.hit = 100 * srv_hit[n];
		      else               self.hit = 0;

		      self.health = srv_health[n];
		      self.kills  = srv_kills[n];
		      self.score  = srv_score[n];

		      memcpy(msg+4, &self, sizeof(SPLAYER));
		      NetSend(csock[i], msg, 4+sizeof(SPLAYER));
		    }
		  }

		  /* player name */
		} else if(strncmp(msg, "NAM", 3) == 0) {
		  strcpy(user_name[i], msg+3);

		  /* answer with all names to all active clients */
		  for(n=0;n<MAX_PLAYER;n++) {
		    if(csock[n]>=0) {
		      strcpy(msg, "NAM");
		      memcpy(msg+3, user_name, sizeof(user_name));
		      NetSend(csock[n], msg, 3+sizeof(user_name));
		    }
		  }

		  /* shot info */
		} else if(strncmp(msg, "SHT", 3) == 0) {

		  /* and mark player as hit and handle health and kills */
		  for(n=0;n<MAX_PLAYER;n++) {
		    /* only handle hits without shields */
		    if((msg[3] & (1<<n))&&(srv_shield[n]<=0.0)) {
		      srv_hit[n] = 1.0;
		      srv_health[n] -= 1;
		      srv_score[i]  +=1;   // 1 point for hit

		      /* player has been killed? */
		      if(srv_health[n] == 0) {
			srv_shield[n] = 1.0;

			restart_player(n, csock[n]);
			srv_kills[i]++;
			srv_score[i]+=10;  // 10 points for kill

			if(srv_kills[i] == MAX_KILLS) {
			  srv_run = FALSE;

			  /* game ended, tell this all clients */
			  for(m=0;m<MAX_PLAYER;m++) {
			    if((player[m].mapped) || (m==n)) {
			      printf("send stop to %d\n", m);
			      NetSend(csock[m], "STOP", 4);
			    }
			  }
			}
		      }
		    }
		  }

		  /* save shots in the net shot list */
		  for(pos=(POS*)(msg+5),n=0;n<MAX_SHOTS;n++,pos++) {
		    if(n<msg[4]) {
		      srv_shot[i][n].mapped    = TRUE;
		      srv_shot[i][n].pos[OB_X] = pos->pos[OB_X];
		      srv_shot[i][n].pos[OB_Y] = pos->pos[OB_Y];
		      srv_shot[i][n].pos[OB_Z] = pos->pos[OB_Z];
		    } else
		      srv_shot[i][n].mapped    = FALSE;
		  }

		  /* reply with shots of other players */
		  strcpy(msg, "SHT"); 

		  /* generate network shot data */
		  for(j=0,n=0;n<MAX_PLAYER;n++) {
		    if(n!=i) {
		      for(m=0;m<MAX_SHOTS;m++) {
			if(srv_shot[n][m].mapped) {
			  other_shot[j].color = n;
			  other_shot[j].pos[OB_X] = srv_shot[n][m].pos[OB_X];
			  other_shot[j].pos[OB_Y] = srv_shot[n][m].pos[OB_Y];
			  other_shot[j].pos[OB_Z] = srv_shot[n][m].pos[OB_Z];
			  j++;
			}
		      }
		    }

		    /* and send reply */
		    msg[3]=j;
		    memcpy(msg+4, other_shot, j*sizeof(CPOS));
		    NetSend(csock[i], msg, 4+j*sizeof(CPOS));
		  }

		  /* client wants to quit */
		} else if(strncmp(msg, "QUIT", 4) == 0) {
		  printf("server: client %d quit\n", i);

		  close(csock[i]);
		  csock[i]=-1;

		  player[i].mapped = FALSE;
		  player_mask &= ~(1<<i);

		  /* erase all active shots */
		  for(j=0;j<MAX_SHOTS;j++)
		    srv_shot[i][j].mapped = FALSE;
		  
		  /* stop if master client goes */
		  if(i==0) quit = TRUE;

		  /* dunno what this is ... */
		} else {
		  printf("server: unknown message from client %d = %c%c%c\n", 
			 i, msg[0],msg[1],msg[2]);
		}
	      } else {
		printf("server: client %d lost\n", i);
		close(csock[i]);
		csock[i]=-1;

		player[i].mapped = FALSE;
		player_mask &= ~(1<<i);

		/* erase all active shots */
		for(j=0;j<MAX_SHOTS;j++)
		  srv_shot[i][j].mapped = FALSE;

		/* stop if master client goes */
		if(i==0) quit = TRUE;
	      }
	    }
	  }
	}
      }
    }
  }

  /* closing connections to all clients */
  for(i=0;i<MAX_PLAYER;i++)
    if(csock[i] != -1)
      close(csock[i]);

  puts("server: terminating");
  close(listen_sock);
  exit(1);
}

int InitNet(char *dest, float *start) {
  struct  sockaddr_in     sin;
  struct  hostent         *hp;
  int i;
  char msg[32];

  if(dest == NULL) {
    if((pid = fork())<0) {
      perror("forking network server");
      return FALSE;
    }

    /* start a comm server */
    if(pid == 0) server();

    server_running = TRUE;
    
    if ((hp=gethostbyname("localhost")) == NULL) {
      perror("gethostbyname(\"localhost\")");
      return FALSE;
    }
  } else {
    printf("Starting network client ...\n");

    if ((hp=gethostbyname(dest)) == NULL) {
      perror("gethostbyname()");
      return FALSE;
    }
  }

  bzero((char *)&sin, sizeof(sin));
  sin.sin_family=AF_INET;
  bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
  sin.sin_port = htons(PORT);

  if ((client_sock=socket(AF_INET,SOCK_STREAM,0)) < 0) {
    perror("client: socket");
    return FALSE;
  }

  /* retry ten times to give server some time to startup */
  i=0;
  do {
    if (connect(client_sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
      i++;
      usleep(10000);  /* wait 10 msec */
    } else
      i=42;
  } while(i<10);

  if(i!=42) {
    perror("client: connect");
    return FALSE;
  }

  /* say hello to server */
  NetSend(client_sock, "GET", 3);
  NetReceive(client_sock, msg, 32);
  me = msg[0];

  /* get start position */
  memcpy(start, &msg[1], 4*sizeof(float));

  /* send own name */
  sprintf(msg, "NAM%s", prefs.name);
  NetSend(client_sock, msg, 3+MAX_NAMELEN+1);

  /* erase all names */
  for(i=0;i<MAX_PLAYER;i++) user_name[i][0]=0;

  return TRUE;
}

void FreeNet(void) {

  /* say good bye */
  NetSend(client_sock, "QUIT", 4);

  close(client_sock);
  sleep(1);

  if(server_running) {
    if(kill(pid, SIGTERM)==-1)
      perror("killing server");
  }
}

int NetHandle(void) {
  char str[1024];
  int i,j, sel, score;
  fd_set rfds;
  struct timeval tv;
  POS my_shot[MAX_SHOTS];
  CPOS *p;
  SPLAYER self;
  float pos[4];

  /* build position message */
  strcpy(str, "POS");
  memcpy(str+3, &player[me], sizeof(PLAYER));
  NetSend(client_sock, str, 3+sizeof(PLAYER));

  /* build shot message */
  strcpy(str, "SHT"); 
  str[3]=local_hit_map;   /* send and */
  local_hit_map = 0;      /* erase hit map */

  /* generate network shot data */
  for(j=0,i=0;i<MAX_SHOTS;i++) {
    if(shot[i].mapped) {
      my_shot[j].pos[OB_X] = shot[i].map.pos[OB_X];
      my_shot[j].pos[OB_Y] = shot[i].map.pos[OB_Y];
      my_shot[j].pos[OB_Z] = shot[i].map.pos[OB_Z];
      j++;
    }
  }

  /* and send info about all shots currently active */
  str[4]=j;
  memcpy(str+5, my_shot, j*sizeof(POS));
  NetSend(client_sock, str, 5+j*sizeof(POS));

  do {
    /* check for incoming messages */
    tv.tv_usec = tv.tv_sec = 0;  /* return immediately */
    FD_ZERO(&rfds);
    FD_SET(client_sock, &rfds);
    
    sel = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);
    if(sel<0) perror("client select");
    else if(sel>0) {
      i=NetReceive(client_sock, str, 1024);
      if(i==0) {
	printf("connection to server lost, exiting!\n");
	return FALSE;
      }
      
      if(strncmp(str,"START", 5)==0) {
	mouse( FALSE );
	game_state = STATE_ACTIVE;
      }

      if(strncmp(str,"STOP", 4)==0) {

	/* check for the winner */
	for(winner=-1,score=0,j=0;j<MAX_PLAYER;j++) {
	  if(player[j].score > score) {
	    score = player[j].score;
	    winner = j;
	  }
	}

	mouse( TRUE );
	game_state = STATE_WAIT;
      }

      else if(strncmp(str,"MAP", 3)==0) {
	player_mask = str[3];

	for(j=0;j<MAX_PLAYER;j++) 
	  if(j!=me)
	    player[j].mapped = player_mask & (1<<j);
      }

      else if(strncmp(str,"SHT", 3)==0) {
	/* transfer other players shots */
	p = (CPOS*)(str+4);
	for(j=0;j<str[3];j++) {
	  o_shot[j].mapped    = TRUE;
	  o_shot[j].color     = p->color;
	  o_shot[j].pos[OB_X] = p->pos[OB_X];
	  o_shot[j].pos[OB_Y] = p->pos[OB_Y];
	  o_shot[j].pos[OB_Z] = p->pos[OB_Z];
	  p++;
	}

	/* disable remaining entries */
	while(j<((MAX_PLAYER-1)*MAX_SHOTS))
	  o_shot[j++].mapped = FALSE;
      }

      /* received all player names */
      else if(strncmp(str, "NAM", 3)==0) {
	memcpy(user_name, str+3, sizeof(user_name));
      }

      /* scott me up beamy */
      else if(strncmp(str, "RST", 3)==0) {
	memcpy(pos, str+3, sizeof(pos));
	init_object(&player[me].map, pos);
      }

      /* received player position */
      else if(strncmp(str, "PLR", 3)==0) {
	i=str[3];

	if(i!=me) memcpy(&player[i], str+4, sizeof(PLAYER));
	else {
	  /* player itself only reduced info (no pos) */
	  memcpy(&self, str+4, sizeof(SPLAYER));

	  /* player has just been hit */
	  if((player[i].hit > 0.0) && (self.hit == 0.0))
	    SoundPlay(SOUND_KILL, 0);

	  player[i].hit    = self.hit;
	  player[i].kills  = self.kills;
	  player[i].health = self.health;
	  player[i].score  = self.score;
	}
      }
    }
  } while(sel != 0);

  return TRUE;
}

/* master client tells server, that he wants to start the game */
void NetStart(void) {
  NetSend(client_sock, "START", 5);
}

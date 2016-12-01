CC = gcc -O2 -ffast-math -Wall

VERSION = "1.1a"

DESTPATH = /usr/local
BINPATH = $(DESTPATH)/bin
GAMEPATH = $(DESTPATH)/games/glMaze
MANPATH = $(DESTPATH)/man/man1

OBJECTS = load_maze.o main.o controls.o shot.o player.o net.o \
	  username.o radar.o prefs.o sound.o

SDL_FLAGS=`sdl-config --cflags`
SDL_LIBS=`sdl-config --libs`

# comment the follwing if you don't want sound
CC += -DSOUND
SDL_LIBS += -lSDL_mixer

all: glmaze
	cd pov; make

load_maze.o: load_maze.c maze.h
main.o: main.c maze.h
controls.o: controls.c maze.h
shot.o: shot.c maze.h
player.o: player.c maze.h
net.o: net.c maze.h
username.o: username.c maze.h
radar.o: radar.c maze.h
prefs.o: prefs.c maze.h

.c.o:
	$(CC) -DVERSION=\"$(VERSION)\" $(SDL_FLAGS) -c -o $@ $<

glmaze: $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(SDL_LIBS) -lGL -lGLU -lm 

clean:
	@echo Cleaning up...
	rm -f *.o *~ */*~ core
	rm -rf levels/.xvpics data/.xvpics
	@echo Done.

dist:
	make clean; rm -f glmaze
	cd ..; tar cvfz glmaze-$(VERSION).tar.gz glmaze

install: glmaze
	install -s glmaze $(BINPATH)
	install glmaze.1 $(MANPATH)
	install -d $(GAMEPATH)/data
	install data/*.* $(GAMEPATH)/data
	install -d $(GAMEPATH)/levels
	install levels/*.* $(GAMEPATH)/levels
	install -d $(GAMEPATH)/sounds
	install sounds/*.* $(GAMEPATH)/sounds

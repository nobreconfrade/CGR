#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ESCAPE 27
#define FRONT 0x1
#define LEFT  0x2
#define RIGHT 0x4
#define BACK  0x8
#define TOP   0x8 << 1
#define BOTTOM  0x8 << 2

int window;

GLuint loop;
GLuint texture[3];

int light = 0;
int blend = 0;

// GLfloat yrot = 45.0f;
GLfloat yrot = 0.0f;
GLfloat lookupdown = 0.0;

float xpos = 38.0f, ypos = 0.0f,zpos = -5.9f;
float camerax = 0.0f, cameray = 0.0f, cameraz = 0.0f;

GLfloat LightAmbient[]  = {0.5f, 0.5f, 0.5f, 1.0f};
GLfloat LightDiffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat LightPosition[] = {0.0f, 0.0f, 2.0f, 1.0f};

GLuint filter = 0;

float MDeltaAnglex = 0.0f,MDeltaAngley = 0.0f, anglex = 0.0,angley = 0.0;
int MxOrigin = -1,MyOrigin = -1;

typedef struct {
    unsigned long sizeX;
    unsigned long sizeY;
    char *data;
} Image;

void DrawCube(int type, int i, int j);

int ImageLoad(char *filename, Image *image);

float rquad = 0.0f;

char maz[15][15]={{'*', '*', '*', '*', '*', '*', '*', '*', '*','*','*','*','*','*','*'},
				 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',' ',' ',' ',' ',' ','*'},
				 {'*', ' ', '*', '*', '*', ' ', '*', '*', '*','*','*',' ','*','*','*'},
				 {'*', ' ', '*', ' ', ' ', ' ', '*', ' ', ' ',' ',' ',' ','*',' ','*'},
				 {'*', ' ', '*', ' ', '*', ' ', '*', '*', '*','*','*','*','*',' ','*'},
				 {'*', ' ', '*', ' ', ' ', ' ', '*', ' ', ' ',' ',' ',' ',' ',' ','*'},
				 {'*', ' ', ' ', ' ', '*', '*', '*', ' ', '*','*','*',' ','*',' ','*'},
				 {'*', ' ', '*', ' ', '*', '*', ' ', ' ', '*',' ',' ',' ','*',' ','*'},
				 {'*', ' ', '*', ' ', ' ', '*', ' ', '*', '*','*','*',' ','*','*','*'},
                 {'*', '*', '*', '*', ' ', ' ', ' ', '*', ' ','*','*',' ',' ',' ','*'},
				 {'*', ' ', ' ', ' ', ' ', '*', ' ', '*', ' ',' ',' ',' ','*','*','*'},
				 {'*', ' ', '*', '*', '*', '*', ' ', '*', ' ','*','*','*','*','*','*'},
				 {'*', ' ', '*', ' ', ' ', ' ', ' ', '*', ' ',' ',' ',' ',' ',' ','*'},
				 {'*', ' ', ' ', ' ', '*', '*', ' ', '*', '*',' ','*','*','*',' ','*'},
				 {'*', '*', '*', '*', '*', '*', '*', '*', '*','*','*','*','*',' ','*'}};

void InitGL2(int Width, int Height)
{
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClearDepth(1.0);
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);

  glMatrixMode(GL_MODELVIEW);
}

static unsigned int getint(fp)
     FILE *fp;
{
  int c, c1, c2, c3;

  c = getc(fp);
  c1 = getc(fp);
  c2 = getc(fp);
  c3 = getc(fp);

  return ((unsigned int) c) +
    (((unsigned int) c1) << 8) +
    (((unsigned int) c2) << 16) +
    (((unsigned int) c3) << 24);
}

static unsigned int getshort(fp)
     FILE *fp;
{
  int c, c1;

  c = getc(fp);
  c1 = getc(fp);

  return ((unsigned int) c) + (((unsigned int) c1) << 8);
}

int ImageLoad(char *filename, Image *image)
{
    FILE *file;
    unsigned long size;
    unsigned long i;
    unsigned short int planes;
    unsigned short int bpp;
    char temp;

    if ((file = fopen(filename, "rb"))==NULL) {
      printf("File Not Found : %s\n",filename);
      return 0;
    }

    fseek(file, 18, SEEK_CUR);


    image->sizeX = getint (file);
    /*printf("Width of %s: %lu\n", filename, image->sizeX);*/

    image->sizeY = getint (file);
    /*printf("Height of %s: %lu\n", filename, image->sizeY);*/

    size = image->sizeX * image->sizeY * 3;

    planes = getshort(file);
    if (planes != 1) {
	printf("Planes from %s is not 1: %u\n", filename, planes);
	return 0;
    }

    bpp = getshort(file);
    if (bpp != 24) {
      printf("Bpp from %s is not 24: %u\n", filename, bpp);
      return 0;
    }

    fseek(file, 24, SEEK_CUR);

    image->data = (char *) malloc(size);
    if (image->data == NULL) {
	printf("Error allocating memory for color-corrected image data");
	return 0;
    }

    if ((i = fread(image->data, size, 1, file)) != 1) {
	printf("Error reading image data from %s.\n", filename);
	return 0;
    }

    for (i=0;i<size;i+=3) { //(bgr -> rgb)
	temp = image->data[i];
	image->data[i] = image->data[i+2];
	image->data[i+2] = temp;
    }

    return 1;
}

GLvoid LoadGLTextures(GLvoid)
{
    Image *image1;

    image1 = (Image *) malloc(sizeof(Image));
    if (image1 == NULL) {
	printf("Error allocating space for image");
	exit(0);
    }

    if (!ImageLoad("Imagens/stell.bmp", image1)) {
	exit(1);
    }

    glGenTextures(3, &texture[0]);

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);

    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, image1->sizeX, image1->sizeY, GL_RGB, GL_UNSIGNED_BYTE, image1->data);
};

GLvoid InitGL(GLsizei Width, GLsizei Height)
{
    LoadGLTextures();
    glEnable(GL_TEXTURE_2D);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);

    glMatrixMode(GL_MODELVIEW);

    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
    glEnable(GL_LIGHT1);
}

void ReSizeGLScene(int Width, int Height)
{
  if (Height==0)
    Height=1;

  glViewport(0, 0, Width, Height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);
  glMatrixMode(GL_MODELVIEW);
}

void DrawGLScene()
{
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glLoadIdentity();

  glTranslatef(-6.0f,0.0f,-40.0f);
  double viewup[3] = {0.0, 1.0, 0.0};
  // double rot = yrot*3.14159 /180.0;
  double rot =  45 *3.14159 /180.0;
  double target[3] = {cos(rot) + sin(rot), lookupdown, -sin(rot) +  cos(rot)}; //Posi��o Inicial da camera


  gluLookAt((double)xpos, 0.0, (double)zpos, xpos + 2* target[0] + camerax, target[1] + cameray, zpos + 2* target[2] +cameraz,  viewup[0],  viewup[1],  viewup[2]);
  glRotatef(yrot, 0.f, 1.f, 0.f);
  int i, j;
  int wall;
  for(i=1; i < 14; i++)
  {
	for(j=1; j < 14; j++)
	{
		wall = BOTTOM;
		if( maz[i][j] == '*') wall |= TOP;
		if( maz[i-1][j] == '*') wall |=BACK;
		if( maz[i+1][j] == '*') wall |=FRONT;
		if( maz[i][j-1] == '*') wall |=LEFT;
		if( maz[i][j+1] == '*') wall |=RIGHT;

		DrawCube(wall,j-1,i-1);
	}
  }


  rquad-=1.0f;

  glutSwapBuffers();
}

void mouseButton(int button, int state, int x, int y) {

	if (button == GLUT_LEFT_BUTTON) {

		if (state == GLUT_UP) {
			anglex += MDeltaAnglex;
			angley += MDeltaAngley;
			MxOrigin = -1;
			MyOrigin = -1;
		}
		else  {
			MxOrigin = x;
			MyOrigin = y;
		}

	}
}

void mouseMove(int x, int y) {

	if (MxOrigin >= 0) {

		MDeltaAnglex = (x - MxOrigin) * 0.1f;

		camerax = sin((anglex + MDeltaAnglex)*3.14159 /180.0);
		cameraz = -cos((anglex + MDeltaAnglex)*3.14159 /180.0);
	}
	if (MyOrigin >=0){

		MDeltaAngley = (y - MyOrigin) * 0.005f;

		cameray = (MDeltaAngley + angley);

	}

}

void DrawCube(int type, int x, int y)
{
  glPushMatrix();
  glTranslatef(x* 2.0f, 0.0f, y * 2.0f);
  glBindTexture(GL_TEXTURE_2D, texture[filter]);
  glBegin(GL_QUADS);

	if(type & TOP)
	{
	  glColor3f(0.0f,1.0f,0.0f);
	  glVertex3f( 1.0f, 1.0f,-1.0f);
	  glTexCoord2f(1.0f,0.0f);
	  glVertex3f(-1.0f, 1.0f,-1.0f);
	  glTexCoord2f(0.0f,0.0f);
          glVertex3f(-1.0f, 1.0f, 1.0f);
	  glTexCoord2f(0.0f,1.0f);
	  glVertex3f( 1.0f, 1.0f, 1.0f);
	  glTexCoord2f(1.0f,1.0f);
	}

   	if(type & BOTTOM)
	{
	  glColor3f(0.0f,1.0f,1.0f);
	  glVertex3f( 1.0f,-1.0f, 1.0f);
	  glTexCoord2f(1.0f,0.0f);
	  glVertex3f(-1.0f,-1.0f, 1.0f);
	  glTexCoord2f(0.0f,0.0f);
	  glVertex3f(-1.0f,-1.0f,-1.0f);
	  glTexCoord2f(0.0f,1.0f);
	  glVertex3f( 1.0f,-1.0f,-1.0f);
	  glTexCoord2f(1.0f,1.0f);
	}
   	if(type & FRONT)
	{
		glColor3f(1.0f,0.0f,0.0f);
		glVertex3f( 1.0f, 1.0f, 1.0f);
		glTexCoord2f(1.0f,0.0f);
		glVertex3f(-1.0f, 1.0f, 1.0f);
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(-1.0f,-1.0f, 1.0f);
		glTexCoord2f(0.0f,1.0f);
		glVertex3f( 1.0f,-1.0f, 1.0f);
		glTexCoord2f(1.0f,1.0f);
	}

	if(type & BACK)
	{
	  glColor3f(1.0f,0.0f,0.0f);
	  glVertex3f( 1.0f,-1.0f,-1.0f);
	  glTexCoord2f(1.0f,0.0f);
	  glVertex3f(-1.0f,-1.0f,-1.0f);
	  glTexCoord2f(0.0f,0.0f);
	  glVertex3f(-1.0f, 1.0f,-1.0f);
	  glTexCoord2f(0.0f,1.0f);
	  glVertex3f( 1.0f, 1.0f,-1.0f);
	  glTexCoord2f(1.0f,1.0f);
	}

	if(type & LEFT)
	{
	  glColor3f(1.0f,0.0f,.0f);
	  glVertex3f(-1.0f, 1.0f, 1.0f);
	  glTexCoord2f(1.0f,0.0f);
	  glVertex3f(-1.0f, 1.0f,-1.0f);
	  glTexCoord2f(0.0f,0.0f);
	  glVertex3f(-1.0f,-1.0f,-1.0f);
	  glTexCoord2f(0.0f,1.0f);
	  glVertex3f(-1.0f,-1.0f, 1.0f);
	  glTexCoord2f(1.0f,1.0f);
	}

	if (type & RIGHT)
	{
	  glColor3f(1.0f,0.0f,.0f);
	  glVertex3f( 1.0f, 1.0f,-1.0f);
	  glTexCoord2f(1.0f,0.0f);
	  glVertex3f( 1.0f, 1.0f, 1.0f);
	  glTexCoord2f(0.0f,0.0f);
	  glVertex3f( 1.0f,-1.0f, 1.0f);
	  glTexCoord2f(0.0f,1.0f);
	  glVertex3f( 1.0f,-1.0f,-1.0f);
	  glTexCoord2f(1.0f,1.0f);
	}
	glEnd();
	glPopMatrix();

}
void keyPressed(unsigned char key, int x, int y)
{
    usleep(100);

    if (key == ESCAPE)
    {
      glutDestroyWindow(window);

      exit(0);
    }
}

void specialKeyPressed(int key, int x, int y)
{
	double rot;

    usleep(100);

    switch (key) {
    case GLUT_KEY_PAGE_UP:
	lookupdown += 0.02f;
	break;

    case GLUT_KEY_PAGE_DOWN:
	lookupdown -= 0.02f;
	break;

    case GLUT_KEY_UP:
	// rot = yrot *3.14159 /180.0;
	rot = 45 *3.14159 /180.0;
	xpos += (float)(cos(rot) + sin(rot)) * 0.08f;
	zpos += (float)(-sin(rot) +  cos(rot)) * 0.08f;
	break;

    case GLUT_KEY_DOWN:
	// rot = yrot *3.14159 /180.0;
	rot = 45 *3.14159 /180.0;
	xpos -= (float)(cos(rot) + sin(rot)) * 0.08f;
	zpos -= (float)(-sin(rot) +  cos(rot)) * 0.08f;
	break;

    case GLUT_KEY_LEFT:
	yrot -= 15.0f;
	break;

    case GLUT_KEY_RIGHT:
	yrot += 15.0f;
	break;

    default:
	printf ("Special key %d pressed. No action there yet.\n", key);
	break;
    }
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);

  glutInitWindowSize(640, 480);

  glutInitWindowPosition(0, 0);

  window = glutCreateWindow("Jeff Molofee's GL Code Tutorial ... NeHe '99");

  glutDisplayFunc(&DrawGLScene);

  glutFullScreen();

  glutIdleFunc(&DrawGLScene);

  glutReshapeFunc(&ReSizeGLScene);

  glutKeyboardFunc(&keyPressed);

  glutSpecialFunc(&specialKeyPressed);

  InitGL(640, 480);

  glutMouseFunc(mouseButton);

  glutMotionFunc(mouseMove);

  glutMainLoop();

  return 1;
}

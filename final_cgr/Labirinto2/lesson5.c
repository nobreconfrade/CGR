//Bibliotecas
#include <GL/glut.h>    // Header File For The GLUT Library
#include <GL/gl.h>	// Header File For The OpenGL32 Library
#include <GL/glu.h>	// Header File For The GLu32 Library
#include <unistd.h>     // Header file for sleeping.
#include <stdio.h>      // Header file for standard file i/o.
#include <stdlib.h>     // Header file for malloc/free.
#include <math.h>       // Header file for trigonometric functions.

/* ASCII code for the escape key. */
#define ESCAPE 27
#define FRONT 0x1
#define LEFT  0x2
#define RIGHT 0x4
#define BACK  0x8
#define TOP   0x8 << 1
#define BOTTOM  0x8 << 2

/* The number of our GLUT window */
int window;

GLuint loop;             // general loop variable
GLuint texture[3];       // storage for 3 textures;

int light = 0;           // lighting on/off
int blend = 0;           // blending on/off

GLfloat yrot = 0.0f;      //Rota��o Horizontal
GLfloat lookupdown = 0.0; //Rota��o Vertical

const float piover180 = 0.0174532925f; //constante para convers�o de graus para radianos

float xpos = 0.0f, ypos = 0.0f,zpos = 0.0f; //Posi��es
float camerax = 0.0f, cameray = 0.0f, cameraz = 0.0f; //cameras

GLfloat LightAmbient[]  = {0.5f, 0.5f, 0.5f, 1.0f};
GLfloat LightDiffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat LightPosition[] = {0.0f, 0.0f, 2.0f, 1.0f};

GLuint filter = 0;       // texture filtering method to use (nearest, linear, linear + mipmaps)

//Variaveis para movimentacao da camera com o mouse
float MDeltaAnglex = 0.0f,MDeltaAngley = 0.0f, anglex = 0.0,angley = 0.0;
int MxOrigin = -1,MyOrigin = -1;

/* Image type - contains height, width, and data */
typedef struct {
    unsigned long sizeX;
    unsigned long sizeY;
    char *data;
} Image;

//Fun��o para desenhar o labirinto com cubos
void DrawCube(int type, int i, int j);

int ImageLoad(char *filename, Image *image);

/* rotation angle for the quadrilateral. */
float rquad = 0.0f;

//Matriz para desenhar o labirinto
char maz[15][15]={{'*', '*', '*', '*', '*', '*', '*', '*', '*',' ',' ',' ',' ',' ',' '},
				 {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '*',' ',' ',' ',' ',' ',' '},
				 {'*', ' ', '*', ' ', '*', '*', '*', ' ', '*',' ',' ',' ',' ',' ',' '},
				 {'*', ' ', '*', ' ', ' ', ' ', '*', ' ', '*',' ',' ',' ',' ',' ',' '},
				 {'*', ' ', '*', '*', ' ', '*', '*', ' ', ' ',' ',' ',' ',' ',' ',' '},
				 {'*', ' ', '*', '*', ' ', '*', '*', ' ', '*',' ',' ',' ',' ',' ',' '},
				 {'*', ' ', '*', ' ', ' ', ' ', ' ', ' ', '*',' ',' ',' ',' ',' ',' '},
				 {'*', ' ', ' ', '*', '*', '*', '*', '*', '*',' ',' ',' ',' ',' ',' '},
				 {'*', '*', '*', '*', '*', '*', '*', '*', '*',' ',' ',' ',' ',' ',' '},
                 {'*', ' ', '*', '*', ' ', '*', '*', ' ', '*',' ',' ',' ',' ',' ',' '},
				 {'*', ' ', '*', ' ', ' ', ' ', ' ', ' ', '*',' ',' ',' ',' ',' ',' '},
				 {'*', ' ', ' ', '*', '*', '*', '*', '*', '*',' ',' ',' ',' ',' ',' '},
				 {'*', '*', '*', '*', '*', '*', '*', '*', '*',' ',' ',' ',' ',' ',' '}};

/* A general OpenGL initialization function.  Sets all of the initial parameters. */
void InitGL2(int Width, int Height)	        // We call this right after our OpenGL window is created.
{
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);		// This Will Clear The Background Color To Black
  glClearDepth(1.0);				// Enables Clearing Of The Depth Buffer
  glDepthFunc(GL_LESS);			        // The Type Of Depth Test To Do
  glEnable(GL_DEPTH_TEST);		        // Enables Depth Testing
  glShadeModel(GL_SMOOTH);			// Enables Smooth Color Shading

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();				// Reset The Projection Matrix

  gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);	// Calculate The Aspect Ratio Of The Window

  glMatrixMode(GL_MODELVIEW);
}

static unsigned int getint(fp)
     FILE *fp;
{
  int c, c1, c2, c3;

  // get 4 bytes
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

  //get 2 bytes
  c = getc(fp);
  c1 = getc(fp);

  return ((unsigned int) c) + (((unsigned int) c1) << 8);
}

//Carregamento da imagem das paredes
int ImageLoad(char *filename, Image *image)
{
    FILE *file;
    unsigned long size;                 // size of the image in bytes.
    unsigned long i;                    // standard counter.
    unsigned short int planes;          // number of planes in image (must be 1)
    unsigned short int bpp;             // number of bits per pixel (must be 24)
    char temp;                          // used to convert bgr to rgb color.

    // make sure the file is there.
    if ((file = fopen(filename, "rb"))==NULL) {
      printf("File Not Found : %s\n",filename);
      return 0;
    }

    // seek through the bmp header, up to the width/height:
    fseek(file, 18, SEEK_CUR);

    // No 100% errorchecking anymore!!!

    // read the width
    image->sizeX = getint (file);
    printf("Width of %s: %lu\n", filename, image->sizeX);

    // read the height
    image->sizeY = getint (file);
    printf("Height of %s: %lu\n", filename, image->sizeY);

    // calculate the size (assuming 24 bits or 3 bytes per pixel).
    size = image->sizeX * image->sizeY * 3;

    // read the planes
    planes = getshort(file);
    if (planes != 1) {
	printf("Planes from %s is not 1: %u\n", filename, planes);
	return 0;
    }

    // read the bpp
    bpp = getshort(file);
    if (bpp != 24) {
      printf("Bpp from %s is not 24: %u\n", filename, bpp);
      return 0;
    }

    // seek past the rest of the bitmap header.
    fseek(file, 24, SEEK_CUR);

    // read the data.
    image->data = (char *) malloc(size);
    if (image->data == NULL) {
	printf("Error allocating memory for color-corrected image data");
	return 0;
    }

    if ((i = fread(image->data, size, 1, file)) != 1) {
	printf("Error reading image data from %s.\n", filename);
	return 0;
    }

    for (i=0;i<size;i+=3) { // reverse all of the colors. (bgr -> rgb)
	temp = image->data[i];
	image->data[i] = image->data[i+2];
	image->data[i+2] = temp;
    }

    // we're done.
    return 1;
}

// Load Bitmaps And Convert To Textures
GLvoid LoadGLTextures(GLvoid)
{
    // Load Texture
    Image *image1;

    // allocate space for texture
    image1 = (Image *) malloc(sizeof(Image));
    if (image1 == NULL) {
	printf("Error allocating space for image");
	exit(0);
    }

    if (!ImageLoad("Imagens/stell.bmp", image1)) {
	exit(1);
    }

    // Create Textures
    glGenTextures(3, &texture[0]);

    // nearest filtered texture
    glBindTexture(GL_TEXTURE_2D, texture[0]);   // 2d texture (x and y size)
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST); // scale cheaply when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); // scale cheaply when image smalled than texture
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);

    // linear filtered texture
    glBindTexture(GL_TEXTURE_2D, texture[1]);   // 2d texture (x and y size)
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);

    // mipmapped texture
    glBindTexture(GL_TEXTURE_2D, texture[2]);   // 2d texture (x and y size)
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST); // scale mipmap when image smalled than texture
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, image1->sizeX, image1->sizeY, GL_RGB, GL_UNSIGNED_BYTE, image1->data);
};

/* A general OpenGL initialization function.  Sets all of the initial parameters. */
GLvoid InitGL(GLsizei Width, GLsizei Height)	// We call this right after our OpenGL window is created.
{
    LoadGLTextures();                           // load the textures.
    glEnable(GL_TEXTURE_2D);                    // Enable texture mapping.

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);          // Set the blending function for translucency (note off at init time)
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// This Will Clear The Background Color To Black
    glClearDepth(1.0);				// Enables Clearing Of The Depth Buffer
    glDepthFunc(GL_LESS);                       // type of depth test to do.
    glEnable(GL_DEPTH_TEST);                    // enables depth testing.
    glShadeModel(GL_SMOOTH);			// Enables Smooth Color Shading

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();				// Reset The Projection Matrix

    gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);	// Calculate The Aspect Ratio Of The Window

    glMatrixMode(GL_MODELVIEW);

    // set up lights.
    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
    glEnable(GL_LIGHT1);
}

/* The function called when our window is resized (which shouldn't happen, because we're fullscreen) */
void ReSizeGLScene(int Width, int Height)
{
  if (Height==0)				// Prevent A Divide By Zero If The Window Is Too Small
    Height=1;

  glViewport(0, 0, Width, Height);		// Reset The Current Viewport And Perspective Transformation

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);
  glMatrixMode(GL_MODELVIEW);
}

/* The main drawing function. */
void DrawGLScene()
{
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
  glLoadIdentity();				// Reset The View

  //(Prototipo): rotaciona o labirinto
  glTranslatef(-6.0f,0.0f,-40.0f);		// Move Left 1.5 Units And Into The Screen 6.0
  //glRotatef(rquad,0.0f,1.0f,1.0f);		// Rotate The Pyramid On The Y axis
  double viewup[3] = {0.0, 1.0, 0.0};
  double rot = yrot *3.14159 /180.0;
  double target[3] = {cos(rot) + sin
	  (rot), lookupdown, -sin(rot) +  cos(rot)}; //Posi��o Inicial da camera


  gluLookAt((double)xpos, 0.0, (double)zpos, xpos + 2* target[0] + camerax, target[1] + cameray, zpos + 2* target[2] +cameraz,  viewup[0],  viewup[1],  viewup[2]);

  // Done Drawing The Cube
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


  rquad-=1.0f;					// Decrease The Rotation Variable For The Cube

  // swap the buffers to display, since double buffering is used.
  glutSwapBuffers();
}

//Fun��es para quando o mouse � precionado
void mouseButton(int button, int state, int x, int y) {

	// only start motion if the left button is pressed
	if (button == GLUT_LEFT_BUTTON) {

		// when the button is released
		if (state == GLUT_UP) {
			anglex += MDeltaAnglex;
			angley += MDeltaAngley;
			MxOrigin = -1;
			MyOrigin = -1;
		}
		else  {// state = GLUT_DOWN
			MxOrigin = x;
			MyOrigin = y;
		}

	}
}

//Fun��o para atualizar o movimento da camera
void mouseMove(int x, int y) {

	// this will only be true when the left button is down
	if (MxOrigin >= 0) {

		// update deltaAngle
		MDeltaAnglex = (x - MxOrigin) * 0.1f;

		// update camera's direction
		camerax = sin((anglex + MDeltaAnglex)*3.14159 /180.0);
		cameraz = -cos((anglex + MDeltaAnglex)*3.14159 /180.0);
	}
	if (MyOrigin >=0){

		MDeltaAngley = (y - MyOrigin) * 0.005f;

		cameray = (MDeltaAngley + angley);

	}

}

//Fun��o para desenhar as paredes
void DrawCube(int type, int x, int y)
{
	  // draw a cube (6 quadrilaterals)
  glPushMatrix();
  glTranslatef(x* 2.0f, 0.0f, y * 2.0f);
  glBindTexture(GL_TEXTURE_2D, texture[filter]);    // pick the texture.
  glBegin(GL_QUADS);				// start drawing the cube.

  // top of cube
	if(type & TOP)
	{
	  glColor3f(0.0f,1.0f,0.0f);			// Set The Color To Blue
	  glVertex3f( 1.0f, 1.0f,-1.0f);		// Top Right Of The Quad (Top)
	  glTexCoord2f(1.0f,0.0f);
	  glVertex3f(-1.0f, 1.0f,-1.0f);		// Top Left Of The Quad (Top)
	  glTexCoord2f(0.0f,0.0f);
	  glVertex3f(-1.0f, 1.0f, 1.0f);		// Bottom Left Of The Quad (Top)
	  glTexCoord2f(0.0f,1.0f);
	  glVertex3f( 1.0f, 1.0f, 1.0f);		// Bottom Right Of The Quad (Top)
	  glTexCoord2f(1.0f,1.0f);
	}

   	if(type & BOTTOM)
	{
	  glColor3f(0.0f,1.0f,1.0f);			// Set The Color To Orange
	  glVertex3f( 1.0f,-1.0f, 1.0f);		// Top Right Of The Quad (Bottom)
	  glTexCoord2f(1.0f,0.0f);
	  glVertex3f(-1.0f,-1.0f, 1.0f);		// Top Left Of The Quad (Bottom)
	  glTexCoord2f(0.0f,0.0f);
	  glVertex3f(-1.0f,-1.0f,-1.0f);		// Bottom Left Of The Quad (Bottom)
	  glTexCoord2f(0.0f,1.0f);
	  glVertex3f( 1.0f,-1.0f,-1.0f);		// Bottom Right Of The Quad (Bottom)
	  glTexCoord2f(1.0f,1.0f);
	}
   	if(type & FRONT)
	{
		// front of cube
		glColor3f(1.0f,0.0f,0.0f);			// Set The Color To Red
		glVertex3f( 1.0f, 1.0f, 1.0f);		// Top Right Of The Quad (Front)
		glTexCoord2f(1.0f,0.0f);
		glVertex3f(-1.0f, 1.0f, 1.0f);		// Top Left Of The Quad (Front)
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(-1.0f,-1.0f, 1.0f);		// Bottom Left Of The Quad (Front)
		glTexCoord2f(0.0f,1.0f);
		glVertex3f( 1.0f,-1.0f, 1.0f);		// Bottom Right Of The Quad (Front)
		glTexCoord2f(1.0f,1.0f);
	}

	if(type & BACK)
	{
		// back of cube.
	  glColor3f(1.0f,0.0f,0.0f);			// Set The Color To Red
	  glVertex3f( 1.0f,-1.0f,-1.0f);		// Top Right Of The Quad (Back)
	  glTexCoord2f(1.0f,0.0f);
	  glVertex3f(-1.0f,-1.0f,-1.0f);		// Top Left Of The Quad (Back)
	  glTexCoord2f(0.0f,0.0f);
	  glVertex3f(-1.0f, 1.0f,-1.0f);		// Bottom Left Of The Quad (Back)
	  glTexCoord2f(0.0f,1.0f);
	  glVertex3f( 1.0f, 1.0f,-1.0f);		// Bottom Right Of The Quad (Back)
	  glTexCoord2f(1.0f,1.0f);
	}

	if(type & LEFT)
	{
	  // left of cube
	  glColor3f(1.0f,0.0f,.0f);			//// Set The Color To Red
	  glVertex3f(-1.0f, 1.0f, 1.0f);		// Top Right Of The Quad (Left)
	  glTexCoord2f(1.0f,0.0f);
	  glVertex3f(-1.0f, 1.0f,-1.0f);		// Top Left Of The Quad (Left)
	  glTexCoord2f(0.0f,0.0f);
	  glVertex3f(-1.0f,-1.0f,-1.0f);		// Bottom Left Of The Quad (Left)
	  glTexCoord2f(0.0f,1.0f);
	  glVertex3f(-1.0f,-1.0f, 1.0f);		// Bottom Right Of The Quad (Left)
	  glTexCoord2f(1.0f,1.0f);
	}

	if (type & RIGHT)
	{
	  // Right of cube
	  glColor3f(1.0f,0.0f,.0f);			//red
	  glVertex3f( 1.0f, 1.0f,-1.0f);	        // Top Right Of The Quad (Right)
	  glTexCoord2f(1.0f,0.0f);
	  glVertex3f( 1.0f, 1.0f, 1.0f);		// Top Left Of The Quad (Right)
	  glTexCoord2f(0.0f,0.0f);
	  glVertex3f( 1.0f,-1.0f, 1.0f);		// Bottom Left Of The Quad (Right)
	  glTexCoord2f(0.0f,1.0f);
	  glVertex3f( 1.0f,-1.0f,-1.0f);		// Bottom Right Of The Quad (Right)
	  glTexCoord2f(1.0f,1.0f);
	}
	glEnd();
	glPopMatrix();

}
/* The function called whenever a key is pressed. */
void keyPressed(unsigned char key, int x, int y)
{
    /* avoid thrashing this call */
    usleep(100);

    /* If escape is pressed, kill everything. */
    if (key == ESCAPE)
    {
      /* shut down our window */
      glutDestroyWindow(window);

      /* exit the program...normal termination. */
      exit(0);
    }
}

/* The function called whenever a normal key is pressed. */
void specialKeyPressed(int key, int x, int y)
{
	double rot;

    /* avoid thrashing this procedure */
    usleep(100);

    switch (key) {
    case GLUT_KEY_PAGE_UP: // tilt up
	lookupdown += 0.02f;
	break;

    case GLUT_KEY_PAGE_DOWN: // tilt down
	lookupdown -= 0.02f;
	break;

    case GLUT_KEY_UP: // walk forward (bob head)
	rot = yrot *3.14159 /180.0;
	xpos += (float)(cos(rot) + sin(rot)) * 0.05f;
	zpos += (float)(-sin(rot) +  cos(rot)) * 0.05f;
	break;

    case GLUT_KEY_DOWN: // walk back (bob head)
	rot = yrot *3.14159 /180.0;
	xpos -= (float)(cos(rot) + sin(rot)) * 0.05f;
	zpos -= (float)(-sin(rot) +  cos(rot)) * 0.05f;
	break;

    case GLUT_KEY_LEFT: // look left
	yrot += 1.5f;
	break;

    case GLUT_KEY_RIGHT: // look right
	yrot -= 1.5f;
	break;

    default:
	printf ("Special key %d pressed. No action there yet.\n", key);
	break;
    }
}

//Fun��o principal
int main(int argc, char **argv)
{
  /* Initialize GLUT state - glut will take any command line arguments that pertain to it or
     X Windows - look at its documentation at http://reality.sgi.com/mjk/spec3/spec3.html */
  glutInit(&argc, argv);

  /* Select type of Display mode:
     Double buffer
     RGBA color
     Alpha components supported
     Depth buffered for automatic clipping */
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);

  /* get a 640 x 480 window */
  glutInitWindowSize(640, 480);

  /* the window starts at the upper left corner of the screen */
  glutInitWindowPosition(0, 0);

  /* Open a window */
  window = glutCreateWindow("Jeff Molofee's GL Code Tutorial ... NeHe '99");

  /* Register the function to do all our OpenGL drawing. */
  glutDisplayFunc(&DrawGLScene);

  /* Go fullscreen.  This is as soon as possible. */
  glutFullScreen();

  /* Even if there are no events, redraw our gl scene. */
  glutIdleFunc(&DrawGLScene);

  /* Register the function called when our window is resized. */
  glutReshapeFunc(&ReSizeGLScene);

  /* Register the function called when the keyboard is pressed. */
  glutKeyboardFunc(&keyPressed);

  /* Register the function called when special keys (arrows, page down, etc) are pressed. */
  glutSpecialFunc(&specialKeyPressed);

  /* Initialize our window. */
  InitGL(640, 480);

  //funcoes do mouse
  glutMouseFunc(mouseButton);
  glutMotionFunc(mouseMove);

  /* Start Event Processing Engine */
  glutMainLoop();

  return 1;
}

#include <GL/glut.h>
#include <math.h>
#include <unistd.h>

#define ESCAPE 27
#define v .577350269

int window;

static int shoulder=0, elbow=0;
GLfloat dx, y, z;

// functions and structs -> cube
static GLfloat vdata[8][3] = {
   {-1.0, -1.0, -1.0}, {1.0, -1.0, -1.0}, {1.0, 1.0, -1.0}, {-1.0, 1.0, -1.0},
   {-1.0, -1.0, 1.0}, {1.0, -1.0, 1.0}, {1.0, 1.0, 1.0}, {-1.0, 1.0, 1.0}
};

static GLfloat ndata[8][3] = {
   {-v, -v, -v}, {v, -v, -v}, {v,v,-v}, {-v, v, -v},
   {-v, -v, v}, {v, -v, v}, {v,v,v}, {-v, v, v}
};


static GLuint vindices[6][4] = {
   {0,3,2,1}, {2,3,7,6}, {0,4,7,3},
   {1,2,6,5}, {4,5,6,7}, {0,1,5,4}
};

static void cube()
{
  int i;

   /* desenhar o cubo */
   for (i=0; i<6; i++) {
     glBegin(GL_POLYGON);
     glColor3f (1.0, 0.0, 0.0);
     glNormal3fv(&ndata[vindices[i][0]][0]);
     glVertex3fv(&vdata[vindices[i][0]][0]);
     glColor3f (0.0, 1.0, 0.0);
     glNormal3fv(&ndata[vindices[i][1]][0]);
     glVertex3fv(&vdata[vindices[i][1]][0]);
     glColor3f (0.0, 0.0, 1.0);
     glNormal3fv(&ndata[vindices[i][2]][0]);
     glVertex3fv(&vdata[vindices[i][2]][0]);
     glColor3f (0.0, 1.0, 1.0);
     glNormal3fv(&ndata[vindices[i][3]][0]);
     glVertex3fv(&vdata[vindices[i][3]][0]);
     glEnd();
   }
}

void start(){
    glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel (GL_SMOOTH);

	GLfloat especular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat shininess[] = { 50.0 };
	GLfloat lightPosition[] = { 0.0, 2.0, 1.0, 0.0 };

	glMaterialfv(GL_FRONT, GL_SPECULAR, especular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    // glEnable ( GL_COLOR_MATERIAL );
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);

	glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    glEnable ( GL_COLOR_MATERIAL );

	glColor3f(0., 0., 0.);
	dx = 0.0;
	y = 0.0;
	z = 0.0;
}

void robot(){
    glPushMatrix();
        glTranslatef (-0.5, 0.0, 0.0);
        glScalef(0.09, 0.28, 0.09);
        cube();
        // glRotatef ((GLfloat) shoulder, 0.0, 0.0, 1.0);

        // glTranslatef (-1.5, 0.0, 0.0);

        glPushMatrix();
            glScalef (0.5, 0.4, 0.4);
            glutSolidSphere (0.5, 150,150);
        glPopMatrix();

        glTranslatef (-0.2, 0.0, 0.0);
        glRotatef ((GLfloat) elbow, 0.0, 0.0, 1.0);
        glTranslatef (-0.5, 0.0, 0.0);

        glPushMatrix();
            glScalef (1.0, 0.4, 0.4);
            glutSolidSphere (0.5, 150,150);
        glPopMatrix();
    glPopMatrix();

}

void render(){
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glTranslatef (0.0, -2.0, -5.0);
	glRotatef(dx, 1.0, 0.0, 0.0);
	glRotatef(y , 0.0, 1.0, 0.0);
	glRotatef(z, 0.0, 0.0, 1.0);
    // start draw
        robot();
    // end draw
	glFlush();
	glutSwapBuffers();
}

void reshape (int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	if (w <= h)
		glOrtho (-2.5, 2.5, -2.5*(GLfloat)h/(GLfloat)w,2.5*(GLfloat)h/(GLfloat)w, -10.0, 10.0);
	else
		glOrtho (-2.5*(GLfloat)w/(GLfloat)h,2.5*(GLfloat)w/(GLfloat)h, -2.5, 2.5, -10.0, 10.0);
	glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
		case ESCAPE:
		glutDestroyWindow(window);
		exit(0);
		break;
	}
}


int main(int argc, char** argv)
{
	glutInit(&argc,argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(700,700);
	glutInitWindowPosition(0,0);
	window = glutCreateWindow("Mega robo nuclear metal geaaar");
	start();
	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
	return 0;
}

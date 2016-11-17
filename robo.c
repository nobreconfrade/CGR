#include <GL/glut.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>

#define ESCAPE 27
#define v .577350269

int window;

GLfloat spin_shoulder=0, spin_elbow=180;
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
	// glEnable(GL_LIGHTING);
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
        glTranslatef (-1.0, 0.5, 0.0);
    // glPushMatrix();
        glRotatef (spin_elbow, 0.0, 0.0, -1.0);
        glPushMatrix();
        glScalef (0.4, 0.6, 0.4);
            cube();
        // glRotatef (spin_elbow, 0.0, 0.0, -1.0);
        glPopMatrix();
        // glTranslatef (-1.5, 0.0, 0.0);
        // glTranslatef(0, 1.0, 0.0);
        glRotatef (spin_shoulder, 0.0, 0.0, -1.0);
        glTranslatef(0, -0.9, 0.0);
        glPushMatrix();
            // glColor3f(1,0,0);
            glScalef(0.25, 0.5, 0.09);
            cube();
            // glutSolidSphere (0.5, 150,150);
        glPopMatrix();

        // glTranslatef (-0.2, 0.0, 0.0);
        // // glRotatef ((GLfloat) elbow, 0.0, 0.0, 1.0);
        // glTranslatef (-0.5, 0.0, 0.0);
        //
        // glPushMatrix();
        //     glScalef (1.0, 0.4, 0.4);
        //     // glutSolidSphere (0.5, 150,150);
        // glPopMatrix();
    glPopMatrix();

}

void domino(){
    static int zrotation = 0;
    glPushMatrix();
        glTranslatef(0.5,-0.2,0);
        glRotatef(zrotation,0,0,-1.0);
        glTranslatef(0,2.0,0);
        glTranslatef(0,-0.8,0);
        glPushMatrix();
        glScalef (0.2, 0.6, 0.2);
        cube();
        glPopMatrix();
        // glTranslatef(0,1.5,0);
        if(spin_elbow + spin_shoulder == 250.0 && zrotation <= 90){
            zrotation +=5;
            // if(zrotation == 90)
                // zrotation = 0;
            glutPostRedisplay();
        }
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
        domino();
    // end draw
    // printf("SS %f -- SE %f \n",spin_shoulder,spin_elbow);
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

void keyboard(unsigned char key, int x, int y){
static int varia = 10;
switch (key) {
    case 'x':
        spin_elbow += 10.0;
        break;
    case 'z':
        if(spin_shoulder >= 30)
            varia = -10;
        if(spin_shoulder <= -30)
            varia = 10;
        spin_shoulder += varia;
        break;
    case 'r':
        spin_elbow = spin_shoulder = 0.0;
        break;
	case ESCAPE:
		glutDestroyWindow(window);
		exit(0);
		break;
	}
    glutPostRedisplay();
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

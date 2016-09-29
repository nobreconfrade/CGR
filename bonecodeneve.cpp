// g++ bonecodeneve.cpp -o bonecodeneve -lGL -lglut -lGLU

#include <stdio.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>

#define LARGURA 1000
#define ALTURA 1000

int janela;

// GLfloat spec[] = {1.0, 1.0, 1.0, 1.0}, shin[] = {50.0}, luz_pos[] = {2,2,2,1};


void Corpo(){
	glColor3f(0.7, 0.7, 0.7);
	glTranslatef(0.0, -1.0, 0.0);
	glutSolidSphere(1, 150, 150); // raio
}

void Cabeca(){
	glColor3f(0.7, 0.7, 0.7);
	glTranslatef(0.0, 1.25, 0.0);
 	glutSolidSphere(0.55,150,150);
}

void Olhos(){
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);

	glTranslatef(0.1, 0.2, 0.0);
	glutSolidSphere(0.08, 100, 100);

	glTranslatef(-0.2, 0.0, 0.0);
	glutSolidSphere(0.08,100,100);
	glPopMatrix();
}

void Nariz(){
	glColor3f(1.0, 0.4, 0.4);
	glRotatef(0.0 ,1.0, 0.0, 0.0);
	glutSolidCone(0.08, 0.5, 100, 2); // base, altura
}

void render(){
    glClear(GL_COLOR_BUFFER_BIT);
    Corpo();
    Cabeca();
    Olhos();
    Nariz();
    glFlush();

    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glLoadIdentity();
    //
    // // glRotatef(15,0,1,0);
    //
    // // glColor3f(1.0, 1.0, 1.0);
    // glTranslatef(0.0, -0.2, -1.0);
    // glutSolidSphere(1, 150, 150);
    //
    // // glTranslatef(0,0.3,0);
    // // glutSolidSphere(0.985f,200,200);
    //
    // glTranslatef(0.0, 1.15, 0.0);
    // 	glutSolidSphere(0.55,150,150);
    //
    // // glTranslatef(0,0.3,0);
    // // glutSolidSphere(0.960f,50,50);
    //
    // glPushMatrix();
    // glColor3f(0.0, 0.0, 0.0);
    //
    // glTranslatef(0.1, 0.2, 0.0);
    // glutSolidSphere(0.08, 100, 100);
    //
    // glTranslatef(-0.2, 0.0, 0.0);
    // glutSolidSphere(0.08,100,100);
    // glPopMatrix();
    //
    // glColor3f(1.0, 0.4, 0.4);
    // glRotatef(0.0 ,1.0, 0.0, 0.0);
    // glutSolidCone(0.08, 0.5, 100, 2);
    // // glTranslatef(0,-0.05,0.045);
    // // glutSolidCone(0.99,0.99,50,50);
    //
    // glutSwapBuffers();
    // // glFlush();
}


void InicGL(int w, int h){

    glClearColor (0.0, 0.0, 0.0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluOrtho2D(-2.5,2.5,-2.5,2.5);

    GLfloat especular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat shininess[] = { 50.0 };
    GLfloat lightPosition[] = { 0.0, -2.0, -2.0, 0.0 };
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel (GL_SMOOTH);

	glMaterialfv(GL_FRONT, GL_SPECULAR, especular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glColorMaterial ( GL_FRONT_AND_BACK, GL_EMISSION );

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
    glEnable (GL_COLOR_MATERIAL);

    // glClearColor (0.0, 0.0, 0.0, 0.0);
	// glColor3f(0.0, 0.0, 0.0);
	// glMatrixMode (GL_PROJECTION);
	// glLoadIdentity ();
	// gluOrtho2D(-2.5,2.5,-2.5,2.5);
    //
    // // glClearColor(0.0f,0.0f,0.0f,0.0f);
    // glClearDepth(1.0);
    // glDepthFunc(GL_LESS);
    // glEnable(GL_DEPTH_TEST);
    //
    // glEnable(GL_LIGHTING);
    // glEnable(GL_LIGHT0);
    // glEnable(GL_DEPTH_TEST);
    // glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
    // glMaterialfv(GL_FRONT, GL_SHININESS, shin);
    // glLightfv(GL_LIGHT0, GL_POSITION, luz_pos);
    //
    // glShadeModel(GL_SMOOTH);
    // glMatrixMode(GL_PROJECTION);
    // glLoadIdentity();
    // gluPerspective(45.0f,(GLfloat)w/h,0.1f,100.0f);
    // glMatrixMode(GL_MODELVIEW);
}

void fechar(unsigned char k, int x, int y){
    usleep(100);
    if(k == 27){
        glutDestroyWindow(janela);
        exit(0);
    }
}

int main(int argc, char** argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
    glutInitWindowSize(LARGURA,ALTURA);
    glutInitWindowPosition(0,0);
    janela = glutCreateWindow("Com certeza um boneco de neve");
    glutDisplayFunc(render);
    // glutFullScreen();
    // glutIdleFunc(render);
    glutKeyboardFunc(&fechar);
    InicGL(LARGURA,ALTURA);
    glutMainLoop();
    return 0;
}

// g++ bonecodeneve.cpp -o bonecodeneve -lGL -lglut -lGLU

#include <stdio.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>

#define LARGURA 1280
#define ALTURA 960

int janela;

GLfloat spec[] = {.7, .7, .7, .7}, shin[] = {90.0}, luz_pos[] = {2,2,2,1};

void render(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(0,-0.2,-1.95);
    glutSolidSphere(1,200,200);

    glTranslatef(0,0.3,0);
    glutSolidSphere(0.985f,200,200);

    glTranslatef(0,0.3,0);
    glutSolidSphere(0.960f,200,200);

    glTranslatef(0,-0.05,0.045);
    glutSolidCone(0.99,0.99,50,50);

    glutSwapBuffers();
}

void InicGL(int w, int h){
    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shin);
    glLightfv(GL_LIGHT0, GL_POSITION, luz_pos);

    glShadeModel(GL_SMOOTH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // gluPerspective(45.0f,(GLfloat)w/h,0.1f,100.0f);
    glMatrixMode(GL_MODELVIEW);
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
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
    glutInitWindowSize(LARGURA,ALTURA);
    glutInitWindowPosition(150,150);
    janela = glutCreateWindow("Com certeza um boneco de neve");
    glutDisplayFunc(render);
    // glutFullScreen();
    glutIdleFunc(render);
    glutKeyboardFunc(&fechar);
    InicGL(LARGURA,ALTURA);
    glutMainLoop();
    return 0;
}

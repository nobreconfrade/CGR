// g++ algumacoisa.cpp -o algumacoisa -lGL -lglut

#include <stdio.h>
#include <GL/glut.h>   
#include <GL/gl.h>	  
#include <GL/glu.h>	  
#include <math.h>
#include <unistd.h>

#define NUM_PARTICULAS 1000
#define LARGURA 1366
#define ALTURA 720

struct Particula{
	float x,y,velocidadeX,velocidadeY;
}particulas[NUM_PARTICULAS];

void render();
void InicGL(int Largura, int Altura);
void InicParticulas();

int main(int argc, char** argv){
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(LARGURA,ALTURA);
	glutInitWindowPosition(100,100);
	glutCreateWindow("Espero que seja chuva");

	glutDisplayFunc(render);
	glutIdleFunc(render);
	InicGL(LARGURA,ALTURA);	
	glutMainLoop();
}


void render(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin(GL_LINES);
	for(int i = 0; i<NUM_PARTICULAS; i++){
		float x_anterior = particulas[i].x;
		float y_anterior = particulas[i].y;
		particulas[i].x += particulas[i].velocidadeX;
		particulas[i].y += particulas[i].velocidadeY;
		glVertex2f(x_anterior,y_anterior);
		glVertex2f(particulas[i].x,particulas[i].y);
	}
	glEnd();
	// printf("--oi--\n\n");
	glutSwapBuffers();
	return;
}

void InicGL(int Largura, int Altura){
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);		
  glClearDepth(1.0);			
  glDepthFunc(GL_LESS);			
  glEnable(GL_DEPTH_TEST);			
  glShadeModel(GL_SMOOTH);		

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();		
  glMatrixMode(GL_MODELVIEW);

  InicParticulas();
}

void InicParticulas(){	
	for(int i=0;i<NUM_PARTICULAS;i++){
		float velocidade = (float)(rand() % 100)/5000 +0.01;
		int angulo = 80;
		particulas[i].velocidadeX = velocidade/10;
		particulas[i].velocidadeY = -velocidade;
		particulas[i].x = (rand() % 1000)/500.0f-1;
		particulas[i].y = 0.9;
		// particulas[i].x = 0.5f;
		// particulas[i].y = 0.5f;
	}
}
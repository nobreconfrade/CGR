// g++ algumacoisa.cpp -o algumacoisa -lGL -lglut


#include <GL/glut.h>   
#include <GL/gl.h>	  
#include <GL/glu.h>	  
#include <math.h>
#include <unistd.h>

#define NUM_PARTICULAS 1000

void render();

int main(int argc, char** argv){
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(1366,720);
	glutInitWindowPosition(100,100);
	glutCreateWindow("Espero que seja chuva");

	glutDisplayFunc(render);
	glutMainLoop();
}


void render(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin(GL_POINTS);
	for(int i = 0; i<NUM_PARTICULAS; i++){
		if(){
			
		}
	}
	glEnd();
	glutSwapBuffers();
	return;
}
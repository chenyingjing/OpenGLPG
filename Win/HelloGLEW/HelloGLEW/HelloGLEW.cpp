#include <iostream>
using namespace std;

#include "glew.h"
#include "freeglut.h"


void display()
{

}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB);
	glutInitWindowSize(512, 512);
	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	//glutCreateWindow(argv[0]);
	glutCreateWindow("Hello GLEW");

	if (glewInit())
	{
		cerr << "Unabe to initialize GLEW ... exitiing" << endl;
		exit(EXIT_FAILURE);
	}

	glutDisplayFunc(display);
	glutMainLoop();

	return 0;
}

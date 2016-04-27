#include "freeglut.h"

void display()
{

}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	//glutInitDisplayMode(GLUT_RGB);
	//glutInitWindowSize(512, 512);
	//glutInitContextVersion(4, 3);
	//glutInitContextProfile(GLUT_CORE_PROFILE);
	//glutCreateWindow(argv[0]);
	glutCreateWindow("Hello GLUT");

	glutDisplayFunc(display);
	glutMainLoop();

	return 0;
}

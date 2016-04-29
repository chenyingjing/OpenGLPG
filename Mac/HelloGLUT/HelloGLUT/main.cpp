
#include <GLUT/glut.h>

void display()
{
    
}

int main(int argc, const char * argv[]) {
    glutInit(&argc, (char **)argv);
    glutInitDisplayMode(GLUT_RGB);
    glutInitWindowSize(512, 512);
    //glutInitContextVersion(4, 3);
    //glutInitContextProfile(GLUT_CORE_PROFILE);
    //glutCreateWindow(argv[0]);
    glutCreateWindow("Hello GLUT");
    
    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
}

#include "helpers.h"

using namespace std;

void CreateCube(int x, int y) {
	glBegin(GL_POLYGON);
	glVertex3f(x, y, 0);
	glVertex3f(x + 1, y, 0);
	glVertex3f(x, y + 1, 0);
	glVertex3f(x + 1, y + 1, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x, y, 1);
	glVertex3f(x + 1, y, 1);
	glVertex3f(x, y + 1, 1);
	glVertex3f(x + 1, y + 1, 1);
	glEnd();
}
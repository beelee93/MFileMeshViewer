#include "Camera.h"

Camera::Camera(int viewWidth, int viewHeight) {
	eye = Vector3d(10, 10, 10);
	lookAt = Vector3d(0, 0, 0);
	up = Vector3d(0, 1, 0);
	ortho = 0;
	viewportWidth = viewWidth;
	viewportHeight = viewHeight;
	perspectiveFOV = 45.0;
}

void Camera::render() {

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (ortho)
		gluOrtho2D(0, viewportWidth, viewportHeight, 0);
	else 
		gluPerspective(perspectiveFOV, (double)viewportWidth / viewportHeight, 1, 100000);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye.x, eye.y, eye.z, lookAt.x, lookAt.y, lookAt.z, up.x, up.y, up.z);

}
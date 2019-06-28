#include "Camera.h"

Camera::Camera(int viewWidth, int viewHeight) {
	eye = Vector3d(10, 10, 10);
	lookAt = Vector3d(0, 0, 0);
	up = Vector3d(0, 1, 0);
	ortho = 0;
	viewportWidth = viewWidth;
	viewportHeight = viewHeight;
	zoom = 1;
	horizaspectlock = 16.0 / 9.0
	if ((viewportHeight / viewportWidth) < horizaspectlock) {
		zoom = zoom / (viewportHeight / viewportWidth) / horizaspectlock;
	}
	perspectiveFOV = 45.0;
}

void Camera::render() {

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (ortho) {
		Vector3d temp = eye - lookAt;
		double d = temp.length();
		double w2 = d * tan(perspectiveFOV / 360 * 3.141592) / zoom;
		double h2 = (double)viewportHeight / viewportWidth * w2;
		glOrtho(-w2, w2, -h2, h2, 1, 100000);
	}
	else 
		gluPerspective(atan(tan(perspectiveFOV * 3.141592 / 360) / zoom) * 360 / 3.141592, (double)viewportWidth / viewportHeight, 1, 100000);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye.x, eye.y, eye.z, lookAt.x, lookAt.y, lookAt.z, up.x, up.y, up.z);

}

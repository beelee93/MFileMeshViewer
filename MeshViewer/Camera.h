#ifndef _CAMERA_H
#define _CAMERA_H

#include "lib/glut.h"
#include "Vector3D.h"

class Camera {
public: 
	Camera(int viewWidth, int viewHeight);

	Vector3d eye;
	Vector3d lookAt;
	Vector3d up;

	int viewportWidth;
	int viewportHeight;

	double perspectiveFOV;

	int ortho;
	void render();
};

#endif
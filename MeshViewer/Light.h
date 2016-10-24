#ifndef _LIGHT_H
#define _LIGHT_H

#include <gl/glut.h>

class Light {
public:
	Light(GLenum id = GL_LIGHT0);
	int getID();

	int getEnabled();
	void setEnabled(int enabled);
	
	float* getAmbient();
	float* getDiffuse();
	float* getSpecular();
	float* getPosition();

	void setAmbient(float r, float g, float b);
	void setDiffuse(float r, float g, float b);
	void setSpecular(float r, float g, float b);
	void setPosition(float x, float y, float z, float w);

private:
	GLenum id;
	int enabled;
	float lightData[16] = { 0 };
};

#endif

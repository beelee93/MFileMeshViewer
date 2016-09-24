#include "Light.h"

Light::Light(GLenum id) {
	this->id = id;
	setEnabled(0);
	setAmbient(0.5f, 0.5f, 0.5f);
	setDiffuse(0.5f, 0.5f, 0.5f);
	setSpecular(1, 1, 1);
	setPosition(1, 1, 1, 0);
}

int Light::getID() {
	return id;
}

int Light::getEnabled() {
	return enabled;
}

void Light::setEnabled(int enabled) {
	this->enabled = enabled;
	if (enabled)
		glEnable(id);
	else
		glDisable(id);
}

float* Light::getAmbient() {
	return &lightData[0];
}

float* Light::getDiffuse() {
	return &lightData[4];
}

float* Light::getSpecular() {
	return &lightData[8];
}

float* Light::getPosition() {
	return &lightData[12];
}


void Light::setAmbient(float r, float g, float b) {
	lightData[0] = r;
	lightData[1] = g;
	lightData[2] = b;
	lightData[3] = 1.0f;

	glLightfv(id, GL_AMBIENT, &lightData[0]);
}

void Light::setDiffuse(float r, float g, float b) {
	lightData[4] = r;
	lightData[5] = g;
	lightData[6] = b;
	lightData[7] = 1.0f;

	glLightfv(id, GL_DIFFUSE, &lightData[4]);
}

void Light::setSpecular(float r, float g, float b) {
	lightData[8] = r;
	lightData[9] = g;
	lightData[10] = b;
	lightData[11] = 1.0f;

	glLightfv(id, GL_SPECULAR, &lightData[8]);
}

// Note: Subject to MODELVIEW transformation matrix
// Set w = 0 for directional light (direction points from x,y,z to origin)
void Light::setPosition(float x, float y, float z, float w) {
	lightData[12] = x;
	lightData[13] = y;
	lightData[14] = z;
	lightData[15] = w;

	glLightfv(id, GL_POSITION, &lightData[12]);
}
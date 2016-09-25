#ifndef _MATERIAL_H
#define _MATERIAL_H

#include "lib/glut.h"
class Material {
public:
	Material();
	float* getAmbient();
	float* getDiffuse();
	float* getSpecular();
	float getShininess();

	void setAmbient(float r, float g, float b);
	void setDiffuse(float r, float g, float b);
	void setSpecular(float r, float g, float b);
	void setShininess(float c);
	void applyMaterial();

private:
	float matData[13];
};

#endif
#include "Material.h"

Material::Material() {
	setAmbient(0.5f, 0.5f, 0.5f);
	setDiffuse(0.5f, 0.5f, 0.5f);
	setSpecular(1, 1, 1);
	setShininess(50.0f);
}

float* Material::getAmbient() {
	return &matData[0];
}

float* Material::getDiffuse() {
	return &matData[4];
}

float* Material::getSpecular() {
	return &matData[8];
}

float Material::getShininess() {
	return matData[12];
}

void Material::setAmbient(float r, float g, float b) {
	matData[0] = r;
	matData[1] = g;
	matData[2] = b;
	matData[3] = 1.0f;
}
void Material::setDiffuse(float r, float g, float b) {
	matData[4] = r;
	matData[5] = g;
	matData[6] = b;
	matData[7] = 1.0f;
}
void Material::setSpecular(float r, float g, float b) {
	matData[8] = r;
	matData[9] = g;
	matData[10] = b;
	matData[11] = 1.0f;
}
void Material::setShininess(float c) {
	matData[12] = c;
}

void Material::applyMaterial() {
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &matData[0]);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &matData[4]);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &matData[8]);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, matData[12]);
}
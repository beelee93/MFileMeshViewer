#include "Vector3d.h"

// =======================================================
// Constructors
// =======================================================


Vector3d::Vector3d() {
	x = y = z = 0;
}

Vector3d::Vector3d(float x, float y, float z, int id) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->id = id;
}

Vector3d::Vector3d(const Vector3d& v1) {
	x = v1.x;
	y = v1.y;
	z = v1.z;
	id = v1.id;
}

// =======================================================
// Functions
// =======================================================

float Vector3d::dot(Vector3d& v1){
	return v1.x*x + v1.y*y + v1.z*z;
}
float Vector3d::length() {
	return (float)sqrt(x*x + y*y + z*z);
}
float Vector3d::lengthSquared() {
	return (float)x*x + y*y + z*z;
}
void Vector3d::normalize() {
	float l = length();
	if (l > 0) {
		x /= l;
		y /= l;
		z /= l;
	}
}
Vector3d Vector3d::cross(Vector3d& v1) {
	return Vector3d(
		y*v1.z - v1.y*z,
		v1.x*z - x*v1.z,
		x*v1.y - v1.x*y
	);
}


// =======================================================
// Operator Overloads
// =======================================================


Vector3d operator+(Vector3d& v1, Vector3d &v2) {
	return Vector3d(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}
Vector3d operator-(Vector3d& v1, Vector3d &v2) {
	return Vector3d(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}
Vector3d operator*(Vector3d& v1, float scalar) {
	return Vector3d(v1.x * scalar, v1.y * scalar, v1.z * scalar);
}
Vector3d operator*(float scalar, Vector3d& v1) {
	return Vector3d(v1.x * scalar, v1.y * scalar, v1.z * scalar);
}


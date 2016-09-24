#ifndef _VECTOR3D_H
#define _VECTOR3D_H

#include <math.h>

class Vector3d {
public:
	float x, y, z;
	int id;

	Vector3d(float x, float y, float z, int id=0);
	Vector3d(const Vector3d& orig);
	Vector3d();
	
	// operator overloads
	friend Vector3d operator+(Vector3d& v1, Vector3d &v2);
	friend Vector3d operator-(Vector3d& v1, Vector3d &v2);
	friend Vector3d operator*(Vector3d& v1, float scalar);
	friend Vector3d operator*(float scalar, Vector3d& v1);

	// functions
	float dot(Vector3d& v1);
	float length();
	float lengthSquared();
	void normalize();
	Vector3d cross(Vector3d& v1);
	
};

#endif
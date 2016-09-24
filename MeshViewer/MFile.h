#ifndef _MFILE_H
#define _MFILE_H

#include <stdio.h>
#include <string.h>
#include "Vector3D.h"
#include <vector>

typedef struct {
	int indices[3];
} MFace;

// Class used to open and store raw information
// on an M file
class MFile {
public:
	MFile(const char* filename);
	~MFile();

	int isLoaded();
	int getVertexCount();
	int getFaceCount();

	MFace* getFace(int index);
	Vector3d* getVertex(int index);
	std::vector<Vector3d>** getVertexList();

private:
	int loaded;
	std::vector<MFace> *faces;
	std::vector<Vector3d> *vertices;
};

#endif
#ifndef _MESH_H
#define _MESH_H

#include <vector>
#include <map>
#include <set>
#include "LinkedList.h"
#include "HalfEdge.h"
#include "MFile.h"
#include "Material.h"

enum MeshDrawMode {
	NO_DRAW = 0x0,
	POINT = 0x1,
	SOLID = 0x2,
	WIREFRAME = 0x4,
	SOLID_AND_WIREFRAME = 0x6
};

typedef struct {
	int u;
	int v;
} MappingKey;

bool key_comp(MappingKey a, MappingKey b);

class Mesh {
public:
	Mesh(MFile* rawMeshData);
	~Mesh();

	int isLoaded();
	int getHalfEdgeCount();
	int getFaceCount();
	int getVertexCount();

	void computeNormals();

	ListNode<HEEdge>* getListHead();
	HEEdge* getHalfEdge(int index);
	HEFace* getFace(int index);
	HEVertex* getVertex(int index);

	struct {
		float minX, minY, minZ;
		float maxX, maxY, maxZ;
	} boundingBox;

	struct {
		Vector3d center;
		float radius;
	} boundingSphere;

	Material* getMaterial();

	void render(MeshDrawMode drawMode);
	static Mesh* loadFromFile(const char* filename);


private:
	LinkedList<HEEdge> *edges;
	int hecount; // number of used elements in the edges array

	std::vector<HEFace> *faces;
	std::vector<HEVertex> *vertices;
	std::vector<Vector3d> *rawVertices;

	int loaded;
	MFile* mfile;
	Material material;

	GLfloat *vertexBuffer, *normalBuffer;
	GLushort *indexBuffer, *indexEdgeBuffer;
	MeshDrawMode currentDrawMode;

	int indexCount, indexEdgeCount, vertexCount, normalCount;
	void generateHalfEdges();
	void generateBuffers();
};



#endif
#include "Mesh.h"
using namespace std;

typedef bool(*KEY_COMP_FN)(MappingKey, MappingKey);

bool key_comp(MappingKey a, MappingKey b) {
	if (a.u < b.u) return true;
	else if (a.u > b.u) return false;
	else
		return (a.v < b.v);
}

// generates half edge mesh data from loaded MFile
Mesh::Mesh(MFile* rawMeshData) {
	int i = 0, vcount, fcount;
	HEVertex tempVertex;
	HEFace tempFace;

	loaded = 0;
	if (!rawMeshData || !rawMeshData->isLoaded())
	{
		printf("Could not generate mesh from raw MFile data!\n");
		return;
	}

	this->mfile = rawMeshData;

	// move the reference over to this instance
	this->rawVertices = *(rawMeshData->getVertexList());

	// generate HEVertex data
	vcount = rawMeshData->getVertexCount();
	vertices = new vector<HEVertex>();

	tempVertex.edge = NULL;
	tempVertex.normal = Vector3d(0, 0, 0);

	for (i = 0; i < vcount; i++) {
		tempVertex.position = rawMeshData->getVertex(i);
		vertices->push_back(tempVertex);
	}

	// initialize HEFace data
	fcount = rawMeshData->getFaceCount();
	faces = new vector<HEFace>();
	tempFace.edge = NULL;
	tempFace.normal = Vector3d(0, 0, 0);
	for (i = 0; i < fcount; i++) {
		faces->push_back(tempFace);
	}

	// initialize the HEEdge list
	edges = new LinkedList<HEEdge>();

	generateHalfEdges();

	printf("Generated Half Edge Data!\n");
	printf("Vertices\t:%d\n", getVertexCount());
	printf("Faces\t\t:%d\n", getFaceCount());
	printf("Edges\t\t:%d\n", getHalfEdgeCount() / 2);
	printf("Euler characteristic: %d\n", getVertexCount() - getHalfEdgeCount() / 2 + getFaceCount());

	// so prevent disposing the raw vertex data
	rawMeshData->getVertexList()[0] = NULL;

	// compute normals
	computeNormals();
	printf("Normals computed.\n");
}

void Mesh::generateHalfEdges() {
	// stores reference to already created Half edges
	typedef map<MappingKey, HEEdge*, KEY_COMP_FN> Map;
	Map hemap(key_comp);

	Map::iterator it;

	MappingKey curKey, prevKey;

	HEEdge *curEdge, *prevEdge, tempEdge;

	HEFace* curFace;
	MFace* mface;
	int i, j, u, v;

	vector<HEEdge>::iterator itEdge;

	// iterate for every face
	for (i = 0; i < mfile->getFaceCount(); i++) {
		curFace = &(faces->at(i));
		mface = mfile->getFace(i);

		// iterate for every edge on this face
		for (j = 0; j < 3; j++) {
			u = j;
			v = (j + 1) % 3;

			curKey.u = mface->indices[u];  // origin vertex index
			curKey.v = mface->indices[v];  // dest vertex index

			// create a new half edge for this 
			curEdge = &(edges->put(HEEdge())->item);
			curEdge->face = curFace;
			curEdge->origin = &(vertices->at(curKey.u));
			curEdge->next = curEdge->prev = NULL;
			curEdge->pair = NULL;

			// a face needs to have an edge for it
			if (!curFace->edge)
				curFace->edge = curEdge;

			// if the vertex has no edge tied to it, then give
			// it an edge
			if (!curEdge->origin->edge)
				curEdge->origin->edge = curEdge;

			// put this half edge into the mapping
			hemap.insert(pair<MappingKey, HEEdge*>(MappingKey(curKey), curEdge));

			// check if its pair exists
			curKey.u = mface->indices[v];
			curKey.v = mface->indices[u];

			if ((it = hemap.find(curKey)) != hemap.end()) {
				// set them to pair up
				prevEdge = it->second;

				prevEdge->pair = curEdge;
				curEdge->pair = prevEdge;
			}
		}

		// link them in clockwise manner
		for (j = 0; j < 3; j++) {
			u = (j + 1) % 3;
			v = (j == 0 ? 2 : j - 1);

			curKey.u = mface->indices[j];
			curKey.v = mface->indices[u];

			prevKey.u = mface->indices[v];
			prevKey.v = mface->indices[j];

			it = hemap.find(curKey);
			curEdge = it->second;

			it = hemap.find(prevKey);
			prevEdge = it->second;

			curEdge->prev = prevEdge;
			prevEdge->next = curEdge;
		}
	}
	
	// upon reaching here, all faces have been stitched together
	// now to create boundary half edges
	map<int, HEEdge*> bhemap; // mapping of VertexIndex -> HE that leads to it
    
	// go thru all half edges and determine those without a pair
	it = hemap.begin();
	while (it != hemap.end()) {
		curEdge = it->second;

		if (!curEdge->pair) {
			// has no pair, so create one
			prevEdge = &(edges->put(HEEdge())->item);
			prevEdge->face = NULL;
			prevEdge->next = NULL;
			prevEdge->prev = NULL;

			// pairing
			prevEdge->pair = curEdge;
			curEdge->pair = prevEdge;

			prevEdge->origin = curEdge->next->origin;
			bhemap.insert(pair<int, HEEdge*>(curEdge->origin->position->id, prevEdge));
		}
			
		it++;
	}

	// now link up all the boundary half edges
	map<int, HEEdge*>::iterator it2 = bhemap.begin(), it3;
	while (it2 != bhemap.end()) {
		curEdge = it2->second;
		
		it3 = bhemap.find(curEdge->origin->position->id);
		prevEdge = it3->second;

		prevEdge->next = curEdge;
		curEdge->prev = prevEdge;

		it2++;
	}

	// compute bounding box/sphere
	HEVertex* vertTemp = &(vertices->at(0));
	boundingBox.maxX = boundingBox.minX = vertTemp->position->x;
	boundingBox.maxY = boundingBox.minY = vertTemp->position->y;
	boundingBox.maxZ = boundingBox.minZ = vertTemp->position->z;

	for (int k = 1; k < vertices->size(); k++) {
		vertTemp = &(vertices->at(k));
		if (vertTemp->position->x < boundingBox.minX)
			boundingBox.minX = vertTemp->position->x;
		if (vertTemp->position->y < boundingBox.minY)
			boundingBox.minY = vertTemp->position->y;
		if (vertTemp->position->z < boundingBox.minZ)
			boundingBox.minZ = vertTemp->position->z;

		if (vertTemp->position->x > boundingBox.maxX)
			boundingBox.maxX = vertTemp->position->x;
		if (vertTemp->position->y > boundingBox.maxY)
			boundingBox.maxY = vertTemp->position->y;
		if (vertTemp->position->z > boundingBox.maxZ)
			boundingBox.maxZ = vertTemp->position->z;
	}

	Vector3d& bs = boundingSphere.center;
	bs.x = (boundingBox.maxX + boundingBox.minX) / 2;
	bs.y = (boundingBox.maxY + boundingBox.minY) / 2;
	bs.z = (boundingBox.maxZ + boundingBox.minZ) / 2;

	Vector3d v1(bs.x, bs.y, bs.z);
	Vector3d v2(boundingBox.maxX, boundingBox.maxY, boundingBox.maxZ);

	boundingSphere.radius = (v1 - v2).length();
}

void Mesh::computeNormals() {
	int i, count;
	HEFace* tempFace;
	HEEdge *tempEdge, *startEdge;
	Vector3d va, vb, vc;
	HEVertex* tempVerts[3];

	// first, compute the face normals
	for (i = 0; i < faces->size(); i++) {
		tempFace = &(faces->at(i));
		
		tempEdge = tempFace->edge;
		tempVerts[0] = tempEdge->origin;
		tempEdge = tempEdge->next;
		tempVerts[1] = tempEdge->origin;
		tempEdge = tempEdge->next;
		tempVerts[2] = tempEdge->origin;
		tempEdge = tempEdge->next;

		// parallel vectors to the face
		va = *(tempVerts[1]->position) - *(tempVerts[0]->position);
		vb = *(tempVerts[2]->position) - *(tempVerts[0]->position);

		// compute normal for this face
		vc = va.cross(vb);
		vc.normalize();

		tempFace->normal = vc;
	}

	// now take each vertex and average the normals 
	// of all its adjacent faces
	for (i = 0; i < vertices->size(); i++) {
		tempVerts[0] = &(vertices->at(i));
		startEdge = tempEdge = tempVerts[0]->edge;

		// init accumulator
		va.x = va.y = va.z = 0;
		count = 0;

		do {
			if (tempEdge->face) { // if this is null, means boundary
				va = va + tempEdge->face->normal;
				count++;
			}

			tempEdge = tempEdge->pair->next;
		} while (tempEdge != startEdge);

		// take the average
		va = va * (1.0f / count);
		va.normalize();

		// set this as the vertex normal
		tempVerts[0]->normal = va;
	}

	// normals computed
}

Mesh::~Mesh() {
	delete edges;
	delete faces;
	delete vertices;
	delete rawVertices;

	edges = NULL;
	faces = NULL;
	vertices = NULL;
	rawVertices = NULL;
}

int Mesh::getHalfEdgeCount() {
	return edges->getSize();
}

int Mesh::getVertexCount() {
	return vertices->size();
}

int Mesh::getFaceCount() {
	return faces->size();
}

HEEdge* Mesh::getHalfEdge(int i) {
	return edges->get(i);
}

HEFace* Mesh::getFace(int i) {
	return &(faces->at(i));
}

HEVertex* Mesh::getVertex(int i) {
	return &(vertices->at(i));
}

int Mesh::isLoaded() {
	return loaded;
}

ListNode<HEEdge>* Mesh::getListHead() {
	return edges->getHead();
}


#include "Mesh.h"
#include <time.h>
using namespace std;

typedef bool(*KEY_COMP_FN)(MappingKey, MappingKey);

bool key_comp(MappingKey a, MappingKey b) {
	if (a.u < b.u) return true;
	else if (a.u > b.u) return false;
	else
		return (a.v < b.v);
}

// file-wide variable
static Mesh* lastRenderedMesh = NULL;

// generates half edge mesh data from loaded MFile
Mesh::Mesh(MFile* rawMeshData) {
	int i = 0, vcount, fcount;
	HEVertex tempVertex;
	HEFace tempFace;

	vertexBuffer = normalBuffer = NULL;
	indexBuffer = indexEdgeBuffer = NULL;
	currentDrawMode = MD_NO_DRAW;

	loaded = 0;
	if (!rawMeshData || !rawMeshData->isLoaded())
	{
		printf("Mesh: Could not generate mesh from raw MFile data!\n");
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

	printf("Mesh: Generated Half Edge Data!\n");
	printf("Mesh: Vertices\t:%d\n", getVertexCount());
	printf("Mesh: Faces\t:%d\n", getFaceCount());
	printf("Mesh: Edges\t:%d\n", getHalfEdgeCount() / 2);
	printf("Mesh: Euler characteristic: %d\n", getVertexCount() - getHalfEdgeCount() / 2 + getFaceCount());

	// so prevent disposing the raw vertex data
	rawMeshData->getVertexList()[0] = NULL;

	// compute normals
	computeNormals();
	printf("Mesh: Normals computed.\n");

	generateBuffers();

	loaded = 1;
}

void Mesh::generateHalfEdges() {
	clock_t cprev = clock();
	// stores reference to already created Half edges
	typedef map<MappingKey, HEEdge*, KEY_COMP_FN> Map;
	Map hemap(key_comp);

	Map::iterator it, it2;

	MappingKey curKey, prevKey;

	HEEdge *curEdge, *prevEdge, *pairEdge, *firstEdge;

	HEFace* curFace;
	MFace* mface;
	int i, j, u, v, *indices;
	int fcount = mfile->getFaceCount();

	vector<HEEdge>::iterator itEdge;

	// iterate every face 
	for (i = fcount; i--;) {
		curFace = &(faces->at(i)); // reference to the current face
		indices = mfile->getFace(i)->indices;

		// iterate for every edge
		for (j = 3; j--;) {
			u = (2-j);
			v = (u + 1) % 3;

			// is there already such an edge?
			curKey.u = indices[u];  // origin vertex index
			curKey.v = indices[v];  // dest vertex index
			it = hemap.find(curKey);

			if (it != hemap.end()) {
				// there is such a half edge
				curEdge = it->second;
			}
			else {
				// no such half edge, so create the pair
				curEdge = &(edges->put(HEEdge())->item);
				pairEdge = &(edges->put(HEEdge())->item);

				curEdge->face = curFace;
				curEdge->origin = &(vertices->at(curKey.u));
				curEdge->next = curEdge->prev = NULL;
				curEdge->pair = pairEdge; // pair

				pairEdge->face = NULL;
				pairEdge->origin = &(vertices->at(curKey.v));
				pairEdge->next = pairEdge->prev = NULL;
				pairEdge->pair = curEdge; // pair

			    // put the pair half edge into the mapping
				prevKey.u = curKey.v;
				prevKey.v = curKey.u;
				hemap.insert(pair<MappingKey, HEEdge*>(MappingKey(prevKey), pairEdge));
			}

			// keep the reference to the first half edge for last link later
			if(j==2) firstEdge = curEdge;

			// a face needs to have an edge for it
			if (!curFace->edge)
				curFace->edge = curEdge;

			// if the vertex has no edge tied to it, then give
			// it an edge
			if (!curEdge->origin->edge)
				curEdge->origin->edge = curEdge;

			// we can begin linking on 2nd iteration
			if (j < 2) {
				prevEdge->next = curEdge;
				curEdge->prev = prevEdge;
			}

			// keep reference for this half edge for next iteration
			prevEdge = curEdge;
		}

		// one last link
		curEdge->next = firstEdge;
		firstEdge->prev = curEdge;
	}

	// upon reaching here, all faces have been stitched together
	// now to create boundary half edges
	
	it2 = hemap.begin();
	while (it2 != hemap.end()) {
		// does this half edge have its prev set?
		curEdge = it2->second;
		if (curEdge->prev) {
			it2++;
			continue;
		}

		// prev is not set, so, let's find it's prev
		prevEdge = curEdge->pair;
		while (prevEdge->next) {
			// this edge is in a link (hence, it's a valid face)
			prevEdge = prevEdge->next->pair;
		}

		// reaching here, prevEdge has no next
		prevEdge->next = curEdge;
		curEdge->prev = prevEdge;

		it2++;
	}
	
	// upon reaching here, all boundary edges have been stitched together
	clock_t b = clock() - cprev;
	printf("Time taken: %.2f s\n", (double)b / CLOCKS_PER_SEC);


	// compute bounding box/sphere
	HEVertex* vertTemp = &(vertices->at(0));
	boundingBox.maxX = boundingBox.minX = vertTemp->position->x;
	boundingBox.maxY = boundingBox.minY = vertTemp->position->y;
	boundingBox.maxZ = boundingBox.minZ = vertTemp->position->z;

	for (int k = 1; k < (int)vertices->size(); k++) {
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
	for (i = (int)faces->size(); i--;) {
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
	for (i = (int)vertices->size(); i--; ) {
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

void Mesh::generateBuffers() {
	int i;
	Vector3d *tempVertex;
	ListNode<HEEdge>* node;
	HEFace* curFace;
	HEEdge* curEdge;

	if (!vertexBuffer) {
		printf("Mesh: Building the vertex buffer...\n");

		vertexCount = this->getVertexCount();
		vertexBuffer = new GLfloat[vertexCount * 3];

		// transfer vertex data
		for ( i = 0; i < vertexCount; i++) {
			tempVertex = this->getVertex(i)->position;
			vertexBuffer[i * 3] = tempVertex->x;
			vertexBuffer[i * 3 + 1] = tempVertex->y;
			vertexBuffer[i * 3 + 2] = tempVertex->z;
		}

	}

	
	if (!indexEdgeBuffer) {
		printf("Mesh: Building the edge index buffer...\n");

		indexEdgeCount = this->getHalfEdgeCount();
		indexEdgeBuffer = new GLushort[indexEdgeCount];
		node = getListHead();

		for (i = 0; i < (indexEdgeCount>>1) && node; i++) {
			// take the first half edge of each twin pair
			indexEdgeBuffer[i * 2] = node->item.origin->position->id;
			indexEdgeBuffer[i * 2 + 1] = node->item.pair->origin->position->id;

			node = node->next;
			if (node) node = node->next;
		}
	}
	

	if (!indexBuffer) {
		printf("Mesh: Building the face index buffer...\n");

		indexCount = this->getFaceCount() * 3; // Each face has 3 indices
		indexBuffer = new GLushort[indexCount];

		for (i = getFaceCount(); i--;) {
			curFace = getFace(i);
			curEdge = curFace->edge;

			// Set the indices
			indexBuffer[i * 3] = curEdge->origin->position->id;
			curEdge = curEdge->next;

			indexBuffer[i * 3 + 1] = curEdge->origin->position->id;
			curEdge = curEdge->next;

			indexBuffer[i * 3 + 2] = curEdge->origin->position->id;
		}
	}

	if (!normalBuffer) {
		printf("Mesh: Building the normals buffer...\n");

		normalCount = getVertexCount() * 3;
		normalBuffer = new GLfloat[normalCount];

		// each vertex will have its normal information
		for (i = getVertexCount(); i--;) {
			tempVertex = &(getVertex(i)->normal);
			normalBuffer[i * 3] = tempVertex->x;
			normalBuffer[i * 3 + 1] = tempVertex->y;
			normalBuffer[i * 3 + 2] = tempVertex->z;
		}
	}
}

// Renders this mesh based on drawMode. Internal variable 
// keeps track of previously rendered mesh so rendering the 
// same mesh need not invoke change of buffer pointer.
void Mesh::render(MeshDrawMode drawMode) {
	if (drawMode == MD_NO_DRAW)
		return;

	glEnableClientState(GL_VERTEX_ARRAY);

	if (lastRenderedMesh != this) {
		// different mesh, so reapply buffers
		
		glVertexPointer(3, GL_FLOAT, 0, vertexBuffer);

		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, normalBuffer);
		glDisableClientState(GL_NORMAL_ARRAY);
	}

	// rendering
	if (drawMode & 0x1) {
		// Point cloud rendering
		glDisable(GL_LIGHTING);
		glDrawArrays(GL_POINTS, 0, vertexCount);
	}
	else {
		if (drawMode & MeshDrawMode::MD_SOLID) {
			this->getMaterial()->applyMaterial();

			glEnable(GL_LIGHTING);
			glEnableClientState(GL_NORMAL_ARRAY);

			// draw the faces. enable polygon offset of a frame is drawn over it
			if (drawMode & MeshDrawMode::MD_WIREFRAME) {
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(1.0f, 1.0f);
				glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, indexBuffer);
				glDisable(GL_POLYGON_OFFSET_FILL);
			}
			else
				glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, indexBuffer);

			glDisableClientState(GL_NORMAL_ARRAY);
			glDisable(GL_LIGHTING);
		}

		if (drawMode & MeshDrawMode::MD_WIREFRAME) {
			// draw the frame
			glDisable(GL_LIGHTING);
			glColor3f(0, 0.85f, 0);
			glLineWidth(0.4f);
			glDrawElements(GL_LINES, indexEdgeCount, GL_UNSIGNED_SHORT, indexEdgeBuffer);
			glLineWidth(1.0f);
		}
	}
	glDisableClientState(GL_VERTEX_ARRAY);

	lastRenderedMesh = this;
}

Mesh::~Mesh() {

	delete[] vertexBuffer;
	delete[] normalBuffer;
	delete[] indexBuffer;
	delete[] indexEdgeBuffer;

	vertexBuffer = normalBuffer = NULL;
	indexBuffer = indexEdgeBuffer = NULL;

	delete faces;
	delete vertices;
	delete edges;
	delete rawVertices;

	edges = NULL;
	faces = NULL;
	vertices = NULL;
	rawVertices = NULL;

}

// loads a mesh from file. returns the pointer to the 
// loaded mesh. If failed, a nullptr is returned.
// be sure to delete the returned pointer
Mesh* Mesh::loadFromFile(const char* filename) {
	printf("Mesh: Loading from file %s\n", filename);

	MFile* file = new MFile(filename);
	Mesh* mesh = NULL;

	if (file->isLoaded()) {
		printf("Mesh: File loaded. Building half edge structure...\n");
		mesh = new Mesh(file);
	}

	delete file;

	return mesh;
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

Material* Mesh::getMaterial() {
	return &material;
}
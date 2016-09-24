#include "main.h"

// STATIC VARIABLES
static clock_t prevClock;
static Camera* camera;
static int viewport[2] = { 800, 800 };

static Mesh *mesh = NULL;

static int meshVisible = 1;

// settings
static DRAWMODE drawMode = DRAWMODE::SOLID;
static SHADEMODE shadeMode = SHADEMODE::FLAT;
static GLfloat drawColor[3] = { 1, 1, 1 };


// lighting
static int lightingEnabled = 1;
static Light lights[LIGHTS_AVAILABLE];

// buffers
static GLfloat* vertexBuffer = NULL;
static GLfloat* normalBuffer = NULL;
static GLushort* indexBuffer = NULL;
static GLushort* indexEdgeBuffer = NULL;
static GLint vertexCount = 0,
			indexCount = 0,
			normalCount = 0,
			indexEdgeCount = 0;

// ======================================================
// Main Entry Point
// ======================================================
int main(int argc, char** argv) {
	atexit(cleanup);
	
	// normal initialisation
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
	glutInitWindowSize(viewport[0], viewport[1]);
	glutInitWindowPosition(100, 100);

	glutCreateWindow("GLUT Demo");

	// set the clearcolor and the callback
	glClearColor(0.0, 0.0, 0.0, 0.0);

	// register your drawing function
	glutDisplayFunc(onRender);
	glutKeyboardUpFunc(onKeyUp);
	glutReshapeFunc(onWindowResize);

	// set up camera
	camera = new Camera(viewport[0], viewport[1]);

	// set up lights
	for (int i = 0; i < LIGHTS_AVAILABLE; i++)
		lights[i] = Light(GL_LIGHT0 + i);

	lights[0].setEnabled(1);

	loadMesh("C:/Users/user/Downloads/TestModels/TestModels/gargoyle.m");


	prevClock = clock();

	// enter the main loop
	glutMainLoop();
	
	return 0;
}

// ======================================================
// Main rendering function
// ======================================================
void onRender(void) {
	glClearColor(0.392f, 0.584f, 0.929f, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// time keeping for animation
	clock_t nowClock = clock();
	double elapsed = (double)(nowClock - prevClock) / CLOCKS_PER_SEC;
	prevClock = nowClock;

	update(elapsed);
	draw(elapsed);


	glFlush();
	glutPostRedisplay();
}

// ======================================================
// Called when window is resized
// ======================================================
void onWindowResize(int width, int height) {
	viewport[0] = width;
	viewport[1] = height;

	camera->viewportWidth = width;
	camera->viewportHeight = height;

	glViewport(0, 0, width, height);
}

// ======================================================
// Called when a key is released
// ======================================================
void onKeyUp(unsigned char key, int x, int y) {
	if (key == 'R') {
		int k = (int)drawMode;
		k = (k + 1) % 4;
		setDrawMode((DRAWMODE)k);
	}
}

double angle = 0;

// ======================================================
// Main Update function (input processing, etc)
// ======================================================
void update(double elapsed) {
	float r = mesh->boundingSphere.radius * 5 / 3;
	
	camera->eye = Vector3d(sin(angle) * r, r, cos(angle) * r) + mesh->boundingSphere.center;
	camera->lookAt = mesh->boundingSphere.center;
	angle += elapsed;
}

// ======================================================
// Drawing calls
// ======================================================
void draw(double elapsed) {
	glLoadIdentity();
	camera->render();
	
	glEnable(GL_DEPTH_TEST);
	drawMesh(elapsed);

	drawAxes(3);
}

// ======================================================
// Helper function to draw axes
// ======================================================
void drawAxes(double length) {
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);  // x
	glVertex3f(0, 0, 0);
	glVertex3f(length, 0, 0);

	glColor3f(0, 1, 0);  // y
	glVertex3f(0, 0, 0);
	glVertex3f(0, length, 0);

	glColor3f(0, 0, 1); // z
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, length);
	glEnd();
}

static float material[13] = {
	0.5f,0.5f,0.5f,1.0f,
	0.5f,0.5f,0.5f,1.0f,
	1.0f,1.0f,1.0f,1.0f,
	60.0f
};

// ======================================================
// Call this before any rendering that requires lighting
// ======================================================
void applyLights() {
	if (lightingEnabled) 
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);
}

// ======================================================
// Draws the loaded mesh object
// elapsed - elapsed time in seconds between draw calls
// ======================================================
void drawMesh(double elapsed) {
	if (!meshVisible || mesh == NULL)
		return;  // don't do anything if no mesh is loaded

	glEnableClientState(GL_VERTEX_ARRAY);

	glColor3fv(drawColor);
	switch (drawMode) {
	case POINT:
		glDrawArrays(GL_POINTS, 0, vertexCount);
		break;

	case WIREFRAME:
		glDrawElements(GL_LINES, indexEdgeCount, GL_UNSIGNED_SHORT, indexEdgeBuffer);
		break;

	case SOLID:
		// set the shade model
		glShadeModel(shadeMode == SMOOTH ? GL_SMOOTH : GL_FLAT);

		glEnable(GL_COLOR_MATERIAL);
		glColor3f(0.5f, 0.5f, 0.5f);

	
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &material[8]);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material[12]);

		// apply lighting
		applyLights();

		glEnableClientState(GL_NORMAL_ARRAY);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, indexBuffer);
		glDisableClientState(GL_NORMAL_ARRAY);

		glDisable(GL_LIGHTING);
		break;

	case SOLID_AND_WIREFRAME:
		// set the shade model
		glShadeModel(shadeMode == SMOOTH ? GL_SMOOTH : GL_FLAT);

		glEnable(GL_COLOR_MATERIAL);
		glColor3f(0.5f, 0.5f, 0.5f);

		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &material[8]);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material[12]);

		// apply lighting
		applyLights();

		// to make sure wireframe lines appear "above" solid
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, 1.0f);

		glEnableClientState(GL_NORMAL_ARRAY);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, indexBuffer);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisable(GL_LIGHTING);
		glDisable(GL_POLYGON_OFFSET_FILL);

		// now draw the frame
		glColor3f(0, 0.85f, 0);
		glLineWidth(0.4f);
		glDrawElements(GL_LINES, indexEdgeCount, GL_UNSIGNED_SHORT, indexEdgeBuffer);
		glLineWidth(1.0f);
		break;
	}

	glDisableClientState(GL_VERTEX_ARRAY);
}

// ======================================================
// Loads a new mesh from the given path to an mfile
// filename - path to MFile
// ======================================================
void loadMesh(const char* filename) {
	// unload previous mesh
	unloadMesh();

	// mesh should be null
	_ASSERT(mesh == NULL);

	MFile* file = new MFile(filename);
	if (file->isLoaded())
	{
		mesh = new Mesh(file);
		printf("Mesh sucessfully loaded\n");
	}
	else
	{
		mesh = NULL;
		printf("Count not load mesh\n");
	}

	delete file;

	setDrawMode(SOLID_AND_WIREFRAME);
}

// ======================================================
// Removes the loaded mesh from memory
// ======================================================
void unloadMesh() {
	if (mesh != NULL) {
		delete mesh;
		mesh = NULL;
		printf("Disposed mesh\n");

		clearBuffers();
	}
}

// ======================================================
// Changes the draw mode. Calling this also resets the 
// buffers (clearing them if need to)
// ======================================================
void setDrawMode(DRAWMODE newDrawMode) {
	int i;
	Vector3d *tempVertex;
	ListNode<HEEdge>* node;
	std::set<HEEdge*> trackEdge;
	HEFace* curFace;
	HEEdge* curEdge;

	drawMode = newDrawMode;

	if (mesh == NULL)
		return;

	// create the vertex buffer, if not already
	if (!vertexBuffer) {
		printf("Building the vertex buffer...\n");

		vertexCount = mesh->getVertexCount();
		vertexBuffer = new GLfloat[vertexCount * 3];

		// transfer vertex data
		for (i = 0; i < vertexCount; i++) {
			tempVertex = mesh->getVertex(i)->position;
			vertexBuffer[i * 3] = tempVertex->x;
			vertexBuffer[i * 3 + 1] = tempVertex->y;
			vertexBuffer[i * 3 + 2] = tempVertex->z;
		}

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, vertexBuffer);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	// create the edge indices if not done so already
	if (drawMode == WIREFRAME || drawMode == SOLID_AND_WIREFRAME) {
		if (!indexEdgeBuffer) {
			printf("Building the edge index buffer...\n");

			indexEdgeCount = mesh->getHalfEdgeCount();
			indexEdgeBuffer = new GLushort[indexEdgeCount];
			node = mesh->getListHead();
			for (i = 0; i < indexEdgeCount && node; node = node->next) {
				// take the first half edge of each twin pair

				// has its pair been allocated?
				if (trackEdge.find(node->item.pair) != trackEdge.end())
					continue;

				// allocate this edge
				indexEdgeBuffer[i * 2] = node->item.origin->position->id;
				indexEdgeBuffer[i * 2 + 1] = node->item.pair->origin->position->id;

				// add this into the set
				trackEdge.insert(&(node->item));
				i++;
			}
		}
	}

	// create the face indices/normal buffers if not done so already
	if (drawMode == SOLID || drawMode == SOLID_AND_WIREFRAME) {
		if (!indexBuffer) {
			printf("Building the face index buffer...\n");

			indexCount = mesh->getFaceCount() * 3; // Each face has 3 indices
			indexBuffer = new GLushort[indexCount];

			for (i = 0; i < mesh->getFaceCount(); i++) {
				curFace = mesh->getFace(i);
				curEdge = curFace->edge;

				// Set the indices
				indexBuffer[i * 3] = curEdge->origin->position->id;
				curEdge = curEdge->next;

				indexBuffer[i * 3 + 1] = curEdge->origin->position->id;
				curEdge = curEdge->next;

				indexBuffer[i * 3 + 2] = curEdge->origin->position->id;
			}
		}

		if(!normalBuffer) {
			printf("Building the normals buffer...\n");

			normalCount = mesh->getVertexCount() * 3;
			normalBuffer = new GLfloat[normalCount];

			// each vertex will have its normal information
			for (i = 0; i < mesh->getVertexCount(); i++) {
				tempVertex = &(mesh->getVertex(i)->normal);
				normalBuffer[i * 3] = tempVertex->x;
				normalBuffer[i * 3 + 1] = tempVertex->y;
				normalBuffer[i * 3 + 2] = tempVertex->z;
			}

			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT, 0, normalBuffer);
			glDisableClientState(GL_NORMAL_ARRAY);
		}
	}
	
}

// ======================================================
// Clears all buffers
// ======================================================
void clearBuffers() {
	if (vertexBuffer != NULL) {
		delete[] vertexBuffer;
		vertexBuffer = NULL;
	}

	if (indexBuffer != NULL) {
		delete[] indexBuffer;
		indexBuffer = NULL;
	}

	if (normalBuffer != NULL) {
		delete[] normalBuffer;
		normalBuffer = NULL;
	}
	vertexCount = 0;
	normalCount = 0;
	indexCount = 0;
}

// ======================================================
// Called when application terminates. All buffers and mesh
// should be removed
// ======================================================
void cleanup() {
	delete camera;
	unloadMesh();
	clearBuffers();
}


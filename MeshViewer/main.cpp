#include "main.h"

// STATIC VARIABLES
static clock_t prevClock;
static Camera* camera;
static int viewport[2] = { 800, 800 };

static Mesh *mesh = NULL;

static int meshVisible = 1;

static DRAWMODE drawMode = DRAWMODE::SMOOTH;

static GLfloat drawColor[3] = { 1, 1, 1 };
static GLfloat* vertexBuffer = NULL;
static GLfloat* normalBuffer = NULL;
static GLushort* indexBuffer = NULL;
static GLint vertexCount = 0, indexCount = 0, normalCount = 0;

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
	glutReshapeFunc(onWindowResize);

	// set up camera
	camera = new Camera(viewport[0], viewport[1]);

	// enter the main loop
	loadMesh("C:/Users/user/Downloads/TestModels/TestModels/gargoyle.m");
	
	prevClock = clock();
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
	1.0f,0.0f,0.0f,1.0f,
	10.0f
};

static float light[4] = { 10,10,10,0 };

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
		glDrawElements(GL_LINES, indexCount, GL_UNSIGNED_SHORT, indexBuffer);
		break;

	case SMOOTH:
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glShadeModel(GL_SMOOTH);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &material[0]);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &material[4]);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &material[8]);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material[12]);
		glLightfv(GL_LIGHT0, GL_POSITION, light);
		glEnableClientState(GL_NORMAL_ARRAY);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, indexBuffer);
		glDisableClientState(GL_NORMAL_ARRAY);

		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_LIGHT0);
		glDisable(GL_LIGHTING);
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

	setDrawMode(SMOOTH);
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

	clearBuffers();

	if (mesh == NULL)
		return;

	// create the vertex buffer
	vertexCount = mesh->getVertexCount();
	vertexBuffer = new GLfloat[vertexCount * 3];

	// transfer vertex data
	for (i = 0; i < vertexCount; i++){
		tempVertex = mesh->getVertex(i)->position;
		vertexBuffer[i * 3] = tempVertex->x;
		vertexBuffer[i * 3 + 1] = tempVertex->y;
		vertexBuffer[i * 3 + 2] = tempVertex->z;
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertexBuffer);
	glDisableClientState(GL_VERTEX_ARRAY);

	switch (drawMode) {
	case POINT:
		break;

	case WIREFRAME:
		indexCount = mesh->getHalfEdgeCount();
		indexBuffer = new GLushort[indexCount];
		node = mesh->getListHead();
		for (i = 0; i < indexCount && node; node=node->next) {
			// take the first half edge of each twin pair
			
			// has its pair been allocated?
			if (trackEdge.find(node->item.pair) != trackEdge.end())
				continue;

			// allocate this edge
			indexBuffer[i * 2] = node->item.origin->position->id;
			indexBuffer[i * 2 + 1] = node->item.pair->origin->position->id;

			// add this into the set
			trackEdge.insert(&(node->item));
			i++;
		}
		break;

	case SMOOTH:
		indexCount = mesh->getFaceCount() * 3; // Each face has 3 indices
		indexBuffer = new GLushort[indexCount];
		
		for (i = 0; i < mesh->getFaceCount(); i++) {
			curFace = mesh->getFace(i);
			curEdge = curFace->edge;

			// Set the indices
			indexBuffer[i * 3] = curEdge->origin->position->id;
			curEdge = curEdge->next;

			indexBuffer[i * 3+1] = curEdge->origin->position->id;
			curEdge = curEdge->next;

			indexBuffer[i * 3+2] = curEdge->origin->position->id;
		}

		normalCount = mesh->getVertexCount()*3;
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
		break;
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


#include "main.h"

// STATIC VARIABLES
static clock_t prevClock;
static Camera* camera;
static int viewport[2] = { 800, 800 };

static Mesh *mesh = NULL;

static int meshVisible = 1;

// settings
static SHADEMODE shadeMode = SHADEMODE::FLAT;
static GLfloat drawColor[3] = { 1, 1, 1 };


// lighting
static int lightingEnabled = 1;
static Light lights[LIGHTS_AVAILABLE];


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

	glColor3fv(drawColor);

	glShadeModel(shadeMode == SMOOTH ? GL_SMOOTH : GL_FLAT);
	applyLights();
	mesh->render(SOLID);
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

	mesh = Mesh::loadFromFile(filename);
}

// ======================================================
// Removes the loaded mesh from memory
// ======================================================
void unloadMesh() {
	if (mesh != NULL) {
		delete mesh;
		mesh = NULL;
		printf("Disposed mesh\n");
	}
}

// ======================================================
// Called when application terminates. All buffers and mesh
// should be removed
// ======================================================
void cleanup() {
	delete camera;
	unloadMesh();
}


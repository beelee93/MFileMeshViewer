#include "main.h"

// STATIC VARIABLES
static int mainWindow, mainMenu;

static clock_t prevClock;
static Camera* camera;
static int viewport[2] = { 800, 800 };

static Mesh *mesh = NULL;

static int meshVisible = 1;


// settings
static SHADEMODE shadeMode = SHADEMODE::SMOOTH;
static GLfloat drawColor[3] = { 1, 1, 1 };


// lighting
static int lightingEnabled = 1;
static Light lights[LIGHTS_AVAILABLE];

static GLUquadric* _quadricAxes = NULL;

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

	mainWindow = glutCreateWindow("GLUT Demo");
	
	// create the menu
	mainMenu = glutCreateMenu(onMenu);
	glutSetMenu(mainMenu);
	glutAddMenuEntry("Open Model", 0);
	glutAttachMenu(0);

	// set the clearcolor and the callback
	glClearColor(0.0, 0.0, 0.0, 0.0);

	// register your drawing function
	glutDisplayFunc(onRender);
	glutKeyboardFunc(onKey);
	glutReshapeFunc(onWindowResize);

	// set up camera
	camera = new Camera(viewport[0], viewport[1]);

	// create quadrics for axes rendering
	_quadricAxes = gluNewQuadric();

	// set up lights
	for (int i = 0; i < LIGHTS_AVAILABLE; i++)
		lights[i] = Light(GL_LIGHT0 + i);

	lights[0].setEnabled(1);
	prevClock = clock();

	// enter the main loop
	glutMainLoop();
	
	return 0;
}

// ======================================================
// Main rendering functions
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
	glutPostRedisplay();
}

// ======================================================
// Called when a keyboard event is raised
// ======================================================
void onKey(unsigned char key, int x, int y) {

}

// ======================================================
// Main Update function (input processing, etc)
// ======================================================
void update(double elapsed) {

}

// ======================================================
// Drawing calls
// ======================================================
void draw(double elapsed) {
	glLoadIdentity();
	camera->render();
	
	glEnable(GL_DEPTH_TEST);
	drawMesh(elapsed);

	drawPlane(10, 5);
	drawAxes(5,1);
}

// ======================================================
// Helper function to draw axes
// ======================================================

void drawAxes(double length, int ignoreDepthTest) {
	if (_quadricAxes == NULL)
		return;

	if(ignoreDepthTest)
		glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);
	
	// draw x-axis
	glPushMatrix();
	glRotatef(90, 0, 1, 0);
	glColor3f(1, 0, 0);
	gluCylinder(_quadricAxes, 0.025, 0.025, length, 10, 1);
	glTranslatef(0, 0, length);
	glutSolidCone(0.09, 0.23, 10, 1);
	glPopMatrix();
	
	// draw y-axis
	glPushMatrix();
	glRotatef(-90, 1, 0, 0);
	glColor3f(0, 1, 0);
	gluCylinder(_quadricAxes, 0.025, 0.025, length, 10, 1);
	glTranslatef(0, 0, length);
	glutSolidCone(0.09, 0.23, 10, 1);
	glPopMatrix();

	// draw z-axis
	glPushMatrix();
	glColor3f(0, 0, 1);
	gluCylinder(_quadricAxes, 0.025, 0.025, length, 10, 1);
	glTranslatef(0, 0, length);
	glutSolidCone(0.09, 0.23, 10, 1);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);

}

// ======================================================
// This will draw the x-z grid plane
// ======================================================
void drawPlane(double inc, int halfgridcount) {
	double length = inc * halfgridcount;
	int i;

	glColor3f(0, 0, 0);
	glBegin(GL_LINES);

	for (i = -halfgridcount; i <= halfgridcount; i++) {
		// draw the x-aligned grid lines
		glVertex3d(-length, 0, i*inc);
		glVertex3d(length, 0, i*inc);

		// draw the z-aligned grid lines
		glVertex3d(i*inc,0,-length);
		glVertex3d(i*inc,0,length);
	}

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
	mesh->render(MD_SOLID);
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
	if (!mesh->isLoaded())
	{
		delete mesh;
		mesh = NULL;
		printf("main: There was an error loading the MFile\n");
		return;
	}

	// center camera on mesh
	float r = mesh->boundingSphere.radius * 5 / 3;

	camera->eye = Vector3d(r, r, r) + mesh->boundingSphere.center;
	camera->lookAt = mesh->boundingSphere.center;
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

	glutDestroyMenu(mainMenu);

	delete camera;
	unloadMesh();

	if (_quadricAxes != NULL) {
		gluDeleteQuadric(_quadricAxes);
		_quadricAxes = NULL;
	}

}

static char filename[MAX_PATH] = "";
void onMenu(int menuId) {
	switch (menuId) {
	case MENU_LOAD:
		// create open file dialog
		OPENFILENAMEA dlgStruct = { 0 };
		dlgStruct.lStructSize = sizeof(OPENFILENAMEA);
		dlgStruct.lpstrFilter = "M Files (*.m)\0*.m\0\0";
		dlgStruct.lpstrTitle = "Open Model";
		dlgStruct.lpstrFile = filename;
		dlgStruct.nMaxFile = MAX_PATH;
		dlgStruct.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

		if (GetOpenFileNameA(&dlgStruct)) {
			// user selected and pressed OK
			loadMesh(filename);
		}
		break;
	}
}

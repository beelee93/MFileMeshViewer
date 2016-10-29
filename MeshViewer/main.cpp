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
static MeshDrawMode renderMode = MeshDrawMode::MD_SOLID;
static GLfloat drawColor[3] = { 1, 1, 1 };
static bool enableDrawGrid = true;
static bool enableDrawAxes = true;

// lighting
static int lightingEnabled = 1;
static Light lights[LIGHTS_AVAILABLE];

static GLUquadric* _quadricAxes = NULL;

// camera movement vectors
static Vector3d vpanH;
static Vector3d vpanV;
static Vector3d vzoom;
static int gestureSampleCount;
static int gestureLastMouse[2];
static double gestureReading[2];

// file loading
static char filename[MAX_PATH] = "";

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

	mainWindow = glutCreateWindow("MFile Viewer (Ong Bee Lee)");
	
	// set the clearcolor and the callback
	glClearColor(0.0, 0.0, 0.0, 0.0);

	// register your drawing function
	glutDisplayFunc(onRender);
	glutKeyboardFunc(onKey);
	glutMotionFunc(onMouseMove);
	glutMouseFunc(onMouse);
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
	switch (key) {
	case 'O':
	case 'o':
	{
		/********** OPEN MODEL **********/
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
	case 'P':
	case 'p':
		/********** SWITCH PERSPECTIVE/ORTHOGONAL **********/
		camera->ortho = !camera->ortho;
		break;

	case 'S':
	case 's':
		/********** SHADE MODES **********/
		shadeMode = shadeMode == SHADEMODE::FLAT ? SHADEMODE::SMOOTH : SHADEMODE::FLAT;
		break;

	case 'G':
	case 'g':
		/********** Toggle Grid **********/
		enableDrawGrid = !enableDrawGrid;
		break;

	case 'A':
	case 'a':
		/********** Toggle Axis **********/
		enableDrawAxes = !enableDrawAxes;
		break;


	case 'C':
	case 'c': 
	{
		/********** Center Camera on Mesh **********/
		if (mesh != NULL) {
			float r = mesh->boundingSphere.radius * 5 / 3;

			camera->eye = Vector3d(r, r, r) + mesh->boundingSphere.center;
			camera->lookAt = mesh->boundingSphere.center;
			camera->up = Vector3d(0, 1, 0);
		}
		else {
			camera->eye = Vector3d(10, 10, 10);
			camera->lookAt = Vector3d(0, 0, 0);
			camera->up = Vector3d(0, 1, 0);
		}
		break;
	}

	case '1':
		/********** SOLID RENDERING **********/
		renderMode = MD_SOLID;
		break;
	case '2':
		/********** SOLID WIREFRAME RENDERING **********/
		renderMode = MD_SOLID_AND_WIREFRAME;
		break;
	case '3':
		/********** WIREFRAME RENDERING **********/
		renderMode = MD_WIREFRAME;
		break;
	case '4':
		/********** POINT RENDERING **********/
		renderMode = MD_POINT;
		break;
	}
}

// ======================================================
// Main Update function (input processing, etc)
// ======================================================
void update(double elapsed) {

}

// ======================================================
// Mouse Events
// ======================================================
void onMouseDown(int x, int y, int button) {
	// set up vectors for camera movements
	vzoom = camera->lookAt - camera->eye;
	if (button == GLUT_LEFT_BUTTON) {
		// set up rotational axes
		vpanV = vzoom.cross(camera->up); // rotate vertical (horizontal axis)
		vpanH = vpanV.cross(vzoom); // rotate horizontal (vertical axis)
	}
	else {
		vzoom = camera->lookAt - camera->eye;
		vpanH = vzoom.cross(camera->up);
		vpanV = vpanH.cross(vzoom);
	}
	
	vzoom.normalize();
	vpanH.normalize();
	vpanV.normalize();

	gestureReading[0] = gestureReading[1] = 0;
	gestureSampleCount = 0;

	gestureLastMouse[0] = x;
	gestureLastMouse[1] = y;

}

void onMouseUp(int x, int y, int button) {

}

// a helper function for gesture processing
inline void updateCameraVectors() {
	// get right vector
	Vector3d tempVect2 = camera->lookAt - camera->eye;
	Vector3d tempVect = tempVect2.cross(vpanH);
	camera->up = tempVect.cross(tempVect2);
	camera->up.normalize();


	vzoom = camera->lookAt - camera->eye;
	vpanV = vzoom.cross(camera->up); // rotate vertical (horizontal axis)
	vpanH = vpanV.cross(vzoom); // rotate horizontal (vertical axis)
	vzoom.normalize();
	vpanH.normalize();
	vpanV.normalize();
}

inline double adjustToDistance() {
	Vector3d tempVect2 = camera->lookAt - camera->eye;
	return pow(tempVect2.lengthSquared(), 0.25);
}

inline double getSmallestRotation(double a, double b) {
	const double PI = 3.141592;
	double temp = b - a;
	int dir = (a - b > 0) ? 1 : -1;

	while (-PI > temp || temp > PI)
	{
		b += dir * 2 * PI;
		temp = b - a;
	}
	return temp;
}

// ======================================================
// Processing of gesture
// ======================================================
void processGesture(int x, int y, int gesture, int button) {
	double deltaX, deltaY, temp;
	Vector3d tempVect, tempVect2;

	gestureSampleCount++;

	switch (button) {
	case GLUT_LEFT_BUTTON:
		/************ LEFT MOUSE BUTTON ************/
		switch (gesture) {
		case GESTURE_HORIZONTAL:
			if (gestureSampleCount >= 2) {
				deltaX = (x - gestureLastMouse[0]) * PAN_SENSITIVITY;

				tempVect2 = camera->lookAt - camera->eye;
				tempVect = vpanV * sin(ROTATE_ANGLE_STEP * deltaX) + vzoom * cos(ROTATE_ANGLE_STEP * deltaX);
				tempVect = tempVect * tempVect2.length();
				camera->eye = camera->lookAt - tempVect;
				
				updateCameraVectors();
			}
			break;

		case GESTURE_VERTICAL:
			if (gestureSampleCount >= 2) {
				deltaY = (y - gestureLastMouse[1]) * PAN_SENSITIVITY;

				tempVect2 = camera->lookAt - camera->eye;
				tempVect = vpanH * -sin(ROTATE_ANGLE_STEP * deltaY) + vzoom * cos(ROTATE_ANGLE_STEP * deltaY);
				tempVect = tempVect * tempVect2.length();
				camera->eye = camera->lookAt - tempVect;

				// get right vector
				updateCameraVectors();
			}

			break;


		case GESTURE_CIRCLE:
			if (gestureSampleCount >= 2) {
				// determine angle change (subtended at center of screen)
				deltaX = atan2(gestureLastMouse[1] - viewport[1] / 2, gestureLastMouse[0] - viewport[0] / 2);
				deltaY = atan2(y - viewport[1] / 2, x - viewport[0] / 2);

				// find difference
				deltaX = getSmallestRotation(deltaX, deltaY) * ROTATE_ANGLE_STEP * 5;;

				// rotate right vector
				tempVect2 = camera->lookAt - camera->eye;
				tempVect = vpanV * cos(deltaX) + vpanH * sin(deltaX);
				camera->up = tempVect.cross(tempVect2);

				camera->up.normalize();

				vzoom = camera->lookAt - camera->eye;
				vpanV = vzoom.cross(camera->up); // rotate vertical (horizontal axis)
				vpanH = vpanV.cross(vzoom); // rotate horizontal (vertical axis)
				vzoom.normalize();
				vpanH.normalize();
				vpanV.normalize();
			}
			break;
		}
		break;

	case GLUT_MIDDLE_BUTTON:
		/************ MIDDLE MOUSE BUTTON ************/
		// Translate camera
		if (gesture != GESTURE_HORIZONTAL && gesture != GESTURE_VERTICAL)
			return;

		if (gestureSampleCount >= 2) {
			temp = adjustToDistance();
			deltaX = (x - gestureLastMouse[0]) * PAN_SENSITIVITY * temp;
			deltaY = (y - gestureLastMouse[1]) * PAN_SENSITIVITY * temp;

			camera->lookAt = camera->lookAt - (vpanH * deltaX) + (vpanV * deltaY);;
			camera->eye = camera->eye - (vpanH * deltaX) + (vpanV * deltaY);
		}

		/*******************************************/
		break;

	case GLUT_RIGHT_BUTTON:
		/************ MIDDLE MOUSE BUTTON ************/
		// Zoom camera

		if (gestureSampleCount >= 2) {
			temp = adjustToDistance();
			printf("%lf\n", temp);
			deltaY = (y - gestureLastMouse[1]) * ZOOM_SENSITIVITY * temp;
			tempVect = camera->eye - (vzoom * deltaY);
			tempVect2 = camera->eye - camera->lookAt;

			if (tempVect.dot(tempVect2) > 0) { // don't go pass "lookAt" 
				camera->eye = tempVect;

				tempVect2 = camera->eye - camera->lookAt;
				if (tempVect2.lengthSquared() < 4) {
					tempVect2.normalize();
					tempVect2 = tempVect2 * 2;
					camera->eye = camera->lookAt + tempVect2;
				}
			}
		}
		/*******************************************/
		break;
	}

	gestureLastMouse[0] = x;
	gestureLastMouse[1] = y;
}

// ======================================================
// Drawing calls
// ======================================================
void draw(double elapsed) {
	glLoadIdentity();
	camera->render();
	
	glEnable(GL_DEPTH_TEST);
	drawMesh(elapsed);

	if(enableDrawGrid)
		drawPlane(10, 5);

	// draw 3d axes
	if(enableDrawAxes)
		drawAxes(5,1);

	// draw overlay axes
	drawOverlayAxes();

	glColor3f(0, 0, 0);
	drawText(camera->ortho ? "Orthographic" : "Perspective", 32, viewport[1]-9);
	
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
// Helper function to draw overlay axes (bottom left corner)
// ======================================================
void drawOverlayAxes() {
	Vector3d temp;

	glViewport(0, 0, 32, 32);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(camera->perspectiveFOV, 1, 1, 1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();

	temp = camera->eye - camera->lookAt;
	temp.normalize();
	temp = temp * 10;

	gluLookAt(temp.x, temp.y, temp.z, 0, 0, 0, camera->up.x, camera->up.y, camera->up.z);
	glLineWidth(3.0f);
	glDisable(GL_DEPTH_TEST);
	glBegin(GL_LINES);
	glColor3d(1, 0, 0);
	glVertex3d(0, 0, 0);
	glVertex3d(3, 0, 0);

	glColor3d(0, 1, 0);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 3, 0);

	glColor3d(0, 0, 1);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 0, 3);
	glEnd();
	glEnable(GL_DEPTH_TEST);
	glLineWidth(1.0f);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glViewport(0, 0, viewport[0], viewport[1]);
}

// ======================================================
// This will draw text
// ======================================================
void drawText(const char* str, int x, int y) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, viewport[0], viewport[1], 0);
	glRasterPos2d(x, y);
	for (int i = 0; str[i] != 0; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
	}
	glPopMatrix();
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
	mesh->render(renderMode);
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

	delete camera;
	unloadMesh();

	if (_quadricAxes != NULL) {
		gluDeleteQuadric(_quadricAxes);
		_quadricAxes = NULL;
	}

}

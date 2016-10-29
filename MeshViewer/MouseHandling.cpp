#include "main.h"

static int buttonPressed = -1;
static int gestureDetected = -1;

static struct {
	int pa[2];
	int pb[2];
	Vector3d a, b;
	double ctheta[2];
	int circleCounter;
	int straightCounter;
	int t;
	int sampleCount;
} GestureCounter;

void resetGestureCounter() {
	GestureCounter.a.x = GestureCounter.a.y = GestureCounter.a.z = 0;
	GestureCounter.b.x = GestureCounter.b.y = GestureCounter.b.z = 0;
	GestureCounter.pa[0] = GestureCounter.pa[1] = GestureCounter.pb[0] = GestureCounter.pb[1] = 0;
	GestureCounter.t = GestureCounter.circleCounter = GestureCounter.straightCounter = 0;
	GestureCounter.sampleCount = 0;
	GestureCounter.ctheta[0] = GestureCounter.ctheta[1] = 0;

	gestureDetected = -1;
}

// ======================================================
// Called when a mouse event is raised
// ======================================================
void onMouse(int button, int state, int x, int y) {
	// ensure only one button is processed at any given time
	if (buttonPressed < 0 && state == GLUT_DOWN) {
		buttonPressed = button;
		onMouseDown(x, y, button);

		resetGestureCounter();
		GestureCounter.b.x = x;
		GestureCounter.b.y = y;
	}
	else if (buttonPressed == button && state == GLUT_UP) {
		buttonPressed = -1;
		onMouseUp(x, y, button);
	}
}

// ======================================================
// Called when mouse is moved within window
// ======================================================
void onMouseMove(int x, int y) {
	Vector3d &a = GestureCounter.a;
	Vector3d &b = GestureCounter.b;
	int *pa = GestureCounter.pa;
	int *pb = GestureCounter.pb;
	double *ctheta = GestureCounter.ctheta;
	double temp;
	int &ccounter = GestureCounter.circleCounter;
	int &scounter = GestureCounter.straightCounter;

	if (gestureDetected >= 0) {
		processGesture(x, y, gestureDetected, buttonPressed);
	}
	else if (buttonPressed >= 0) {
		// a button is held down
		GestureCounter.t++;
		if (GestureCounter.t % 2 == 0) {
			GestureCounter.sampleCount++;

			// ensure mouse has moved significantly

			// get the absolute points
			pa[0] = pb[0];
			pa[1] = pb[1];

			pb[0] = x;
			pb[1] = y;

			// get the difference vectors
			a.x = b.x;
			a.y = b.y;

			b.x = pb[0] - pa[0];
			b.y = pb[1] - pa[1];

			// don't continue if this was first/second sample sample
			if (GestureCounter.sampleCount >= 2) {
				ctheta[0] = ctheta[1];
				ctheta[1] = a.dot(b) / a.length() / b.length();

				temp = ctheta[1] - ctheta[0];
				if (-GESTURE_TOLERANCE < temp && temp < GESTURE_TOLERANCE) {
					// check that the movement is not aligned in 45 degrees direction
					temp = 1 - 2 * b.y*b.y / b.lengthSquared();

					if (-0.8 < temp && temp < 0.8)
						ccounter++;
					else
						scounter++;
				}
				else
					ccounter++;

				if (scounter > GESTURE_COUNT_THRESHOLD) {
					// straight gesture

					// determine axis
					if (b.x * b.x < b.y*b.y) {
						// change in x is less significant than that in y
						// assume vertical
						gestureDetected = GESTURE_VERTICAL;
					}
					else {
						gestureDetected = GESTURE_HORIZONTAL;
					}
				}
				else if (ccounter > GESTURE_COUNT_THRESHOLD)
					gestureDetected = GESTURE_CIRCLE;

			}
		}
	}
}

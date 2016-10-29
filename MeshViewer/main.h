#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>
#include <string.h>
#include <vector>
#include <Windows.h>
#include <commdlg.h>
#include <time.h>
#include <gl/glut.h>
#include "lib/glui.h"
#include "LinkedList.h"
#include "MFile.h"
#include "Mesh.h"
#include "Camera.h"
#include "HalfEdge.h"
#include "Vector3D.h"
#include "Light.h"
#include "Material.h"

#define LIGHTS_AVAILABLE 2

#define GESTURE_TOLERANCE 0.05
#define GESTURE_COUNT_THRESHOLD 5

#define GESTURE_VERTICAL	0
#define GESTURE_HORIZONTAL	1
#define GESTURE_CIRCLE		2

#define PAN_SENSITIVITY		0.1
#define ZOOM_SENSITIVITY	0.3
#define ROTATE_ANGLE_STEP	0.1
enum SHADEMODE { FLAT, SMOOTH };

void cleanup();

void onRender(void);
void onWindowResize(int, int);
void onKey(unsigned char key, int x, int y);
void update(double elapsed);
void draw(double elapsed);
void drawAxes(double length, int ignoreDepthTest = 0);
void drawOverlayAxes();
void drawPlane(double inc, int halfgridcount);
void loadMesh(const char* filename);
void unloadMesh();
void applyLights();
void drawMesh(double);

void onMouse(int, int, int, int);
void onMouseMove(int, int);

void onMouseDown(int, int, int);
void onMouseUp(int, int, int);
void processGesture(int, int, int, int);

void drawText(const char* str, int x, int y);

// menu items
#define MENU_LOAD 0

#endif

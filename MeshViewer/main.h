#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>
#include <string.h>
#include <vector>
#include <set>
#include <time.h>
#include "lib/glut.h"
#include "LinkedList.h"
#include "MFile.h"
#include "Mesh.h"
#include "Camera.h"
#include "HalfEdge.h"
#include "Vector3D.h"
#include "Light.h"

#define LIGHTS_AVAILABLE 2

enum DRAWMODE { WIREFRAME=0, POINT=1, SOLID=2, SOLID_AND_WIREFRAME=3 };
enum SHADEMODE { FLAT, SMOOTH };

void cleanup();
void onRender(void);
void onWindowResize(int, int);
void onKeyUp(unsigned char key, int x, int y);
void update(double elapsed);
void draw(double elapsed);
void drawAxes(double);
void loadMesh(const char* filename);
void unloadMesh();
void applyLights();
void drawMesh(double);
void setDrawMode(DRAWMODE newDrawMode);
void clearBuffers();

#endif

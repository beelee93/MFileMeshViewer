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

enum SHADEMODE { FLAT, SMOOTH };

void cleanup();

void onRender(void);
void onWindowResize(int, int);
void onKey(unsigned char key, int x, int y);
void update(double elapsed);
void draw(double elapsed);
void drawAxes(double length, int ignoreDepthTest = 0);
void drawPlane(double inc, int halfgridcount);
void loadMesh(const char* filename);
void unloadMesh();
void applyLights();
void drawMesh(double);

void onMenu(int);

// menu items
#define MENU_LOAD 0

#endif

#ifndef _HALFEDGE_H
#define _HALFEDGE_H

#include "Vector3D.h"

struct _HEVertex;
struct _HEEdge;
struct _HEFace;

typedef struct _HEFace {
	struct _HEEdge* edge;
	Vector3d normal;
} HEFace;

typedef struct _HEVertex {
	Vector3d* position;   // referenced from MFile
	Vector3d normal;
	struct _HEEdge* edge;
} HEVertex;

typedef struct _HEEdge {
	struct _HEEdge* next;
	struct _HEEdge* prev;
	struct _HEEdge* pair;
	HEVertex* origin;
	HEFace* face;
} HEEdge;


#endif
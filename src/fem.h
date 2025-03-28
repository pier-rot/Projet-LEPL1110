#ifndef _FEM_H_
#define _FEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "gmshc.h"


#define ErrorGmsh(a)   femErrorGmsh(a,__LINE__,__FILE__)
#define MAXNAME 128

typedef struct {
    int nNodes;
    double *X;
    double *Y;
} femNodes;

typedef struct {
    int nLocalNode;
    int nElem;
    int *elem;
    femNodes *nodes;
} femMesh;

typedef struct {
    femMesh *mesh;
    int nElem;
    int *elem;
    char name[MAXNAME];
} femDomain;


typedef struct {
    double xPlate, yPlate, LxPlate, LyPlate;
    double xHole, yHole, rHole, hHole, dHole;
    double xNotch, yNotch, rNotch, hNotch, dNotch;
    double h;
    double (*geoSize)(double x, double y);
    femNodes *theNodes;
    femMesh  *theElements;
    femMesh  *theEdges;
    int nDomains;
    femDomain **theDomains;
} femGeo;


void geoSetSizeCallback(double (*geoSize)(double x, double y));
#endif
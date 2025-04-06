#ifndef _UTILS_H
#define _UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAXNAME 128

typedef enum {FEM_TRIANGLE,FEM_QUAD,FEM_EDGE} femElementType;
typedef enum {DIRICHLET_X,DIRICHLET_Y,NEUMANN_X,NEUMANN_Y} femBoundaryType;
typedef enum {PLANAR_STRESS,PLANAR_STRAIN,AXISYM} femElasticCase;

typedef struct {
    int nNodes;
    double* X;
    double* Y;
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
    femNodes* nodes;
    femElementType elementType;
    femMesh* mesh;
    femMesh* edges;
    int nDomains;
    femDomain** domains;
} femGeo;

typedef struct {
    int n;
    femElementType type;
} femDiscrete;

typedef struct {
    int n;
    const double* xsi;
    const double* eta;
    const double* weight;
} femIntegration;

typedef struct {
    double* B;
    double** A;
    int size;
} femFullSystem;

typedef struct {
    femDomain* domain;
    femBoundaryType type; 
    double value;
} femBoundaryCondition;

typedef struct {
    double E,nu,rho,g;
    double A,B,C;
    int planarStrainStress;
    int nBoundaryConditions;
    femBoundaryCondition** conditions;  
    int* constrainedNodes;

    // not sure if needed
    double* soluce;
    double* residuals;


    femGeo* geometry;
    femDiscrete* space;
    femIntegration* rule;
    femDiscrete* spaceEdge;
    femIntegration* ruleEdge;
    femFullSystem* system;
} femProblem;

femGeo* geoRead(const char *filename);
femGeo* geoInit();
void geoFree(femGeo* geo);
void geoPrint(femGeo* geo);
void geoSetDomain(femGeo* geo, int iDomain, char* name);

double femMin(double *x, int n);
double femMax(double *x, int n);

#endif
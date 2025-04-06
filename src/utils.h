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
typedef enum {SOLVER_FULL, SOLVER_BAND, SOLVER_GC} femSolverType;
typedef enum {NONE, X, Y, RCMK} femRenumberType;

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
    void (*x2)(double *xsi, double *eta);
    void (*phi2)(double xsi, double eta, double *phi);
    void (*dphi2dx)(double xsi, double eta, double *dphidxsi, double *dphideta);
    void (*x)(double *xsi);
    void (*phi)(double xsi, double *phi);
    void (*dphidx)(double xsi, double *dphidxsi);
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
    double** A;
    double* B;
    int band;
} femBandSystem;

typedef struct {
    femSolverType type;
    void* solver;
    int size;
} femSolver;

typedef struct {
    femDomain* domain;
    femBoundaryType type; 
    double value;
} femBoundaryCondition;

typedef struct {
    double E,nu,rho,g,T;
    double A,B,C;
    int planarStrainStress;
    int nBoundaryConditions;
    femBoundaryCondition** conditions;  
    int* constrainedNodes;

    // not sure if needed
    double* soluce;
    double* residuals;

    femSolverType solverType;
    femGeo* geometry;
    femDiscrete* space;
    femIntegration* rule;
    femDiscrete* spaceEdge;
    femIntegration* ruleEdge;
    femFullSystem* system;
} femProblem;

// Geometry functions
femGeo* geoRead(const char *filename);
femGeo* geoInit();
void geoFree(femGeo* geo);
void geoPrint(femGeo* geo);
void geoSetDomain(femGeo* geo, int iDomain, char* name);
int geoGetDomain(femGeo* geo, char* name);

// Integration functions
femIntegration* femIntegrationCreate(int n, femElementType type);
void femIntegrationFree(femIntegration* rule);

// Discretisation functions
femDiscrete* femDiscreteCreate(int n, femElementType type);
void femDiscreteFree(femDiscrete* mySpace);
void femDiscreteXsi2(femDiscrete* mySpace, double *xsi, double *eta);
void femDiscretePhi2(femDiscrete* mySpace, double xsi, double eta, double *phi);
void femDiscreteDphi2(femDiscrete* mySpace, double xsi, double eta, double *dphidxsi, double *dphideta);
void femDiscreteXsi(femDiscrete* mySpace, double *xsi);
void femDiscretePhi(femDiscrete* mySpace, double xsi, double *phi);
void femDiscreteDphi(femDiscrete* mySpace, double xsi, double *dphidxsi);
void femDiscretePrint(femDiscrete *mySpace);

// System functions
void femFullSystemAlloc(femFullSystem* system, int size);
void femFullSystemInit(femFullSystem* system);
femFullSystem* femFullSystemCreate(int size);
void femFullSystemFree(femFullSystem* system);
void femFullSystemPrint(femFullSystem* system);
double* femFullSystemEliminate(femFullSystem* system);
void femFullSystemConstrain(femFullSystem* system, int node, double value);

// Linear elasticity functions
femProblem* femElasticityCreate(femGeo* geo, double E, double nu, double rho, double g, double T, femElasticCase iCase);
void femElasticityPrint(femProblem* problem);
void femElasticityAddBoundaryCondition(femProblem* problem, char* name, femBoundaryType type, double value);
void femElasticityAssembleElements(femProblem* problem);
void femElasticityAssembleNeumann(femProblem* problem);
double* femElasticitySolve(femProblem* problem);
double* femElasticityForces(femProblem* problem);
double femElasticityIntegrate(femProblem* problem, double (*f)(double x, double y));
void femElasticityFree(femProblem* problem);

#endif
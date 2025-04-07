#ifndef _UTILS_H
#define _UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "gmshc.h"

#define MAXNAME 128
#define FALSE 0 
#define TRUE  1



typedef enum {FEM_TRIANGLE,FEM_QUAD,FEM_EDGE} femElementType;
typedef enum {DIRICHLET_X,DIRICHLET_Y,NEUMANN_X,NEUMANN_Y} femBoundaryType;
typedef enum {PLANAR_STRESS,PLANAR_STRAIN,AXISYM} femElasticCase;
typedef enum {SOLVER_FULL, SOLVER_BAND, SOLVER_GC} femSolverType;
typedef enum {NONE, X, Y, RCMK} femRenumberType;

typedef struct {
    int nNodes;
    double* X;
    double* Y;
    int* number;
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
    int size;
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

    femRenumberType renumberType;
    femSolverType solverType;

    femGeo* geometry;
    femDiscrete* space;
    femIntegration* rule;
    femDiscrete* spaceEdge;
    femIntegration* ruleEdge;
    femSolver* solver;
    //femFullSystem *system;
    //femBandSystem *bandSystem;
} femProblem;

// Geometry functions
femGeo* geoRead(const char *filename);
femGeo* geoInit();
void geoFree(femGeo* geo);
void geoPrint(femGeo* geo);
void geoNodesPrint(femGeo* geo);
void geoSetDomain(femGeo* geo, int iDomain, char* name);
int geoGetDomain2(femGeo* geo, char* name);
void femMeshRenumber(femMesh* mesh, femRenumberType type);
int compare(const void *N1, const void *N2);
void geoMeshGenerate();

//void geoMeshImport(); //TO DO
//void geoMeshWrite(const char* filename); //TO DO

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

// Full system functions
void femFullSystemAlloc(femFullSystem* system, int size);
void femFullSystemInit(femFullSystem* system);
femFullSystem* femFullSystemCreate(int size);
void femFullSystemFree(femFullSystem* system);
void femFullSystemPrint(femFullSystem* system);
double* femFullSystemEliminate(femFullSystem* system, int size);
void femFullSystemAssemble(femFullSystem* system, femProblem* problem, int* mapX, int* mapY, 
                          double* phi, double* dphidx, double* dphidy, double xLoc, double wJac, double nLoc);
void femFullSystemConstrain(femFullSystem* system, int node, double value);

// Band system functions
void femBandSystemAlloc(femBandSystem* system, int size, int band);
void femBandSystemInit(femBandSystem* system, int size);
femBandSystem* femBandSystemCreate(int band, int size);
void femBandSystemFree(femBandSystem* system);
void femBandSystemPrint(femBandSystem* system, int size);
void femBandSystemAssemble(femBandSystem* system, femProblem* problem, int* mapX, int* mapY, 
                          double* phi, double* dphidx, double* dphidy, double xLoc, double wJac, double nLoc);
double* femBandSystemEliminate(femBandSystem* system, int size);
int inBand(int band, int row, int col);
void femBandSystemConstrain(femBandSystem* system, int node, double value, int size);

// Renumbering functions
void femRenumber(femGeo* geo, femRenumberType type);
int femComputeBand(femGeo* geo);

// Solver functions
femSolver* femSolverCreate(int size);
femSolver* femSolverBandCreate(int size, int band);
femSolver* femSolverFullCreate(int size);
void femSolverFree(femSolver* solver);
void femSolverInit(femSolver* solver);
void femSolverPrint(femSolver* solver);
void femSolverAssemble(femSolver *solver, femProblem *problem, int *mapX, int *mapY, double *phi, double *dphidx, double *dphidy, double weightedJac, double xLoc, int nLoc);
void femSolverSystemConstrain(femSolver* solver, int node, double value);
double* femSolverEliminate(femSolver* solver);

// Linear elasticity functions
femProblem* femElasticityCreate(femGeo* geo, double E, double nu, double rho, double g, double T, femElasticCase iCase);
femProblem* femElasticityRead(femGeo* geo, const char*, femSolverType solverType, femRenumberType renumberType);
void femElasticityPrint(femProblem* problem);
void femElasticityFullPrint(femProblem* problem);
void femElasticityAddBoundaryCondition(femProblem* problem, char* name, femBoundaryType type, double value);
void femElasticityAssembleElements(femProblem* problem);
void femElasticityAssembleNeumann(femProblem* problem);
void femElasticityApplyDirichlet(femProblem* problem);
double* femElasticitySolve(femProblem* problem);
double* femElasticityForces(femProblem* problem);
double femElasticityIntegrate(femProblem* problem, double (*f)(double x, double y));
void femElasticityFree(femProblem* problem);

double              femMin(double *x, int n);
double              femMax(double *x, int n);
void                femError(char *text, int line, char *file);
void                femErrorScan(int test, int line, char *file);
void                femErrorGmsh(int test, int line, char *file);
void                femWarning(char *text, int line, char *file);


#endif
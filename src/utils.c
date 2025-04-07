#include "utils.h"

femGeo* geoInit() {
    femGeo* geo = (femGeo*)malloc(sizeof(femGeo));
    if (geo == NULL) {
        fprintf(stderr, "Memory allocation failed for femGeo structure.\n");
        return NULL;
    }

    geo->nodes = (femNodes*)malloc(sizeof(femNodes));    
    if (geo->nodes == NULL) {
        fprintf(stderr, "Memory allocation failed for femNodes structure.\n");
        free(geo);
        return NULL;
    }


    geo->mesh = (femMesh*)malloc(sizeof(femMesh));
    if (geo->mesh == NULL) {
        fprintf(stderr, "Memory allocation failed for femMesh structure.\n");
        free(geo->nodes);
        free(geo);
        return NULL;
    }

    geo->edges = (femMesh*)malloc(sizeof(femMesh));
    if (geo->edges == NULL) {
        fprintf(stderr, "Memory allocation failed for femMesh structure.\n");
        free(geo->mesh);
        free(geo->nodes);
        free(geo);
        return NULL;
    }
    geo->nDomains = 0;
    geo->domains = (femDomain**)malloc(sizeof(femDomain*));
    if (geo->domains == NULL) {
        fprintf(stderr, "Memory allocation failed for femDomain structure.\n");
        free(geo->edges);
        free(geo->mesh);
        free(geo->nodes);
        free(geo);
        return NULL;
    }
    return geo;
}

void geoFree(femGeo* geo){
    if (geo->nodes){
        free(geo->nodes->X);
        free(geo->nodes->Y);
        free(geo->nodes);
    }

    if (geo->mesh){
        free(geo->mesh->elem);
        free(geo->mesh);
    }

    if (geo->edges){
        free(geo->edges->elem);
        free(geo->edges);
    }
    
    if (geo->domains){
        for (int i = 0; i < geo->nDomains; i++){
            free(geo->domains[i]->elem);
            free(geo->domains[i]);
        }
        free(geo->domains);
    }
    free(geo);
}

femGeo* geoRead(const char* filename){

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open file %s\n", filename);
        return NULL;
    }

    femGeo* geo = geoInit();
    if (geo == NULL) {
        fprintf(stderr, "Failed to initialize femGeo structure.\n");
        return NULL;
    }

    femNodes* nodes = geo->nodes;
    femMesh* mesh = geo->mesh;
    femMesh* edges = geo->edges;
    geo->mesh->nodes = nodes;

    // Read the number of nodes
    fscanf(file, "Number of nodes %d\n", &nodes->nNodes);
    nodes->X = (double*)malloc(nodes->nNodes * sizeof(double));
    nodes->Y = (double*)malloc(nodes->nNodes * sizeof(double));
    nodes->number = (int*)malloc(nodes->nNodes * sizeof(int));
    
    // Err check for memory allocation
    if (nodes->X == NULL || nodes->Y == NULL) {
        fprintf(stderr, "Memory allocation failed for node coordinates.\n");
        geoFree(geo);
        return NULL;
    }

    // Read the node coordinates
    for (int i = 0; i < nodes->nNodes; i++) {
        fscanf(file, "%6d : %14le %14le\n", &i, &nodes->X[i], &nodes->Y[i]);
    }

    // Read the number of edges
    fscanf(file, "Number of edges %d\n", &edges->nElem);
    edges->elem = (int*)malloc(edges->nElem * 2 * sizeof(int));
    for(int i = 0; i < edges->nElem; i++){
        fscanf(file, "%6d : %6d %6d\n", &i, &edges->elem[2*i], &edges->elem[2*i+1]);
    }

    // Read the number of triangles
    char elemType[16];
    fscanf(file, "Number of %s %d\n", elemType, &mesh->nElem);
    if (strcmp(elemType, "triangles") == 0) {
        geo->elementType = FEM_TRIANGLE;
        mesh->nLocalNode = 3;

        mesh->elem = (int*)malloc(mesh->nElem * 3 * sizeof(int));
        if (mesh->elem == NULL) {
            fprintf(stderr, "Memory allocation failed for triangle indices.\n");
            geoFree(geo);
            return NULL;
        }
        // Read the triangles indices
        for (int i = 0; i < mesh->nElem; i++) {
            fscanf(file, "%6d : %6d %6d %6d\n", &i, &mesh->elem[3*i], &mesh->elem[3*i+1], &mesh->elem[3*i+2]);
        }
    } else if (strcmp(elemType, "quads") == 0) {
        geo->elementType = FEM_QUAD;
        mesh->nLocalNode = 4;

        mesh->elem = (int*)malloc(mesh->nElem * 4 * sizeof(int));
        if (mesh->elem == NULL) {
            fprintf(stderr, "Memory allocation failed for quad indices.\n");
            geoFree(geo);
            return NULL;
        }
        // Read the quads indices
        for (int i = 0; i < mesh->nElem; i++) {
            fscanf(file, "%6d : %6d %6d %6d %6d\n", &i, &mesh->elem[4*i], &mesh->elem[4*i+1], &mesh->elem[4*i+2], &mesh->elem[4*i+3]);
        }
    } else if (strcmp(elemType, "edges") == 0) {
        geo->elementType = FEM_EDGE;
        mesh->nLocalNode = 2;
    
        mesh->elem = (int*)malloc(mesh->nElem * 2 * sizeof(int));
        if (mesh->elem == NULL) {
            fprintf(stderr, "Memory allocation failed for edge indices.\n");
            geoFree(geo);
            return NULL;
        }
        // Read the edges indices
        for (int i = 0; i < mesh->nElem; i++) {
            fscanf(file, "%6d : %6d %6d\n", &i, &mesh->elem[2*i], &mesh->elem[2*i+1]);
        }
    } else {
        fprintf(stderr, "Invalid element type: %s\n", elemType);
        geoFree(geo);
        return NULL;
    }


    // Read the number of domains
    fscanf(file, "Number of domains %d\n", &geo->nDomains);
    geo->domains = (femDomain**)malloc(geo->nDomains * sizeof(femDomain*));
    if (geo->domains == NULL) {
        fprintf(stderr, "Memory allocation failed for domains.\n");
        geoFree(geo);
        return NULL;
    }

    // Read the domains
    for(int iDomain = 0; iDomain < geo->nDomains; iDomain++){
        geo->domains[iDomain] = (femDomain*)malloc(sizeof(femDomain));
        if (geo->domains[iDomain] == NULL) {
            fprintf(stderr, "Memory allocation failed for domain %d.\n", iDomain);
            geoFree(geo);
            return NULL;
        }
        geo->domains[iDomain]->mesh = edges;
        int iTemp;
        // Read the domain information
        fscanf(file, "  Domain : %6d \n", &iTemp);
        fscanf(file, "  Name : %[^\n]s \n", geo->domains[iDomain]->name);
        fscanf(file, "  Number of elements : %6d\n", &geo->domains[iDomain]->nElem);
        geo->domains[iDomain]->elem = (int*)malloc(geo->domains[iDomain]->nElem * sizeof(int));
        for(int i = 0; i < geo->domains[iDomain]->nElem; i++) {
            
            if ( (i+1) != geo->domains[iDomain]->nElem && (i+1) % 10 == 0) {
                fscanf(file, "%6d \n", &geo->domains[iDomain]->elem[i]);
            } else {
                fscanf(file, "%6d", &geo->domains[iDomain]->elem[i]);
            }
        }

    }

    fclose(file);
    return geo;
}

void geoPrint(femGeo* geo){
    printf("Number of nodes: %d\n", geo->nodes->nNodes);
    printf("Number of elements: %d\n", geo->mesh->nElem);
    printf("Number of edges: %d\n", geo->edges->nElem);
    printf("Number of domains: %d\n", geo->nDomains);
    printf("Element type: ");
    if (geo->elementType == FEM_TRIANGLE) {
        printf("Triangles\n");
    } else if (geo->elementType == FEM_QUAD) {
        printf("Quads\n");
    } else if (geo->elementType == FEM_EDGE) {
        printf("Edges\n");
    } else {
        printf("Unknown\n");
    }

    for(int iDomain = 0; iDomain < geo->nDomains; iDomain++) {
        printf("  Domain : %6d \n", iDomain);
        printf("  Name : %s \n", geo->domains[iDomain]->name);
        printf("  Number of elements : %6d\n", geo->domains[iDomain]->nElem);

        for(int i = 0; i < geo->domains[iDomain]->nElem; i++) {
            printf("%6d ", geo->domains[iDomain]->elem[i]);
            if ( (i+1) != geo->domains[iDomain]->nElem && (i+1) % 10 == 0) {
                printf("\n");
            }
        }
        printf("\n");
    }
}

void geoNodesPrint(femGeo* geo){
    femNodes* nodes = geo->nodes;
    if(nodes){
        printf("Nodes: %d\n", nodes->nNodes);
        for(int i = 0; i < nodes->nNodes; i++){
            printf("%6d : %6d : %le %le\n", i, nodes->number[i], nodes->X[i], nodes->Y[i]);
        }
    }  
}

int geoGetDomain(femGeo* geo, char* name){
    int index = -1;
    int nDomains = geo->nDomains;
    for (int i = 0; i < nDomains; i++) {
        if (strcmp(geo->domains[i]->name, name) == 0) {
            index = i;
        }
    }
    return index;
}

void geoSetDomain(femGeo* geo, int iDomain, char* name){
    if (iDomain < 0 || iDomain >= geo->nDomains) {
        fprintf(stderr, "Invalid domain index: %d\n", iDomain);
        return;
    }
    if (geoGetDomain(geo, name) != -1) {
        fprintf(stderr, "Domain name already exists: %s\n", name);
        return;
    }
    sprintf(geo->domains[iDomain]->name, "%s", name);
}

double* pos;
int compare(const void *N1, const void *N2)
{
    int* i1 = (int*)N1;
    int* i2 = (int*)N2;
    double diff = pos[*i1] - pos[*i2];
    return (diff < 0) - (diff > 0);
}

void femMeshRenumber(femMesh *theMesh, femRenumberType renumType)
{
    int i, *inverse;
    inverse = (int *) malloc(sizeof(int) * theMesh->nodes->nNodes);
    for (i = 0; i < theMesh->nodes->nNodes; i++){
                inverse[i] = i;
    }

    switch (renumType) {
        case NONE :
            break;
        case X :
            pos = theMesh->nodes->X;
            qsort(inverse, theMesh->nodes->nNodes, sizeof(int), compare);
            break;
        case Y :
            pos = theMesh->nodes->Y;
            qsort(inverse, theMesh->nodes->nNodes, sizeof(int), compare);
            break;
        case RCMK :
            //TODO
            break;
        default:
            fprintf(stderr,"Unknown renumbering type\n");
    }
                
    for (i = 0; i < theMesh->nodes->nNodes; i++){
        theMesh->nodes->number[inverse[i]] = i;
    }
    free(inverse);
}

int femComputeBand(femGeo* geo){
    femMesh* mesh = geo->mesh;

    int iElem, j, maxNum, minNum, nodeNum, elemNum, band;
    band = 0;

    for (iElem = 0; iElem < mesh->nElem; iElem++)
    {   
        maxNum = INT_MIN;
        minNum = INT_MAX;

        for (j = 0; j < mesh->nLocalNode; j++)
        {
            elemNum = mesh->elem[iElem * mesh->nLocalNode + j];
            nodeNum = mesh->nodes->number[elemNum];

            maxNum = (nodeNum > maxNum) ? nodeNum : maxNum;
            minNum = (nodeNum < minNum) ? nodeNum : minNum;
        }
        if (band < maxNum - minNum) { band = maxNum - minNum; }
    }
    return 2 * (band + 1);
}
static const double _gaussQuad4Xsi[4]    = {-0.577350269189626,-0.577350269189626, 0.577350269189626, 0.577350269189626};
static const double _gaussQuad4Eta[4]    = { 0.577350269189626,-0.577350269189626,-0.577350269189626, 0.577350269189626};
static const double _gaussQuad4Weight[4] = { 1.000000000000000, 1.000000000000000, 1.000000000000000, 1.000000000000000};
static const double _gaussTri3Xsi[3]     = { 0.166666666666667, 0.666666666666667, 0.166666666666667};
static const double _gaussTri3Eta[3]     = { 0.166666666666667, 0.166666666666667, 0.666666666666667};
static const double _gaussTri3Weight[3]  = { 0.166666666666667, 0.166666666666667, 0.166666666666667};
static const double _gaussEdge2Xsi[2]    = { 0.577350269189626,-0.577350269189626};
static const double _gaussEdge2Weight[2] = { 1.000000000000000, 1.000000000000000};

femIntegration* femIntegrationCreate(int n, femElementType type){
    femIntegration* rule = (femIntegration*)malloc(sizeof(femIntegration));
    if (rule == NULL) {
        fprintf(stderr, "Memory allocation failed for femIntegration structure.\n");
        return NULL;
    }

    if (type == FEM_EDGE && n == 2 ){
        rule->n = n;
        rule->xsi = _gaussEdge2Xsi;
        rule->eta = NULL;
        rule->weight = _gaussEdge2Weight;
    } else if (type == FEM_TRIANGLE && n == 3){
        rule->n = n;
        rule->xsi = _gaussTri3Xsi;
        rule->eta = _gaussTri3Eta;
        rule->weight = _gaussTri3Weight;
    } else if (type == FEM_QUAD && n == 4){
        rule->n = n;
        rule->xsi = _gaussQuad4Xsi;
        rule->eta = _gaussQuad4Eta;
        rule->weight = _gaussQuad4Weight;
    } else {
        fprintf(stderr, "Invalid integration rule for element type.\n");
        free(rule);
        return NULL;
    }
    return rule;
}

void femIntegrationFree(femIntegration* rule){
    if (rule != NULL) {
        free(rule);
    }
}

// Nodes for parent edge
void _e1c0_x(double *xsi) 
{
    xsi[0] = -1.0;  
    xsi[1] =  1.0;  
}

// Basis functions for parent edge
void _e1c0_phi(double xsi,  double *phi)
{
    phi[0] = (1 - xsi) / 2.0;  
    phi[1] = (1 + xsi) / 2.0;
}

// Derivative of basis functions for parent edge
void _e1c0_dphidx(double xsi, double *dphidxsi)
{
    dphidxsi[0] = -0.5;  
    dphidxsi[1] =  0.5;
}

// Nodes for parent triangle
void _p1c0_x(double *xsi, double *eta) 
{
    xsi[0] =  0.0;  eta[0] =  0.0;
    xsi[1] =  1.0;  eta[1] =  0.0;
    xsi[2] =  0.0;  eta[2] =  1.0;
}

// Basis functions for parent triangle
void _p1c0_phi(double xsi, double eta, double *phi)
{
    phi[0] = 1 - xsi - eta;  
    phi[1] = xsi;
    phi[2] = eta;
}

// Derivative of basis functions for parent triangle
void _p1c0_dphidx(double xsi, double eta, double *dphidxsi, double *dphideta)
{
    dphidxsi[0] = -1.0;  
    dphidxsi[1] =  1.0;
    dphidxsi[2] =  0.0;
    dphideta[0] = -1.0;  
    dphideta[1] =  0.0;
    dphideta[2] =  1.0;
}

// Nodes for parent quad
void _q1c0_x(double *xsi, double *eta) 
{
    xsi[0] =  1.0;  eta[0] =  1.0;
    xsi[1] = -1.0;  eta[1] =  1.0;
    xsi[2] = -1.0;  eta[2] = -1.0;
    xsi[3] =  1.0;  eta[3] = -1.0;
}

// Basis functions for parent quad
void _q1c0_phi(double xsi, double eta, double *phi)
{
    phi[0] = (1.0 + xsi) * (1.0 + eta) / 4.0;  
    phi[1] = (1.0 - xsi) * (1.0 + eta) / 4.0;
    phi[2] = (1.0 - xsi) * (1.0 - eta) / 4.0;
    phi[3] = (1.0 + xsi) * (1.0 - eta) / 4.0;
}

// Derivative of basis functions for parent quad
void _q1c0_dphidx(double xsi, double eta, double *dphidxsi, double *dphideta)
{
    dphidxsi[0] =   (1.0 + eta) / 4.0;  
    dphidxsi[1] = - (1.0 + eta) / 4.0;
    dphidxsi[2] = - (1.0 - eta) / 4.0;
    dphidxsi[3] =   (1.0 - eta) / 4.0;
    dphideta[0] =   (1.0 + xsi) / 4.0;  
    dphideta[1] =   (1.0 - xsi) / 4.0;
    dphideta[2] = - (1.0 - xsi) / 4.0;
    dphideta[3] = - (1.0 + xsi) / 4.0;

}

femDiscrete* femDiscreteCreate(int n, femElementType type){
    femDiscrete* discrete = (femDiscrete*) malloc(sizeof(femDiscrete));
    if (discrete == NULL) {
        fprintf(stderr, "Memory allocation failed for femDiscrete structure.\n");
        return NULL;
    }
    discrete->n = 0;
    discrete->type = type;

    discrete->x = NULL;
    discrete->phi = NULL;
    discrete->dphidx = NULL;
    discrete->x2 = NULL;
    discrete->phi2 = NULL;
    discrete->dphi2dx = NULL;

    if (type == FEM_EDGE && n == 2){
        discrete->n = n;
        discrete->x = _e1c0_x;
        discrete->phi = _e1c0_phi;
        discrete->dphidx = _e1c0_dphidx;
    } else if (type == FEM_TRIANGLE && n == 3){
        discrete->n = n;
        discrete->x2 = _p1c0_x;
        discrete->phi2 = _p1c0_phi;
        discrete->dphi2dx = _p1c0_dphidx;
    } else if (type == FEM_QUAD && n == 4){
        discrete->n = n;
        discrete->x2 = _q1c0_x;
        discrete->phi2 = _q1c0_phi;
        discrete->dphi2dx = _q1c0_dphidx;
    } else {
        fprintf(stderr, "Invalid discrete rule for element type.\n");
        free(discrete);
        return NULL;
    }
    return discrete;
}

void femDiscreteFree(femDiscrete* discrete){
    if (discrete != NULL) {
        free(discrete);
    }
}

// Functions to compute different values from the discrete spaces
void femDiscreteXsi2(femDiscrete* mySpace, double *xsi, double *eta)
{
    mySpace->x2(xsi,eta);
}

void femDiscretePhi2(femDiscrete* mySpace, double xsi, double eta, double *phi)
{
    mySpace->phi2(xsi,eta,phi);
}

void femDiscreteDphi2(femDiscrete* mySpace, double xsi, double eta, double *dphidxsi, double *dphideta)
{
    mySpace->dphi2dx(xsi,eta,dphidxsi,dphideta);
}

void femDiscreteXsi(femDiscrete* mySpace, double *xsi)
{
    mySpace->x(xsi);
}

void femDiscretePhi(femDiscrete* mySpace, double xsi, double *phi)
{
    mySpace->phi(xsi,phi);
}

void femDiscreteDphi(femDiscrete* mySpace, double xsi, double *dphidxsi)
{
    mySpace->dphidx(xsi,dphidxsi);
}

void femDiscretePrint(femDiscrete *mySpace)
{
    int i,j;
    int n = mySpace->n;
    double xsi[4], eta[4], phi[4], dphidxsi[4], dphideta[4];

    if (mySpace->type == FEM_EDGE) {
        femDiscreteXsi(mySpace,xsi);
        for (i=0; i < n; i++) {           
            femDiscretePhi(mySpace,xsi[i],phi);
            femDiscreteDphi(mySpace,xsi[i],dphidxsi);
            for (j=0; j < n; j++)  {
                printf("(xsi=%+.1f) : ",xsi[i]);
                printf(" phi(%d)=%+.1f",j,phi[j]);  
                printf("   dphidxsi(%d)=%+.1f \n",j,dphidxsi[j]); }
            printf(" \n"); }}
    
    if (mySpace->type == FEM_QUAD || mySpace->type == FEM_TRIANGLE) {
        femDiscreteXsi2(mySpace, xsi, eta);
        for (i = 0; i < n; i++)  {    
            femDiscretePhi2(mySpace, xsi[i], eta[i], phi);
            femDiscreteDphi2(mySpace, xsi[i], eta[i], dphidxsi, dphideta);
            for (j = 0; j < n; j++) {  
                printf("(xsi=%+.1f,eta=%+.1f) : ", xsi[i], eta[i]);  
                printf(" phi(%d)=%+.1f", j, phi[j]);
                printf("   dphidxsi(%d)=%+.1f", j, dphidxsi[j]);
                printf("   dphideta(%d)=%+.1f \n", j, dphideta[j]); }
            printf(" \n"); }}   
}
// Solver functions
femSolver* femSolverFullCreate(int size){
    femSolver* solver = (femSolver*) malloc(sizeof(femSolver));
    solver->type = SOLVER_FULL;
    solver->solver = femFullSystemCreate(size);
    return solver;
}

femSolver* femSolverBandCreate(int size, int band){
    femSolver* solver = (femSolver*) femSolverCreate(size);
    solver->type = SOLVER_BAND;
    solver->solver = femBandSystemCreate(band, size);
    return solver;
}

femSolver* femSolverCreate(int size){
    femSolver* solver = (femSolver*) malloc(sizeof(femSolver));
    if (solver == NULL) {
        fprintf(stderr, "Memory allocation failed for femSolver structure.\n");
        return NULL;
    }
    solver->size = size;
    return solver;
}

// TODO : SOLVER_GC
void femSolverFree(femSolver* solver){
    switch (solver->type) {
    case SOLVER_FULL:
        femFullSystemFree((femFullSystem*)solver->solver);
        break;
    case SOLVER_BAND:
        femBandSystemFree((femBandSystem*)solver->solver);
        break;
    default:
        break;
    }
}

double* femSolverEliminate(femSolver* solver){
    switch (solver->type) {
    case SOLVER_FULL:
        return femFullSystemEliminate((femFullSystem*)solver->solver, solver->size);
    case SOLVER_BAND:
        return femBandSystemEliminate((femBandSystem*)solver->solver, solver->size);
    default:
        fprintf(stderr, "Unknown solver type.\n");
        return NULL;
    }
}


void femSolverAssemble(femSolver *solver, femProblem *problem, int *mapX, int *mapY, double *phi, double *dphidx, double *dphidy, double weightedJac, double xLoc, int nLoc){
    switch(solver->type) {
        case SOLVER_FULL:
            femFullSystemAssemble((femFullSystem*)solver->solver, problem, mapX, mapY, phi, dphidx, dphidy, weightedJac, xLoc, nLoc);
            break;
        case SOLVER_BAND:
            femBandSystemAssemble((femBandSystem*)solver->solver, problem, mapX, mapY, phi, dphidx, dphidy, weightedJac, xLoc, nLoc);
            break;
        default:
            fprintf(stderr, "Unknown solver type.\n");
            break;
    }
}
    // Full system functions
void femFullSystemAlloc(femFullSystem* system, int size){
    int i;
    double* elem = (double*) malloc(sizeof(double) * size * (size +1));
    system->A = (double**) malloc(sizeof(double*) * size);
    system->B = elem;
    system->A[0] = elem + size;
    system->size = size;
    for (i = 1; i < size; i++) {
        system->A[i] = system->A[i - 1] + size;
    }
}

void femFullSystemInit(femFullSystem* system){
    int i, size = system->size;
    for ( i = 0; i < size*(size+1); i++){
        system->B[i] = 0.0;
    }
}

femFullSystem* femFullSystemCreate(int size){
    femFullSystem* system = (femFullSystem*) malloc(sizeof(femFullSystem));
    if (system == NULL) {
        fprintf(stderr, "Memory allocation failed for femFullSystem structure.\n");
        return NULL;
    }
    
    femFullSystemAlloc(system, size);
    femFullSystemInit(system);
    return system;
}

void femFullSystemFree(femFullSystem* system){
    if (system != NULL) {
        free(system->A);
        free(system->B);
        free(system);
    }
}

void femFullSystemPrint(femFullSystem* system){
    double **A, *B;
    int i,j,size;
    
    A = system->A;
    B = system->B;
    size = system->size;

    for(i = 0; i< size; i++){
        for(j = 0; j < size; j++){
            if (A[i][j] == 0) printf("         ");
            else printf("%+.2f ", A[i][j]);
        }
        printf(" : %+.1e \n", B[i]);
    }
}

double* femFullSystemEliminate(femFullSystem* system, int size){
    double** A;
    double* B;
    double factor;
    int i,j,k;

    A = system->A;
    B = system->B;

    // Gaussian elimination
    for (k=0; k < size; k++){
        if (fabs(A[k][k] <= 1e-16)){
            printf("Pivot is %e at index %d\nCannot eliminate\n", A[k][k], k);
            return NULL;
        }
        for (i=k+1; i<size; i++){
            factor = A[i][k] / A[k][k];
            for (j=k+1; j<size; j++){
                A[i][j] -= factor * A[k][j];
            }
            B[i] -= factor * B[k];
        }
    }

    // Back substitution
    for(i = size-1; i>= 0; i--){
        factor = 0;
        for (j=i+1; j<size; j++){
            factor += A[i][j] * B[j];
        }
        B[i] = (B[i] - factor) / A[i][i];      
    }

    return(system->B);
}

void femFullSystemAssemble(femFullSystem* system, femProblem* problem, int* mapX, int* mapY, double* phi, double* dphidx, double* dphidy, double xLoc, double wJac, double nLoc){
    double** A = system->A;
    double* B = system->B;
    double a = problem->A;
    double b = problem->B;
    double c = problem->C;
    double rho = problem->rho;
    double g = problem->g;

    if (problem->planarStrainStress == PLANAR_STRAIN || problem->planarStrainStress == PLANAR_STRESS){
        for(int i = 0; i < nLoc; i++){
            for(int j = 0; j < nLoc; j++){
                A[mapX[i]][mapX[j]] += (a * dphidx[i] * dphidx[j] + b * dphidy[i] * dphidy[j]) * wJac;
                A[mapX[i]][mapY[j]] += (c * dphidx[i] * dphidy[j] + b * dphidy[i] * dphidx[j]) * wJac;
                A[mapY[i]][mapX[j]] += (c * dphidx[i] * dphidy[j] + b * dphidy[i] * dphidx[j]) * wJac;
                A[mapY[i]][mapY[j]] += (a * dphidy[i] * dphidy[j] + b * dphidx[i] * dphidx[j]) * wJac;
            }
            B[mapX[i]] -= phi[i] * g * rho * wJac;
        }
    }
    else if (problem->planarStrainStress == AXISYM)
    {
        for (int i = 0; i < nLoc; i++)
        {
            for (int j = 0; j < nLoc; j++)
            {
                A[mapX[i]][mapX[j]] += (dphidx[i] * a * xLoc * dphidx[j] + dphidy[i] * c * xLoc * dphidy[j] + dphidx[i] * b * phi[j] + phi[i] * (b * dphidx[j] + a * phi[j] / xLoc)) * wJac;
                A[mapX[i]][mapY[j]] += (dphidx[i] * b * xLoc * dphidy[j] + dphidy[i] * c * xLoc * dphidx[j] + phi[i] * b * dphidy[j]) * wJac;
                A[mapY[i]][mapX[j]] += (dphidy[i] * b * xLoc * dphidx[j] + dphidx[i] * c * xLoc * dphidy[j] + dphidy[i] * b * phi[j]) * wJac;
                A[mapY[i]][mapY[j]] += (dphidy[i] * a * xLoc * dphidy[j] + dphidx[i] * c * xLoc * dphidx[j]) * wJac;
            }
            B[mapX[i]] -= phi[i] * xLoc * g * rho * wJac;
        }
    } else {
        fprintf(stderr, "Unknown planar strain/stress type.\n");
        return;
    }
}

void femFullSystemConstrain(femFullSystem* system, int node, double value){
    double** A;
    double* B;
    int i, size;

    A = system->A;
    B = system->B;
    size = system->size;
    if (node < 0 || node >= size) {
        fprintf(stderr, "Invalid node index: %d\n", node);
        return;
    }

    for(i = 0; i < size; i++){
        B[i] -= A[i][node] * value;
        A[i][node] = 0.0;
    }

    for(i = 0; i < size; i++){
        A[node][i] = 0.0;
    }

    A[node][node] = 1.0;
    B[node] = value;
}

// Band system functions
femBandSystem* femBandSystemCreate(int band, int size){
    femBandSystem* system = (femBandSystem*) malloc(sizeof(femBandSystem));
    if (system == NULL) {
        fprintf(stderr, "Memory allocation failed for femBandSystem structure.\n");
        return NULL;
    }
    femBandSystemAlloc(system, size, band);
    femBandSystemInit(system, size);
    return system;
}

void femBandSystemAlloc(femBandSystem* system, int size, int band){
    system->B = (double*) malloc(sizeof(double) * size * (band + 1));
    if (system->B == NULL){fprintf(stderr, "Memory allocation failed for band system B.\n"); return;}
    system->A = (double**) malloc(sizeof(double*) * size);
    if (system->A == NULL){fprintf(stderr, "Memory allocation failed for band system A.\n"); return;}
    system->band = band;
    system->A[0] = system->B + size;
    for (int i = 1; i < size; i++){system->A[i] = system->A[i-1] + band - 1;}

}

void femBandSystemInit(femBandSystem* system, int size){
    int i;
    for (i = 0; i < size*(system->band + 1); i++){
        system->B[i] = 0.0;
    }
}

void femBandSystemFree(femBandSystem* system){
    if (system != NULL) {
        free(system->A);
        free(system->B);
        free(system);
    }
}

void femBandSystemPrint(femBandSystem* system, int size){
    double** A = system->A;
    double* B = system->B;
    int band = system->band;
    for (int i = 0; i < size;i++){
        for (int j = 0; j < band; j++){
            if (A[i][j] == 0) printf("         ");
            else printf("%+.2f ", A[i][j]);
        }
        printf(" : %+.1e \n", B[i]);
    }
}

void femBandSystemAssemble(femBandSystem* system, femProblem* problem, int* mapX, int* mapY, double* phi,
                            double* dphidx, double* dphidy, double xLoc, double wJac, double nLoc){
    double** A = system->A;
    double* B = system->B;
    double a = problem->A;
    double b = problem->B;
    double c = problem->C;
    double rho = problem->rho;
    double g = problem->g;
    int band = system->band;
    int i, j;

    if (problem->planarStrainStress == PLANAR_STRAIN || problem->planarStrainStress == PLANAR_STRESS){
        for (i = 0; i < nLoc; i++){
            for (j = 0; j < nLoc; j++){
                if (inBand(band, mapX[i], mapX[j])){
                    A[mapX[i]][mapX[j]] += (a * dphidx[i] * dphidx[j] + b * dphidy[i] * dphidy[j]) * wJac;
                }
                if (inBand(band, mapX[i], mapY[j])){
                    A[mapX[i]][mapY[j]] += (c * dphidx[i] * dphidy[j] + b * dphidy[i] * dphidx[j]) * wJac;
                }
                if (inBand(band, mapY[i], mapX[j])){
                    A[mapY[i]][mapX[j]] += (c * dphidx[i] * dphidy[j] + b * dphidy[i] * dphidx[j]) * wJac;
                }
                if (inBand(band, mapY[i], mapY[j])){
                    A[mapY[i]][mapY[j]] += (a * dphidy[i] * dphidy[j] + b * dphidx[i] * dphidx[j]) * wJac;
                }
            }
            B[mapX[i]] -= phi[i] * g * rho * wJac;
        }
    } else if (problem->planarStrainStress == AXISYM){
        for (i = 0; i < nLoc; i++) {
            for (j = 0; j < nLoc; j++) {
                if (inBand(band, mapX[i], mapX[j])){
                    A[mapX[i]][mapX[j]] += (dphidx[i] * a * xLoc * dphidx[j] + dphidy[i] * c * xLoc * dphidy[j] + dphidx[i] * b * phi[j] + phi[i] * (b * dphidx[j] + a * phi[j] / xLoc)) * wJac;
                }
                if (inBand(band, mapX[i], mapY[j])){
                    A[mapX[i]][mapY[j]] += (dphidx[i] * b * xLoc * dphidy[j] + dphidy[i] * c * xLoc * dphidx[j] + phi[i] * b * dphidy[j]) * wJac;
                }
                if (inBand(band, mapY[i], mapX[j])){
                    A[mapY[i]][mapX[j]] += (dphidy[i] * b * xLoc * dphidx[j] + dphidx[i] * c * xLoc * dphidy[j] + dphidy[i] * b * phi[j]) * wJac;
                }
                if (inBand(band, mapY[i], mapY[j])){
                    A[mapY[i]][mapY[j]] += (dphidy[i] * a * xLoc * dphidy[j] + dphidx[i] * c * xLoc * dphidx[j]) * wJac;
                }
            }
            B[mapX[i]] -= phi[i] * xLoc * g * rho * wJac;
        }
    }
}
// OK
double* femBandSystemEliminate(femBandSystem* system, int size){
    double **A, *B, factor;
    int i, j, k, jend, band;
    A = system->A;
    B = system->B;
    band = system->band;

    /* Gauss elimination */
    for (k = 0; k < size; k++)
    {
        if (fabs(A[k][k]) <= 1e-16) { 
            fprintf(stderr, "Pivot is %e at index %d\nCannot eliminate\n", A[k][k], k);
            return NULL; 
        }
        jend = (k + band < size) ? k + band : size;
        for (i = k + 1; i < jend; i++)
        {
            factor = A[k][i] / A[k][k];
            for (j = i ; j < jend; j++) { A[i][j] -= factor * A[k][j]; }
            B[i] -= factor * B[k];
        }    
    }
    
    /* Back-substitution */
    for (i = size - 1; i >= 0 ; i--)
    {
        factor = 0;
        jend = (i + band < size) ? i + band : size;
        for (j = i + 1 ; j < jend; j++) { factor += A[i][j] * B[j]; }
        B[i] = ( B[i] - factor) / A[i][i];
    }
    return B;
}

int inBand(int band, int row, int col){
    return (col >= row && col < row + band);
}
// Linear elasticity functions
femProblem* femElasticityCreate(femGeo* geo, double E, double nu, double rho, double g, double T, femElasticCase iCase) {
    femProblem* problem = (femProblem*)malloc(sizeof(femProblem));

    if (problem == NULL) {
        fprintf(stderr, "Memory allocation failed for femProblem structure.\n");
        return NULL;
    }

    problem->E = E;
    problem->nu = nu;
    problem->rho = rho;
    problem->g = g;
    problem->T = T;

    if (iCase == PLANAR_STRESS){
        problem->A = E / (1 - nu * nu);
        problem->B = nu * E / (1 - nu * nu);
        problem->C = E / (2 * (1 + nu));
    } else if (iCase == PLANAR_STRAIN){
        problem->A = E / ((1 + nu) * (1 - 2 * nu));
        problem->B = nu * E / ((1 + nu) * (1 - 2 * nu));
        problem->C = E / (2 * (1 + nu));
    } else {
        fprintf(stderr, "Invalid case for linear elasticity.\n");
        free(problem);
        return NULL;
    }

    problem->planarStrainStress = iCase;
    problem->nBoundaryConditions = 0;
    problem->conditions = NULL;
    int size = 2 * geo->nodes->nNodes;
    
    problem->constrainedNodes = (int*)malloc(size * sizeof(int));
    if (problem->constrainedNodes == NULL) {
        fprintf(stderr, "Memory allocation failed for constrained nodes.\n");
        free(problem);
        return NULL;
    }

    problem->soluce = (double*)malloc(size * sizeof(double));
    if (problem->soluce == NULL) {
        fprintf(stderr, "Memory allocation failed for solution vector.\n");
        free(problem->constrainedNodes);
        free(problem);
        return NULL;
    }

    problem->residuals = (double*)malloc(size * sizeof(double));
    if (problem->residuals == NULL) {
        fprintf(stderr, "Memory allocation failed for residuals vector.\n");
        free(problem->soluce);
        free(problem->constrainedNodes);
        free(problem);
        return NULL;
    }

    for(int i = 0; i<size; i++){
        problem->constrainedNodes[i] = -1.0;
        problem->soluce[i] = 0.0;
        problem->residuals[i] = 0.0;
    }

    problem->geometry = geo;
    if (geo->mesh->nLocalNode == 3) {
        problem->space = femDiscreteCreate(3, FEM_TRIANGLE);
        problem->rule = femIntegrationCreate(3, FEM_TRIANGLE);
    } else if (geo->mesh->nLocalNode == 4) {
        problem->space = femDiscreteCreate(4, FEM_QUAD);
        problem->rule = femIntegrationCreate(4, FEM_QUAD);
    }
    problem->spaceEdge = femDiscreteCreate(2, FEM_EDGE);
    problem->ruleEdge = femIntegrationCreate(2, FEM_EDGE);

    
    if (problem->solver->type == SOLVER_FULL) {
        problem->solver->solver = (femFullSystem*) femFullSystemCreate(size);
    } else if (problem->solver->type == SOLVER_BAND) {
        femRenumberType renumType = problem ->renumberType;
        int band = femComputeBand(geo);
        problem->solver->solver = (femBandSystem*) femBandSystemCreate(band, size);
        femMesh* theMesh = geo->mesh;
        femMeshRenumber(theMesh, renumType);
        int band_renum = femComputeBand(geo); //TO DO 

        if (band_renum != band) {
            //TO DO
        }
    }else {
        fprintf(stderr, "Unknown solver type.\n");
        free(problem->soluce);
        free(problem->constrainedNodes);
        free(problem->residuals);
        free(problem);
        return NULL;
    }  

    return problem;
}

femProblem* femElasticityRead(femGeo* geo, const char* problemPath, femSolverType solverType, femRenumberType renumberType){
    FILE* file = fopen(problemPath, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", problemPath);
        return NULL;
    }

    femElasticCase iCase;
    double E, nu, rho, g, T;

    fscanf(file, "Problem type    : %u\n", &iCase);
    fscanf(file, "E : %le\n", &E);
    fscanf(file, "nu : %le\n", &nu);
    fscanf(file, "rho : %le\n", &rho);
    fscanf(file, "g : %le\n", &g);
    fscanf(file, "T : %le\n", &T);

    femProblem* problem = femElasticityCreate(geo, E, nu, rho, g, T, iCase);
    int nCond;
    fscanf(file, "Conditions : %d\n", &nCond);
    for (int i = 0; i < nCond; i++){
        char* condType = (char*)malloc(16 * sizeof(char));
        char* domainName = (char*)malloc(MAXNAME * sizeof(char));
        double value;
        fscanf(file, "Type : %s\n", condType);
        fscanf(file, "Domaine : %s\n", domainName);
        if(geoGetDomain(geo, domainName) == -1){
            fprintf(stderr, "Domain not found: %s\n", domainName);
            free(condType);
            free(domainName);
            break;
        }
        fscanf(file, "Valeur : %le\n", &value);
        if (strcmp(condType, "DIRICHLET_X") == 0) {
            femElasticityAddBoundaryCondition(problem, domainName, DIRICHLET_X, value);
        } else if (strcmp(condType, "DIRICHLET_Y") == 0) {
            femElasticityAddBoundaryCondition(problem, domainName, DIRICHLET_Y, value);
        } else if (strcmp(condType, "NEUMANN_X") == 0) {
            femElasticityAddBoundaryCondition(problem, domainName, NEUMANN_X, value);
        } else if (strcmp(condType, "NEUMANN_Y") == 0) {
            femElasticityAddBoundaryCondition(problem, domainName, NEUMANN_Y, value);
        } else {
            fprintf(stderr, "Unknown condition type: %s\n", condType);
        }
        free(condType);
        free(domainName);
    }
    int size = 2 * geo->nodes->nNodes;
    if (SOLVER_FULL == solverType) {
        problem->solver = femSolverFullCreate(size);
    } else if (SOLVER_BAND == solverType) {
        int band = femComputeBand(geo);
        problem->solver = femSolverBandCreate(size, band);
    } else {
        fprintf(stderr, "Unknown solver type: %d\n", solverType);
        fclose(file);
        free(problem);
        return NULL;
    }
    problem->solverType = solverType;
    problem->renumberType = renumberType;
    problem->planarStrainStress = iCase;

    if (geo->mesh->nLocalNode == 3) {
        problem->space = femDiscreteCreate(3, FEM_TRIANGLE);
        problem->rule = femIntegrationCreate(3, FEM_TRIANGLE);
    } else if (geo->mesh->nLocalNode == 4) {
        problem->space = femDiscreteCreate(4, FEM_QUAD);
        problem->rule = femIntegrationCreate(4, FEM_QUAD);
    }
    problem->spaceEdge = femDiscreteCreate(2, FEM_EDGE);
    problem->ruleEdge = femIntegrationCreate(2, FEM_EDGE);

    fclose(file);
    return problem;
}

void femElasticityPrint(femProblem* problem) {
    printf("\n\n ======================================================================================= \n\n");
    printf(" Linear elasticity problem \n");
    printf("   Young modulus   E   = %14.7e [N/m2]\n",problem->E);
    printf("   Poisson's ratio nu  = %14.7e [-]\n",problem->nu);
    printf("   Density         rho = %14.7e [kg/m3]\n",problem->rho);
    printf("   Gravity         g   = %14.7e [m/s2]\n",problem->g);
    printf("   String tension  T   = %14.7e [N/m2]\n",problem->T);
    
    if (problem->planarStrainStress == PLANAR_STRAIN)  printf("   Planar strains formulation \n");
    if (problem->planarStrainStress == PLANAR_STRESS)  printf("   Planar stresses formulation \n");
    if (problem->planarStrainStress == AXISYM)         printf("   Axisymmetric formulation \n");

    printf("   Boundary conditions : \n");
    for(int i=0; i < problem->nBoundaryConditions; i++) {
          femBoundaryCondition *theCondition = problem->conditions[i];
          double value = theCondition->value;
          printf("  %20s :",theCondition->domain->name);
          if (theCondition->type==DIRICHLET_X)  printf(" imposing %9.2e as the horizontal displacement  \n",value);
          if (theCondition->type==DIRICHLET_Y)  printf(" imposing %9.2e as the vertical displacement  \n",value); 
          if (theCondition->type==NEUMANN_X)    printf(" imposing %9.2e as the horizontal force density \n",value); 
          if (theCondition->type==NEUMANN_Y)    printf(" imposing %9.2e as the vertical force density \n",value);}
    printf(" ======================================================================================= \n\n");

}

void femElasticityFullPrint(femProblem* problem){
    printf("\n\n");
    printf("Physical param : E = %le, nu = %le, rho = %le, g = %le, T = %le\n", problem->E, problem->nu, problem->rho, problem->g, problem->T);
    printf("A = %le, B = %le, C = %le\n", problem->A, problem->B, problem->C);
    printf("planarStrainStress = %d\n", problem->planarStrainStress);
    printf("nBoundaryConditions = %d\n", problem->nBoundaryConditions);
    for (int i = 0; i < problem->nBoundaryConditions; i++) {
        femBoundaryCondition* condition = problem->conditions[i];
        printf("Condition %d: domain = %s, type = %d, value = %le\n", i, condition->domain->name, condition->type, condition->value);
    }
    printf("Constrained Nodes :\n");
    for (int i=0; i<problem->geometry->nodes->nNodes; i++) {
        if (problem->constrainedNodes[2*i+0] != -1) {
            printf("  %d : %d \n",i,problem->constrainedNodes[2*i+0]); }
        if (problem->constrainedNodes[2*i+1] != -1) {
            printf("  %d : %d \n",i,problem->constrainedNodes[2*i+1]); } }
    printf("RenumberType = %d\n", problem->renumberType);
    printf("SolverType = %d\n", problem->solverType);
    // geoPrint(problem->geometry);
}

void femElasticityAddBoundaryCondition(femProblem* problem, char* name, femBoundaryType type, double value){
    int iDomain = geoGetDomain(problem->geometry, name);
    if (iDomain == -1) {
        fprintf(stderr, "Domain not found: %s\n", name);
        return;
    }

    femBoundaryCondition* condition = (femBoundaryCondition*)malloc(sizeof(femBoundaryCondition));
    condition->domain = problem->geometry->domains[iDomain];
    condition->type = type;
    condition->value = value;
    problem->nBoundaryConditions++;

    int size = problem->nBoundaryConditions;
    if (problem->conditions == NULL) {
        problem->conditions = (femBoundaryCondition**)malloc(size * sizeof(femBoundaryCondition*));
    } else {
        problem->conditions = (femBoundaryCondition**)realloc(problem->conditions, size * sizeof(femBoundaryCondition*));
    }
    if (problem->conditions == NULL) {
        fprintf(stderr, "Memory allocation failed for boundary conditions.\n");
        free(condition);
        return;
    }
    problem->conditions[size - 1] = condition;

    int shift=-1;
    if (type == DIRICHLET_X) shift = 0;
    if (type == DIRICHLET_Y) shift = 1;
    if (shift == -1) return;

    int* elem = condition->domain->elem;
    int nElem = condition->domain->nElem;
    for (int e = 0; e < nElem; e++){
        for (int i = 0; i < 2; i++){
            int node = condition->domain->mesh->elem[2*elem[e] + i];
            problem->constrainedNodes[2*node + shift] = size - 1;
        }
    }
}

// TODO
void femElasticityAssembleElements(femProblem* problem){
    femSolver *solver = problem->solver;
    femIntegration *rule = problem->rule;
    femDiscrete *space = problem->space;
    femGeo *geo = problem->geometry;
    femNodes *theNodes = geo->nodes;
    femMesh *theMesh = geo->mesh;

    int nLocal = space->n;
    int *number = theMesh->nodes->number;    

    double xLoc, x[nLocal], y[nLocal], phi[nLocal], dphidxsi[nLocal], dphideta[nLocal], dphidx[nLocal], dphidy[nLocal];
    double xsi, eta, weight, dxdxsi, dxdeta, dydxsi, dydeta, jac, weightedJac;
    int iElem, iInteg, iEdge, i, map[nLocal], mapX[nLocal], mapY[nLocal];

    for (iElem = 0; iElem < theMesh->nElem; iElem++)
    {
        for (i = 0; i < space->n; i++)
        {
            map[i] = theMesh->elem[iElem * nLocal + i];
            x[i] = theNodes->X[map[i]];
            y[i] = theNodes->Y[map[i]];
            map[i] = number[map[i]];
            mapX[i] = 2 * map[i];
            mapY[i] = 2 * map[i] + 1;
        }

        for (iInteg = 0; iInteg < rule->n; iInteg++)
        {
            xsi    = rule->xsi[iInteg];
            eta    = rule->eta[iInteg];
            weight = rule->weight[iInteg];

            femDiscretePhi2(space, xsi, eta, phi);
            femDiscreteDphi2(space, xsi, eta, dphidxsi, dphideta);

            dxdxsi = 0.0; dydxsi = 0.0;
            dxdeta = 0.0; dydeta = 0.0;
            xLoc = 0.0;
            for (i = 0; i < space->n; i++)
            {
                dxdxsi += x[i] * dphidxsi[i];
                dxdeta += x[i] * dphideta[i];
                dydxsi += y[i] * dphidxsi[i];
                dydeta += y[i] * dphideta[i];
                xLoc   += x[i] * phi[i];
            }

            jac = dxdxsi * dydeta - dxdeta * dydxsi;
            if (jac < 0.0) { printf("Jacobian should be positive!\n"); }
            jac = fabs(jac);

            for (i = 0; i < space->n; i++)
            {
                dphidx[i] = (dphidxsi[i] * dydeta - dphideta[i] * dydxsi) / jac;
                dphidy[i] = (dphideta[i] * dxdxsi - dphidxsi[i] * dxdeta) / jac;
            }

            weightedJac = jac * weight;

            femSolverAssemble(solver, problem, mapX, mapY, phi, dphidx, dphidy, weightedJac, xLoc, space->n);
        }
    }
}

// TODO
void femElasticityAssembleNeumann(femProblem* problem){

    if (problem->solver->type == SOLVER_FULL) {
        femFullSystem  *system = (femFullSystem*) problem->solver->solver;
    } else if (problem->solver->type == SOLVER_BAND) {
        femBandSystem *system = (femBandSystem*) problem->solver->solver;
    } else {
        printf("Error: Unknown solver type.\n");
        return;
    }

    //femFullSystem  *system = problem->system;
    femIntegration *rule = problem->ruleEdge;
    femDiscrete    *space = problem->spaceEdge;
    femGeo         *geo = problem->geometry;
    femNodes       *nodes = geo->nodes;
    femMesh        *edges = geo->edges;

    int iCond, iEdge, iElem, iInteg, i, j;
    int map[2], mapU[2];
    double x[2], y[2], phi[2];
    double *B = system->B;
    int nNodes = 2;

    for (iCond = 0; iCond < problem->nBoundaryConditions; iCond++) {
        femBoundaryCondition *bc = problem->conditions[iCond];
        femBoundaryType type = bc->type;
        double imposedValue = bc->value;
        double r = 0.0;
        double shift = -1;
        int nLocal = 2;

        if (type == NEUMANN_X) {
            shift = 0.0;
        } else if (type == NEUMANN_Y) {
            shift = 1.0;
        } else {
            continue; // Skip if not a Neumann condition
        }

        for (iEdge = 0; iEdge < bc->domain->nElem; iEdge++) {
            iElem = bc->domain->elem[iEdge];
            for (i = 0; i < nLocal; i++) {
                map[i] = edges->elem[iElem * nLocal + i];
                x[i] = nodes->X[map[i]];
                y[i] = nodes->Y[map[i]];
                mapU[i] = nodes->number[map[i]];
            }

            double jac = sqrt((x[1] - x[0]) * (x[1] - x[0]) + (y[1] - y[0]) * (y[1] - y[0]))/2.0;
            for (iInteg = 0; iInteg < rule->n; iInteg++) {
                double xsi = rule->xsi[iInteg];
                //double eta = rule->eta[iInteg];
                double weight = rule->weight[iInteg];

                femDiscretePhi(space, xsi, phi);
                //femDiscretePhi2(space, xsi, eta, phi);

                if (problem->planarStrainStress == AXISYM) {
                    //TO DO
                    
                }
                for (i = 0; i < space->n; i++) {
                    B[mapU[i]] += imposedValue * phi[i] * jac * weight;
                }
            }
        }
    }
}

void  femFullSystemConstrain(femFullSystem *mySystem, int myNode, double myValue) { //repris de fem.c devoir6
    double  **A, *B;
    int     i, size;

    A    = mySystem->A;
    B    = mySystem->B;
    size = mySystem->size;

    for (i=0; i < size; i++) {
        
        B[i] = B[i] - myValue * A[i][myNode];
        A[i][myNode] = 0; 
        A[myNode][i] = 0; 
    } 

    A[myNode][myNode] = 1.0;
    B[myNode] = myValue;
}

//TO DO
void femBandSystemConstrain(femBandSystem* mySystem, int node, double value, int size) {
    double** A = mySystem->A;
    double* B = mySystem->B;
    int band = mySystem->band;
    int i, j, jend;

    if (node < 0 || node >= size) {
        fprintf(stderr, "Invalid node index: %d\n", node);
        return;
    }

    // Mettre à jour B en annulant les contributions de la colonne "node"
    for (int i = 0; i < size; i++) {
        int j = node - i;
        if (j >= -band && j <= band) {
            int k = j + band;
            B[i] -= value * A[i][k];
            A[i][k] = 0.0;
        }
    }

    // Mettre à zéro la ligne correspondante
    for (int j = -band; j <= band; j++) {
        int col = node + j;
        if (col >= 0 && col < size) {
            int k = -j + band;
            A[node][k] = 0.0;
        }
    }

    A[node][node] = 1.0;
    B[node] = value;
}


// TODO
void femElasticityApplyDirichlet(femProblem* problem){
    
    femFullSystem *system = NULL;
    if (problem->solver->type == SOLVER_FULL) {
        system = (femFullSystem*) problem->solver->solver;
    } else if (problem->solver->type == SOLVER_BAND) {
        system = (femBandSystem*) problem->solver->solver;
    } else {
        printf("Erreur : femElasticityApplyDirichlet est prévu uniquement pour SOLVER_FULL ou SOLVER_BAND\n");
    exit(1);
    }

    int *nodes = problem->constrainedNodes;
    int size = system->size;
    for (int i = 0; i < size; i++)
    {
        if (nodes[i] != -1)
        {
            double val = problem->conditions[nodes[i]]->value;
            if (problem->solver->type == SOLVER_FULL) {
                femFullSystemConstrain(system, i, val);
            } else {
                femBandSystemConstrain(system, i, val, size);
            }
                

        }
    }

}

// TODO
double* femElasticitySolve(femProblem* problem){
    femSolver* solver = problem->solver;
    femNodes* nodes = problem->geometry->nodes;
    double* soluce;

    femElasticityAssembleElements(problem); // OK
    femElasticityAssembleNeumann(problem); // TODO
    femElasticityApplyDirichlet(problem); // TODO

    soluce = femSolverEliminate(solver); // OK
    for (int i = 0; i <nodes->nNodes; i++){
        problem->soluce[2*i]= soluce[2*nodes->number[i]];
        problem->soluce[2*i+1]= soluce[2*nodes->number[i]+1];
    }
    return problem->soluce;
}

// TODO
double *femElasticityForces(femProblem *theProblem)
{
    
}

// OK
double femElasticityIntegrate(femProblem* problem, double (*f)(double x, double y)){
    femIntegration* rule = problem->rule;
    femGeo* geo = problem->geometry;
    femNodes* nodes = geo->nodes;
    femMesh* mesh = geo->mesh;
    femDiscrete* space = problem->space;

    double x[4], y[4], phi[4], dphidxsi[4], dphideta[4], dphidx[4], dphidy[4];
    int iElem, iInteg, i, map[4];
    int nLocal = mesh->nLocalNode;
    double value = 0.0;

    for (iElem = 0; iElem < mesh->nElem; iElem++) {
        for (i = 0; i < nLocal; i++){
            map[i] = mesh->elem[iElem * nLocal + i];
            x[i] = nodes->X[map[i]];
            y[i] = nodes->Y[map[i]];
        }

        for (iInteg = 0; iInteg < rule->n; iInteg++){
            double xsi = rule->xsi[iInteg];
            double eta = rule->eta[iInteg];
            double weight = rule->weight[iInteg];

            // Compute the shape functions and their derivatives
            femDiscretePhi2(space, xsi, eta, phi);
            femDiscreteDphi2(space, xsi, eta, dphidxsi, dphideta);

            double dxdxsi = 0.0;
            double dxdeta = 0.0;
            double dydxsi = 0.0;
            double dydeta = 0.0;
            for (i = 0; i < space->n; i++){
                dxdxsi += x[i] * dphidxsi[i];
                dxdeta += x[i] * dphideta[i];
                dydxsi += y[i] * dphidxsi[i];
                dydeta += y[i] * dphideta[i];
            }
            double jac = fabs(dxdxsi * dydeta - dxdeta * dydxsi);
            for (i = 0; i < space->n; i++){
                value+= phi[i] * f(x[i], y[i]) * jac * weight;
            }
        }
    }
    return value;
}

// OK
void femElasticityFree(femProblem* problem){
    if (problem != NULL){
        if (problem->geometry != NULL) {
            geoFree(problem->geometry);
        }
        if (problem->space != NULL) {
            femDiscreteFree(problem->space);
        }
        if (problem->rule != NULL) {
            femIntegrationFree(problem->rule);
        }
        if (problem->spaceEdge != NULL) {
            femDiscreteFree(problem->spaceEdge);
        }
        if (problem->ruleEdge != NULL) {
            femIntegrationFree(problem->ruleEdge);
        }
        if (problem->solver != NULL) {
            free(problem->solver->solver);
        }
        if (problem->constrainedNodes != NULL) {
            free(problem->constrainedNodes);
        }
        if (problem->soluce != NULL) {
            free(problem->soluce);
        }
        if (problem->residuals != NULL) {
            free(problem->residuals);
        }
        if (problem->conditions != NULL) {
            for(int i = 0; i < problem->nBoundaryConditions; i++){
                free(problem->conditions[i]);
            }
            free(problem->conditions);
        }
        free(problem);
    }
}


////// Solveur GC ////

//TO DO 

////////////////////////

void geoMeshGenerate() {
    femGeo* theGeometry = geoGetGeometry();
    int ierr;

    double c = 0.15;
    double h = 0.1;
    double f = 0.025;
    double l = 0.7;
    double L = 0.005;
    double pi = 3.14159265358979323846;
    double xc = 0.1203;

    double c1 = (c*c + l*l - (l*h) + ((h*h)/4.0))/(2.0*c);
    double c2 = (c*c + l*l - (l*h) - f*f + ((h*h)/4.0))/(2.0*(c-f));
    double beta = pi - acos((c2-xc)/(c2-f));
    double alpha = L/(c2-f);


    int A = gmshModelOccAddPoint(0,0,0,0.0,-1,&ierr); ErrorGmsh(ierr);
    int B = gmshModelOccAddPoint(0,(h/2),0,0.0,-1,&ierr); ErrorGmsh(ierr);
    int C = gmshModelOccAddPoint(c,l,0,0.0,-1,&ierr); ErrorGmsh(ierr);
    int D = gmshModelOccAddPoint(c2+(c2-f)*cos(beta-(alpha/2)),(h/2)+(c2-f)*sin(beta-(alpha/2)),0,0.0,-1,&ierr); ErrorGmsh(ierr);
    int E = gmshModelOccAddPoint(c2+(c2-f)*cos(beta+(alpha/2)),(h/2)+(c2-f)*sin(beta+(alpha/2)),0,0.0,-1,&ierr); ErrorGmsh(ierr);
    int F = gmshModelOccAddPoint(f,(h/2),0,0.0,-1,&ierr); ErrorGmsh(ierr);
    int G = gmshModelOccAddPoint(f,0,0,0.0,-1,&ierr); ErrorGmsh(ierr);

    int C1 = gmshModelOccAddPoint(c1,h/2,0,0.0,-1,&ierr); ErrorGmsh(ierr);
    int C2 = gmshModelOccAddPoint(c2,h/2,0,0.0,-1,&ierr); ErrorGmsh(ierr);

    // dimtags for center points
    int C1dt [] = {0,C1};
    int C2dt [] = {0,C2};
    int* center_points [] = {C1dt,C2dt};


    int bottom_handle = gmshModelOccAddLine(A,G,-1,&ierr); ErrorGmsh(ierr);
    int right_handle = gmshModelOccAddLine(G,F,-1,&ierr); ErrorGmsh(ierr);
    int right_arc = gmshModelOccAddCircleArc(F,C2,E,-1,1,&ierr); ErrorGmsh(ierr);
    int tension_arc = gmshModelOccAddCircleArc(E,C2,D,-1,1,&ierr); ErrorGmsh(ierr);
    int right_tip = gmshModelOccAddCircleArc(D,C2,C,-1,1,&ierr); ErrorGmsh(ierr);
    int left_arc = gmshModelOccAddCircleArc(C,C1,B,-1,1,&ierr); ErrorGmsh(ierr);
    int left_handle = gmshModelOccAddLine(B,A,-1,&ierr); ErrorGmsh(ierr);
    // Creation des arcs de cercle
    int curves [] = {bottom_handle,right_handle,right_arc,tension_arc,right_tip,left_arc,left_handle};
    int contour = gmshModelOccAddCurveLoop(curves,7,-1,&ierr); ErrorGmsh(ierr);
    int contourArr [] = {contour};
    int plane = gmshModelOccAddPlaneSurface(contourArr,1,-1,&ierr); ErrorGmsh(ierr);
    for(int i = 0; i < 2; i++){
        gmshModelOccRemove(center_points[i],2,0,&ierr); ErrorGmsh(ierr);
    }
    gmshModelOccSynchronize(&ierr);

    if (theGeometry->elementType == FEM_QUAD) {
        gmshOptionSetNumber("Mesh.SaveAll",1,&ierr);
        gmshOptionSetNumber("Mesh.RecombineAll",1,&ierr);
        gmshOptionSetNumber("Mesh.Algorithm",11,&ierr);  
        gmshOptionSetNumber("Mesh.SmoothRatio", 21.5, &ierr);  
        gmshOptionSetNumber("Mesh.RecombinationAlgorithm",1.0,&ierr); 
        gmshModelGeoMeshSetRecombine(2,1,45,&ierr);  
        gmshModelMeshGenerate(2,&ierr);  }
  
    if (theGeometry->elementType == FEM_TRIANGLE) {
        gmshOptionSetNumber("Mesh.SaveAll",1,&ierr);
        gmshModelMeshGenerate(2,&ierr);  }

    return;
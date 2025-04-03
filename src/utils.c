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

    // Read the number of nodes
    fscanf(file, "Number of nodes %d\n", &nodes->nNodes);
    nodes->X = (double*)malloc(nodes->nNodes * sizeof(double));
    nodes->Y = (double*)malloc(nodes->nNodes * sizeof(double));
    
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
        int iTemp;
        // Read the domain information
        fscanf(file, "  Domain : %6d \n", &iTemp);
        fscanf(file, "  Name : %s %6d\n", geo->domains[iDomain]->name, &iTemp);
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

// Print the discrete space
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

double* femFullSystemEliminate(femFullSystem* system){
    double** A;
    double* B;
    double factor;
    int i,j,k,size;

    A = system->A;
    B = system->B;
    size = system->size;

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
    problem->system = femFullSystemCreate(size);

    printf("Discrete space for the mesh:\n");
    femDiscretePrint(problem->space);
    printf("Discrete space for the edges:\n");
    femDiscretePrint(problem->spaceEdge);

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
          if (theCondition->type==NEUMANN_X)    printf(" imposing %9.2e as the horizontal force desnity \n",value); 
          if (theCondition->type==NEUMANN_Y)    printf(" imposing %9.2e as the vertical force density \n",value);}
    printf(" ======================================================================================= \n\n");

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

}

// TODO
void femElasticityAssembleNeumann(femProblem* problem){

}

// TODO
double* femElasticitySolve(femProblem* problem){

}

// TODO
double* femElasticityForces(femProblem* problem){

}

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
        if (problem->system != NULL) {
            femFullSystemFree(problem->system);
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

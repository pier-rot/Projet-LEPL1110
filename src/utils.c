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
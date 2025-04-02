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

    // if (elementType == FEM_TRIANGLE){
    //     geo->nodes->nNodes = 3;
    // } else if (elementType == FEM_QUAD){
    //     geo->nodes->nNodes = 4;
    // } else if (elementType == FEM_EDGE){
    //     geo->nodes->nNodes = 2;
    // } else {
    //     fprintf(stderr, "Invalid element type.\n");
    //     free(geo->nodes);
    //     free(geo);
    //     return NULL;
    // }

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
    // printf("Number of nodes: %d\n", nodes->nNodes);
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

    // Print the node coordinates
    // printf("Node coordinates:\n");
    // for (int i = 0; i < nodes->nNodes; i++) {
        // printf("%6d : %14.7e %14.7e \n", i, nodes->X[i], nodes->Y[i]);
    // }

    // Read the number of edges
    fscanf(file, "Number of edges %d\n", &edges->nElem);
    printf("Number of edges: %d\n", edges->nElem);
    edges->elem = (int*)malloc(edges->nElem * 2 * sizeof(int));
    for(int i = 0; i < edges->nElem; i++){
        fscanf(file, "%6d : %6d %6d\n", &i, &edges->elem[2*i], &edges->elem[2*i+1]);
    }

    // Print the edges
    // for(int i = 0; i < edges->nElem; i++) {
        // printf("%6d : %6d %6d\n", i, edges->elem[2*i], edges->elem[2*i+1]);
    // }

    // Read the number of triangles
    fscanf(file, "Number of triangles %d\n", &mesh->nElem);
    printf("Number of elements: %d\n", mesh->nElem);

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

    // Print the triangles
    // for (int i = 0; i < mesh->nElem; i++) {
        // printf("%6d : %6d %6d %6d\n", i, mesh->elem[3*i], mesh->elem[3*i+1], mesh->elem[3*i+2]);
    // }

    // Read the number of domains
    fscanf(file, "Number of domains %d\n", &geo->nDomains);
    printf("Number of domains: %d\n", geo->nDomains);
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

    // Print the domains
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



    fclose(file);
    return geo;
}

void geoPrint(femGeo* geo){
    printf("Number of nodes: %d\n", geo->nodes->nNodes);
    printf("Number of elements: %d\n", geo->mesh->nElem);
    printf("Number of edges: %d\n", geo->edges->nElem);
    for (int i = 0; i < geo->nDomains; i++) {
        printf("Domain %d: %s\n", i, geo->domains[i]->name);
    }
}

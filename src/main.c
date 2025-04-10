#include <stdio.h>
#include <math.h>
#include "utils.h"
#include "glfem.h"

#define MAXFILENAMELENGTH 256

int main(int argc, char const *argv[])
{
    char meshfile[MAXFILENAMELENGTH];
    char problemfile[MAXFILENAMELENGTH];
    
    // Check if the correct number of arguments are provided
    if (argc < 3){
        printf("Not enough arguments provided.\n");
        printf("Usage: %s <mesh_path> <problem_path>\n", argv[0]);
        printf("Using default mesh and problem files.\n\n");
        
        sprintf(meshfile, "%s", "./data/mesh.txt");
        sprintf(problemfile, "%s", "./data/problem.txt");

        printf("Using \"%s\" as the mesh file.\n", meshfile);
        printf("Using \"%s\" as the problem file.\n", problemfile);
    } else if (argc == 3){
        printf("Correct number of arguments provided.\n\n");
        
        sprintf(meshfile, "%s", argv[1]);
        sprintf(problemfile, "%s", argv[2]);

        printf("Using \"%s\" as the mesh file.\n", meshfile);
        printf("Using \"%s\" as the problem file.\n", problemfile);
        
    } else {
        printf("Too many arguments provided.\n");
        return 1;
    }

    // FEM parameters
    femElementType elementType = FEM_TRIANGLE; // FEM_TRIANGLE or FEM_QUAD
    femElasticCase iCase = PLANAR_STRESS;  // PLANAR_STRESS or PLANAR_STRAIN or AXISYM
    femSolverType solverType = SOLVER_FULL; // SOLVER_FULL or SOLVER_BAND or SOLVER_GC
    femRenumberType renumberType = NONE; // NONE or X or Y or RCMK

    // Initialize the femGeo structure
    femGeo* geo = geoRead(meshfile);
    if (geo == NULL) {
        fprintf(stderr, "Failed to initialize femGeo structure.\n");
        return 1;
    }
    // geoPrint(geo);
    femMeshRenumber(geo->mesh, renumberType);
    // geoNodesPrint(geo);
    

    femProblem* problem = femElasticityRead(geo, problemfile, solverType, renumberType);
    if (problem == NULL) {
        fprintf(stderr, "Failed to initialize femProblem structure.\n");
        geoFree(geo);
        return 1;
    }

    femElasticityPrint(problem);    

    double* soluce = problem->soluce;
    soluce = femElasticitySolve(problem);
    
    
    double* displacement = malloc(geo->nodes->nNodes * sizeof(double));
    //double *forcesX = malloc(geo->nodes->nNodes * sizeof(double));
    //double *forcesY = malloc(geo->nodes->nNodes * sizeof(double));
    
    for (int i = 0; i < geo->nodes->nNodes; i++) {
       displacement[i] = sqrt(pow(soluce[2*i], 2) + pow(soluce[2*i+1], 2));
    }

    double min = displacement[0];
    double max = displacement[0];
    for (int i = 1; i < geo->nodes->nNodes; i++) {
        if (displacement[i] < min) {
            min = displacement[i];
        }
        if (displacement[i] > max) {
            max = displacement[i];
        }
    }
    // printf("Displacement:\n");
    // for (int i = 0; i < geo->nodes->nNodes; i++) {
    //     printf("Node %d: %le\n", i, displacement[i]);
    // }
    printf("\n");
    printf("Displacement min: %le\n", min);
    printf("Displacement max: %le\n", max);

    //
    // Visualisation du maillage
    //

    int mode = 1; 
    int domain = 0;
    int freezingButton = FALSE;
    double t, told = 0;
    char theMessage[MAXNAME];
   
 
    GLFWwindow *window = glfemInit("EPL1110 : Maillage et calcul de champs de contraintes");
    glfwMakeContextCurrent(window);

    do {
        int w,h;
        glfwGetFramebufferSize(window, &w, &h);
        glfemReshapeWindows(geo->nodes, w, h);

        t = glfwGetTime();  
        if (glfwGetKey(window, 'D') == GLFW_PRESS) { mode = 0; }
        if (glfwGetKey(window, 'V') == GLFW_PRESS) { mode = 1; }
        if (glfwGetKey(window, 'N') == GLFW_PRESS && freezingButton == FALSE) { domain++; freezingButton = TRUE; told = t; }
        if (t - told > 0.5) { freezingButton = FALSE; }
        
        if (mode == 0)
        {
            domain = domain % geo->nDomains;
            glfemPlotDomain( geo->domains[domain]); 
            sprintf(theMessage, "%s : %d ",geo->domains[domain]->name,domain);
            glColor3f(1.0,0.0,0.0); glfemMessage(theMessage);
        }
        else if (mode == 1)
        {
            glfemPlotField(geo->mesh,displacement);
            glfemPlotMesh(geo->mesh); 
            sprintf(theMessage, "Number of elements : %d ",geo->mesh->nElem);
            glColor3f(1.0,0.0,0.0); glfemMessage(theMessage);
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();

    } while(glfwGetKey(window,GLFW_KEY_ESCAPE) != GLFW_PRESS &&
             glfwWindowShouldClose(window) != 1);

    free(displacement);
    
    geoFree(geo);
    
    // femFree(problem);
    exit(EXIT_SUCCESS);
    return 0;
}

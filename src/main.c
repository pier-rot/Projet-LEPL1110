#include <stdio.h>
#include <math.h>
#include "utils.h"

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
    femRenumberType renumberType = X; // NONE or X or Y or RCMK

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
    
    // femElasticityFullPrint(problem);
    geoFree(geo);
    // femFree(problem);
    exit(EXIT_SUCCESS);
    return 0;
}

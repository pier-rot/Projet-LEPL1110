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

    // Initialize the femGeo structure
    femGeo* geo = geoRead(meshfile);
    if (geo == NULL) {
        fprintf(stderr, "Failed to initialize femGeo structure.\n");
        return 1;
    }

    geoPrint(geo);

    ///*

    //
    //  -3- Champ de la taille de r�f�rence du maillage
    //

    double *meshSizeField = malloc(geo->nodes->nNodes*sizeof(double));
    femNodes *theNodes = geo->nodes;
    for(int i=0; i < theNodes->nNodes; ++i)
    meshSizeField[i] = geoSize(theNodes->X[i], theNodes->Y[i]);
    double hMin = femMin(meshSizeField,theNodes->nNodes);  
    double hMax = femMax(meshSizeField,theNodes->nNodes);  
    printf(" ==== Global requested h : %14.7e \n",geo->h);
    printf(" ==== Minimum h          : %14.7e \n",hMin);
    printf(" ==== Maximum h          : %14.7e \n",hMax);



    //
    //  -4- Visualisation du maillage
    //  
    
    int mode = 1; // Change mode by pressing "j", "k", "l"
    int domain = 0;
    int freezingButton = FALSE;
    double t, told = 0;
    char theMessage[256];
    double pos[2] = {20,460};
 
    GLFWwindow* window = glfemInit("EPL1110 : Mesh generation ");
    glfwMakeContextCurrent(window);

    do {
        int w, h;
        glfwGetFramebufferSize(window,&w,&h);
        glfemReshapeWindows(geo->nodes,w,h);
        t = glfwGetTime();  
        // glfemChangeState(&mode, theMeshes->nMesh);
        if (glfwGetKey(window,'D') == GLFW_PRESS) { mode = 0;}
        if (glfwGetKey(window,'V') == GLFW_PRESS) { mode = 1;}
        if (glfwGetKey(window,'N') == GLFW_PRESS && freezingButton == FALSE) { domain++; freezingButton = TRUE; told = t;}
        
        if (t-told > 0.5) { freezingButton = FALSE; }
            
        if (mode == 1) {
            glfemPlotField(geo->edges, meshSizeField);
            glfemPlotMesh(geo->edges); 
            sprintf(theMessage, "Number of elements : %d ",geo->edges->nElem);        
            glColor3f(1.0,0.0,0.0); glfemMessage(theMessage); 
        }
        if (mode == 0) {
            domain = domain % geo->nDomains;
            glfemPlotDomain( geo->domains[domain]);         
            sprintf(theMessage, "%s : %d ",geo->domain[domain]->name,domain);
            glColor3f(1.0,0.0,0.0); glfemMessage(theMessage);
        }
            
        glfwSwapBuffers(window);
        glfwPollEvents();
    } while(glfwGetKey(window,GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) != 1 );
            
    // Check if the ESC key was pressed or the window was closed

    free(meshSizeField);  
    geoFinalize();
    glfwTerminate(); 

    //*/
    


    return 0;
}



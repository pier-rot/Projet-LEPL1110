#include <stdio.h>
#include <math.h>
#include "utils.h"
#include "glfem.h"


#define MAXFILENAMELENGTH 256

double fun(double x, double y) { return 1; }

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
    int ierr;
    
    
    femGeo* geo = geoInit();

    /*/
    // Geometry parameters

    double h = geo->h;
    double x0 = geo->xNotch;
    double y0 = geo->yNotch;
    double r0 = geo->rNotch;
    double h0 = geo->hNotch;
    double d0 = geo->dNotch;
  
    
    double x1 = geo->xHole;
    double y1 = geo->yHole;
    double r1 = geo->rHole;
    double h1 = geo->hHole;
    double d1 = geo->dHole;
    */


   
    geoMeshGenerate();
    geoMeshImport();

    
    geoSetDomainName(0,"Bottom");
    geoSetDomainName(3,"AttachPoint");
    geoSetDomainName(1,"HandleRight");
    geoSetDomainName(6,"HandleLeft");

    //
    //  -2- Creation du fichier du maillage
    //
        
    char filename[] = "../data/elasticity.txt";
    geoMeshWrite(filename);

    //
    // Création du problème
    //

    double E   = 17.2e9; //Young's modulus in Pa
    double nu  = 0.3; //Poisson's ratio between -1 et 0.5
    double rho = 7.85e3; //Density in kg/m^3
    double g   = 0.0; //Gravity in m/s^2
    double t2   = 0.01; // thickness of the plate

    /*

    femNodes *theNodes = geo->nodes;
    theNodes->number = malloc(theNodes->nNodes * sizeof(int));
    for (int i = 0; i<theNodes->nNodes; i++) {
        theNodes->number[i] = i;

    }
    */

    femProblem* theProblem = femElasticityCreate(geo,E,nu,rho,g,t2,iCase);
    femElasticityAddBoundaryCondition(theProblem,"HandleRight",DIRICHLET_X,0.0);
    femElasticityAddBoundaryCondition(theProblem,"Bottom",DIRICHLET_Y,0.0);
    femElasticityAddBoundaryCondition(theProblem,"AttachPoint",NEUMANN_Y,-10e7);
    femElasticityPrint(theProblem);

    //
    //  -3- Resolution du probleme et calcul des forces
    //

    double *theSoluce = femElasticitySolve(theProblem);
    double *theForces = femElasticityForces(theProblem);
    double area       = femElasticityIntegrate(theProblem, fun);   
   
    
    //
    //  -4- Deformation du maillage pour le plot final
    //      Creation du champ de la norme du deplacement
    //
    
    femNodes *theNodes = geo->nodes;
    double deformationFactor = 1e5;
    double *normDisplacement = malloc(theNodes->nNodes * sizeof(double));
    double *forcesX = malloc(theNodes->nNodes * sizeof(double));
    double *forcesY = malloc(theNodes->nNodes * sizeof(double));
    
    for (int i = 0; i < theNodes->nNodes; i++)
    {
        theNodes->X[i] += theSoluce[2*i+0]*deformationFactor;
        theNodes->Y[i] += theSoluce[2*i+1]*deformationFactor;
        normDisplacement[i] = sqrt(theSoluce[2*i+0]*theSoluce[2*i+0] + theSoluce[2*i+1]*theSoluce[2*i+1]);
        forcesX[i] = theForces[2*i+0];
        forcesY[i] = theForces[2*i+1];
    }
  
    double hMin = femMin(normDisplacement, theNodes->nNodes);  
    double hMax = femMax(normDisplacement, theNodes->nNodes); 

    printf(" ==== Minimum displacement          : %14.7e [m] \n",hMin);
    printf(" ==== Maximum displacement          : %14.7e [m] \n",hMax);

    //
    //  -5- Calcul de la force globale resultante
    //

    double theGlobalForce[2] = {0, 0};
    for (int i = 0; i < theProblem->geometry->nodes->nNodes; i++)
    {
        theGlobalForce[0] += theForces[2*i+0];
        theGlobalForce[1] += theForces[2*i+1];
    }

    printf(" ==== Global horizontal force       : %14.7e [N] \n",theGlobalForce[0]);
    printf(" ==== Global vertical force         : %14.7e [N] \n",theGlobalForce[1]);
    printf(" ==== Weight                        : %14.7e [N] \n", area * rho * g);



    //
    //  -6- Visualisation du maillage
    //  
    
    int mode = 1; 
    int domain = 0;
    int freezingButton = FALSE;
    double t, told = 0;
    char theMessage[MAXNAME];
   
 
    GLFWwindow *window = glfemInit("EPL1110 : Recovering forces on constrained nodes");
    glfwMakeContextCurrent(window);

    do {
        int w,h;
        glfwGetFramebufferSize(window, &w, &h);
        glfemReshapeWindows(geo->nodes, w, h);

        t = glfwGetTime();  
        if (glfwGetKey(window, 'D') == GLFW_PRESS) { mode = 0; }
        if (glfwGetKey(window, 'V') == GLFW_PRESS) { mode = 1; }
        if (glfwGetKey(window, 'X') == GLFW_PRESS) { mode = 2; }
        if (glfwGetKey(window, 'Y') == GLFW_PRESS) { mode = 3; }
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
            glfemPlotField(geo->mesh,normDisplacement);
            glfemPlotMesh(geo->mesh); 
            sprintf(theMessage, "Number of elements : %d ",geo->mesh->nElem);
            glColor3f(1.0,0.0,0.0); glfemMessage(theMessage);
        }
        else if (mode == 2)
        {
            glfemPlotField(geo->mesh,forcesX);
            glfemPlotMesh(geo->mesh); 
            sprintf(theMessage, "Number of elements : %d ",geo->mesh->nElem);
            glColor3f(1.0,0.0,0.0); glfemMessage(theMessage);
        }
        else if (mode == 3)
        {
            glfemPlotField(geo->mesh,forcesY);
            glfemPlotMesh(geo->mesh); 
            sprintf(theMessage, "Number of elements : %d ",geo->mesh->nElem);
            glColor3f(1.0,0.0,0.0); glfemMessage(theMessage);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();

    } while(glfwGetKey(window,GLFW_KEY_ESCAPE) != GLFW_PRESS &&
             glfwWindowShouldClose(window) != 1);
            
    // Check if the ESC key was pressed or the window was closed

    free(normDisplacement);
    free(forcesX);
    free(forcesY);
    femElasticityFree(theProblem) ; 
    geoFree(geo);
    glfwTerminate();
    
    
    //
    //  
    //

    /*

    // Initialize the femGeo structure
    //femGeo* geo = geoRead(meshfile);
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
    */

    exit(EXIT_SUCCESS);
    return 0;
}

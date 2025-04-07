#include "utils.h"

double **A_copy = NULL;
double *B_copy  = NULL;
double *femElasticityForces(femProblem *problem)
{
    double *residuals = problem->residuals;
    double *soluce    = problem->soluce;
    void *solverStruct = problem->solver->solver;
    int size;
    
    femFullSystem *systemFull = NULL;
    femBandSystem *systemBand = NULL;

    if (problem->solver->type == SOLVER_FULL) {
        systemFull = (femFullSystem *)solverStruct;
        size = systemFull->size;
    } else if (problem->solver->type == SOLVER_BAND) {
        systemBand = (femBandSystem *)solverStruct;
        size = systemBand->size;
    } else {
        printf("Erreur : femElasticityForces est prévu uniquement pour SOLVER_FULL ou SOLVER_BAND\n");
        exit(1);
    }


    if (residuals == NULL) { 
        residuals = (double *) malloc(sizeof(double) * size); 
    }

    // Initialize residuals to zero
    for (int i = 0; i < size; i++) { residuals[i] = 0.0; }

    
    //Compute residuals: R = A * U - B 
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++) { residuals[i] += A_copy[i][j] * soluce[j]; }
        residuals[i] -= B_copy[i];
    }

    
    for (int i = 0; i < size; i++) { free(A_copy[i]); A_copy[i] = NULL;}
    free(A_copy); free(B_copy);
    A_copy = NULL; B_copy = NULL;

    
    return residuals;
}
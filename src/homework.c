#include "fem.h"




void geoMeshGenerate() {

    femGeo* theGeometry = geoGetGeometry();

    double w = theGeometry->LxPlate;
    double h = theGeometry->LyPlate;
    
    int ierr;
    double r = w/4;
    int idRect = gmshModelOccAddRectangle(0.0,0.0,0.0,w,h,-1,0.0,&ierr); 
    int idDisk = gmshModelOccAddDisk(w/2.0,h/2.0,0.0,r,r,-1,NULL,0,NULL,0,&ierr); 
    int idSlit = gmshModelOccAddRectangle(w/2.0,h/2.0-r,0.0,w,2.0*r,-1,0.0,&ierr); 
    int rect[] = {2,idRect};
    int disk[] = {2,idDisk};
    int slit[] = {2,idSlit};

    gmshModelOccCut(rect,2,disk,2,NULL,NULL,NULL,NULL,NULL,-1,1,1,&ierr); 
    gmshModelOccCut(rect,2,slit,2,NULL,NULL,NULL,NULL,NULL,-1,1,1,&ierr); 
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
}


double *femElasticitySolve(femProblem *theProblem)
{

    femFullSystem  *theSystem = theProblem->system;
    femIntegration *theRule = theProblem->rule;
    femDiscrete    *theSpace = theProblem->space;
    femGeo         *theGeometry = theProblem->geometry;
    femNodes       *theNodes = theGeometry->theNodes;
    femMesh        *theMesh = theGeometry->theElements;
    
    
    double x[4],y[4],phi[4],dphidxsi[4],dphideta[4],dphidx[4],dphidy[4];
    int iElem,iInteg,iEdge,i,j,d,map[4],mapX[4],mapY[4];
    
    int nLocal = theMesh->nLocalNode;

    double a   = theProblem->A;
    double b   = theProblem->B;
    double c   = theProblem->C;      
    double rho = theProblem->rho;
    double g   = theProblem->g;
    double **A = theSystem->A;
    double *B  = theSystem->B;
    
    // Initialize system arrays to zero
    femFullSystemInit(theSystem);
    
    // Loop over elements for assembly
    for (iElem = 0; iElem < theMesh->nElem; iElem++) {
        // Get element nodes and coordinates
        for (i = 0; i < nLocal; i++) {
            map[i] = theMesh->elem[iElem * nLocal + i];
            x[i] = theNodes->X[map[i]];
            y[i] = theNodes->Y[map[i]];
            mapX[i] = 2 * map[i];
            mapY[i] = 2 * map[i] + 1;
        }
        
        // Integration loop
        for (iInteg = 0; iInteg < theRule->n; iInteg++) {
            double xsi = theRule->xsi[iInteg];
            double eta = theRule->eta[iInteg];
            double weight = theRule->weight[iInteg];
            
            // Shape functions and derivatives
            femDiscretePhi2(theSpace, xsi, eta, phi);
            femDiscreteDphi2(theSpace, xsi, eta, dphidxsi, dphideta);
            
            // Compute jacobian
            double dxdxsi = 0.0, dydxsi = 0.0, dxdeta = 0.0, dydeta = 0.0;
            for (i = 0; i < nLocal; i++) {
                dxdxsi += x[i] * dphidxsi[i];
                dydxsi += y[i] * dphidxsi[i];
                dxdeta += x[i] * dphideta[i];
                dydeta += y[i] * dphideta[i];
            }
            
            double jacobian = fabs(dxdxsi * dydeta - dydxsi * dxdeta);
            double invJacobian[2][2] = {
                { dydeta / jacobian, -dydxsi / jacobian },
                { -dxdeta / jacobian, dxdxsi / jacobian }
            };
            
            for (i = 0; i < nLocal; i++) {
                dphidx[i] = invJacobian[0][0] * dphidxsi[i] + invJacobian[0][1] * dphideta[i];
                dphidy[i] = invJacobian[1][0] * dphidxsi[i] + invJacobian[1][1] * dphideta[i];
            }
            
            for (i = 0; i < nLocal; i++) {
                for (j = 0; j < nLocal; j++) {
                    double Axx = a * dphidx[i] * dphidx[j] + c * dphidy[i] * dphidy[j];
                    double Ayy = c * dphidx[i] * dphidx[j] + a * dphidy[i] * dphidy[j];
                    double Axy = b * dphidx[i] * dphidy[j];
                    double Ayx = b * dphidy[i] * dphidx[j];
                    
                    A[mapX[i]][mapX[j]] += Axx * jacobian * weight;
                    A[mapY[i]][mapY[j]] += Ayy * jacobian * weight;
                    A[mapX[i]][mapY[j]] += Axy * jacobian * weight;
                    A[mapY[i]][mapX[j]] += Ayx * jacobian * weight;
                }
                B[mapY[i]] += phi[i] * rho * g * jacobian * weight;
            }
        }
    }
    
    // Apply boundary conditions
    int *theConstrainedNodes = theProblem->constrainedNodes;     
    for (int i = 0; i < theSystem->size; i++) {
        if (theConstrainedNodes[i] != -1) {
            double value = theProblem->conditions[theConstrainedNodes[i]]->value;
            femFullSystemConstrain(theSystem, i, value);
        }
    }
                            
    // Solve the system
    return femFullSystemEliminate(theSystem);
}

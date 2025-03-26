#include "fem.h"
// Fait par Marie Deprez & Pierre Vandermeersch



// double geoSize(double x, double y){
// 
    // femGeo* theGeometry = geoGetGeometry();
    // 
    // return 0.01;
// }

void geoMeshGenerate() {
 
    int ierr;

    double c = 0.15;
    double h = 0.1;
    double f = 0.025;
    double l = 0.7;
    double c1 = (c*c + l*l - (l*h) + ((h*h)/4.0))/(2.0*c);
    double c2 = (c*c + l*l - (l*h) - f*f + ((h*h)/4.0))/(2.0*(c-f));

    // Extremites de l'arc
    int A = gmshModelOccAddPoint(c,l,0,0.0,-1,&ierr); ErrorGmsh(ierr);
    int B = gmshModelOccAddPoint(c,-l,0,0.0,-1,&ierr); ErrorGmsh(ierr);

    // Coins de la poignée rectangulaire
    int P1 = gmshModelOccAddPoint(0,h/2,0,0.0,-1,&ierr); ErrorGmsh(ierr);
    int P2 = gmshModelOccAddPoint(f,h/2,0,0.0,-1,&ierr); ErrorGmsh(ierr);
    int P3 = gmshModelOccAddPoint(0,-h/2,0,0.0,-1,&ierr); ErrorGmsh(ierr);
    int P4 = gmshModelOccAddPoint(f,-h/2,0,0.0,-1,&ierr); ErrorGmsh(ierr);

    // Centres des arcs de cercle pour contruire l'arc
    int C1 = gmshModelOccAddPoint(c1,h/2,0,0.0,-1,&ierr); ErrorGmsh(ierr);
    int C2 = gmshModelOccAddPoint(c2,h/2,0,0.0,-1,&ierr); ErrorGmsh(ierr);
    int C3 = gmshModelOccAddPoint(c1,-h/2,0,0.0,-1,&ierr); ErrorGmsh(ierr);
    int C4 = gmshModelOccAddPoint(c2,-h/2,0,0.0,-1,&ierr); ErrorGmsh(ierr);
    // DimTags array
    int C1dt [] = {0,C1};
    int C2dt [] = {0,C2};
    int C3dt [] = {0,C3};
    int C4dt [] = {0,C4};
    int* center_points [] = {C1dt,C2dt,C3dt,C4dt};

    // Creation des arcs de cercle
    int top_left_arc = gmshModelOccAddCircleArc(A,C1,P1,-1,1,&ierr); ErrorGmsh(ierr);
    int top_right_arc = gmshModelOccAddCircleArc(P2,C2,A,-1,1,&ierr); ErrorGmsh(ierr);
    int bottom_left_arc = gmshModelOccAddCircleArc(P3,C3,B,-1,1,&ierr); ErrorGmsh(ierr);
    int bottom_right_arc = gmshModelOccAddCircleArc(B,C4,P4,-1,1,&ierr); ErrorGmsh(ierr);
    int left_handle = gmshModelOccAddLine(P1,P3,-1,&ierr); ErrorGmsh(ierr);
    int right_handle = gmshModelOccAddLine(P4,P2,-1,&ierr); ErrorGmsh(ierr);
    int curves [] = {top_left_arc,left_handle,bottom_left_arc,bottom_right_arc,right_handle,top_right_arc};
    int contour = gmshModelOccAddCurveLoop(curves,6,-1,&ierr); ErrorGmsh(ierr);
    int contourArr [] = {contour};
    int plane = gmshModelOccAddPlaneSurface(contourArr,1,-1,&ierr); ErrorGmsh(ierr);
    for(int i = 0; i < 4; i++){
        gmshModelOccRemove(center_points[i],2,0,&ierr); ErrorGmsh(ierr);
    }
    
                                  
    gmshModelOccSynchronize(&ierr);
    gmshOptionSetNumber("Mesh.SaveAll", 1, &ierr);
    gmshModelMeshGenerate(2, &ierr);  
       
//
//  Generation de quads :-)
//
//    gmshOptionSetNumber("Mesh.SaveAll", 1, &ierr);
//    gmshOptionSetNumber("Mesh.RecombineAll", 1, &ierr);
//    gmshOptionSetNumber("Mesh.Algorithm", 8, &ierr);
//    gmshOptionSetNumber("Mesh.RecombinationAlgorithm", 1.0, &ierr);
//    gmshModelGeoMeshSetRecombine(2,1,45,&ierr);
//    gmshModelMeshGenerate(2, &ierr);  
   
}
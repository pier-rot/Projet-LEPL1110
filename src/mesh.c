#include "fem.h"
#include <math.h>

double geoSize(double x, double y)
{
    return 0.01;
}

void geoMeshGenerate()
{
    int ierr;

    double c = 0.15;
    double h = 0.1;
    double f = 0.025;
    double l = 0.7;
    double L = 0.005;
    double pi = 3.14159265358979323846;
    double xc = 0.1203;

    double c1 = (c*c + l*l - (l*h) + ((h*h)/4.0))/(2.0*c);
    double c2 = (c*c + l*l - (l*h) - f*f + ((h*h)/4.0))/(2.0*(c-f));
    double beta = pi - acos((c2-xc)/(c2-f));
    double alpha = L/(c2-f);


    int A = gmshModelOccAddPoint(0,0,0,0.0,-1,&ierr); 
    int B = gmshModelOccAddPoint(0,(h/2),0,0.0,-1,&ierr); 
    int C = gmshModelOccAddPoint(c,l,0,0.0,-1,&ierr); 
    int D = gmshModelOccAddPoint(c2+(c2-f)*cos(beta-(alpha/2)),(h/2)+(c2-f)*sin(beta-(alpha/2)),0,0.0,-1,&ierr); 
    int E = gmshModelOccAddPoint(c2+(c2-f)*cos(beta+(alpha/2)),(h/2)+(c2-f)*sin(beta+(alpha/2)),0,0.0,-1,&ierr); 
    int F = gmshModelOccAddPoint(f,(h/2),0,0.0,-1,&ierr); 
    int G = gmshModelOccAddPoint(f,0,0,0.0,-1,&ierr); 

    int C1 = gmshModelOccAddPoint(c1,h/2,0,0.0,-1,&ierr); 
    int C2 = gmshModelOccAddPoint(c2,h/2,0,0.0,-1,&ierr); 

    // dimtags for center points
    int C1dt [] = {0,C1};
    int C2dt [] = {0,C2};
    int* center_points [] = {C1dt,C2dt};


    int bottom_handle = gmshModelOccAddLine(A,G,-1,&ierr); 
    int right_handle = gmshModelOccAddLine(G,F,-1,&ierr); 
    int right_arc = gmshModelOccAddCircleArc(F,C2,E,-1,1,&ierr); 
    int tension_arc = gmshModelOccAddCircleArc(E,C2,D,-1,1,&ierr); 
    int right_tip = gmshModelOccAddCircleArc(D,C2,C,-1,1,&ierr); 
    int left_arc = gmshModelOccAddCircleArc(C,C1,B,-1,1,&ierr); 
    int left_handle = gmshModelOccAddLine(B,A,-1,&ierr); 
    // Creation des arcs de cercle
    int curves [] = {bottom_handle,right_handle,right_arc,tension_arc,right_tip,left_arc,left_handle};
    int contour = gmshModelOccAddCurveLoop(curves,7,-1,&ierr); 
    int contourArr [] = {contour};
    int plane = gmshModelOccAddPlaneSurface(contourArr,1,-1,&ierr); 
    for(int i = 0; i < 2; i++){
        gmshModelOccRemove(center_points[i],2,0,&ierr); 
    }              
   
    geoSetSizeCallback(geoSize);
                                  
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
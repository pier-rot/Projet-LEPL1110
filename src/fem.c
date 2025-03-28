#include "fem.h"

femGeo theGeometry;

void geoSetSizeCallback(double (*geoSize)(double x, double y)) {
    theGeometry.geoSize = geoSize; 
}


void femErrorGmsh(int ierr, int line, char *file)                                  
{ 
    if (ierr == 0)  return;
    printf("\n-------------------------------------------------------------------------------- ");
    printf("\n  Error in %s at line %d : \n  error code returned by gmsh %d\n", file, line, ierr);
    printf("--------------------------------------------------------------------- Yek Yek !! \n\n");
    gmshFinalize(NULL);                                        
    exit(69);                                                 
}
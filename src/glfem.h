#ifndef _GLFEM_H_
#define _GLFEM_H_

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

//#include "fem.h"
#include "utils.h"


void        glfemDrawColorElement(float *x, float *y, double *u, int n);
void 		    glfemDrawElement(float *x, float *y, int n);
void 		    glfemDrawNodes(double* x, double* y,int n);
int         glfemGetAction();

void 		    glfemReshapeWindows(femNodes *theNodes, int width, int height);
void 		    glfemPlotField(femMesh *theMesh, double *u);
void 		    glfemPlotMesh(femMesh *theMesh);
void        glfemPlotDomain(femDomain *theDomain);

void 		    glfemMessage(char *message);
void 		    glfemDrawMessage(int h, int v, char *message);
void 		    glfemSetRasterSize(int width, int height);

GLFWwindow* glfemInit(char *windowName);
static void glfemKeyCallback(GLFWwindow* self,int key,int scancode,int action,int mods);




#endif
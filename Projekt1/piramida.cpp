#include "stdafx.h"
#include <GLTools.h>
#include <windows.h>
#include <GLFrustum.h>
#include <GLFrame.h>
#ifdef __APPLE__
#include <glut/glut.h>          // OS X version of GLUT
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

void SetUpFrame(GLFrame &frame,const M3DVector3f origin,
				const M3DVector3f forward,
				const M3DVector3f up);

void LookAt(GLFrame &frame, const M3DVector3f eye,
        const M3DVector3f at,
        const M3DVector3f up);

GLuint				shader;
GLFrame             frame;
GLFrame             cameraFrame;
GLFrustum           viewFrustum;
GLuint				MVPMatrixLocation;

M3DMatrix44f		CameraMatrix;
M3DMatrix44f		ProjectionMatrix;

void SetUpFrame(GLFrame &frame,const M3DVector3f origin,const M3DVector3f forward,const M3DVector3f up) {
						frame.SetOrigin(origin);
						frame.SetForwardVector(forward);
						M3DVector3f side,oUp;
						m3dCrossProduct3(side,forward,up);
						m3dCrossProduct3(oUp,side,forward);
						frame.SetUpVector(oUp);
						frame.Normalize();
};

void LookAt(GLFrame &frame, const M3DVector3f eye,const M3DVector3f at,const M3DVector3f up) {
    M3DVector3f forward;
    m3dSubtractVectors3(forward, at, eye);
    SetUpFrame(frame, eye, forward, up);
}

void ChangeSize(int w, int h) {
    glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(30, w/h, 50, 150);
}

void SetupRC() {
	glClearColor(0.1f, 0.2f, 0.3f, 0.0f);
    shader = gltLoadShaderPairWithAttributes("pass_thru_shader.vp", "pass_thru_shader.fp",2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_COLOR, "vColor");
    fprintf(stdout, "GLT_ATTRIBUTE_VERTEX : %d\nGLT_ATTRIBUTE_COLOR : %d \n",GLT_ATTRIBUTE_VERTEX, GLT_ATTRIBUTE_COLOR);

	MVPMatrixLocation = glGetUniformLocation(shader,"MVMatrix");
	if(MVPMatrixLocation==-1) {
		fprintf(stderr,"uniform MVMatrix could not be found\n");
	}

	M3DVector3f	eye = {0.0f, -0.3f, 2.0f};		//Ustawienia kamery
	M3DVector3f	at = {0.0f, 1.0f, -1.0f};
	M3DVector3f	up = {1.0f, 1.0f, 0.0f};

	LookAt(frame,eye,at,up);

}

// Called to draw scene

void RenderScene(void) {
	M3DMatrix44f CameraMatrix;
    M3DMatrix44f ViewProjectionMatrix;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
   
	glUseProgram(shader);

	cameraFrame.GetCameraMatrix(CameraMatrix);
	frame.GetMatrix(ProjectionMatrix);
	m3dMatrixMultiply44(ViewProjectionMatrix,ProjectionMatrix,CameraMatrix);
	glUniformMatrix4fv(MVPMatrixLocation,1,GL_FALSE,ViewProjectionMatrix);

	glBegin(GL_QUADS);
		glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.5, 0.5, 0.5);
		glVertex3f(-1.0f, 1.0f, 0.0f);
		glVertex3f(-1.0f,-1.0f, 0.0f);
		glVertex3f( 1.0f,-1.0f, 0.0f);
		glVertex3f( 1.0f, 1.0f, 0.0f);
	glEnd();

	glBegin(GL_TRIANGLES);
		glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0, 1.0, 0.0);
		glVertex3f(0.0, 0.0, 2.0);
		glVertex3f(-1.0, 1.0, 0.0);
		glVertex3f(-1.0, -1.0, 0.0);
		glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 2.0);
		glVertex3f(-1.0, -1.0, 0.0);
		glVertex3f(1.0, -1.0, 0.0);

		glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0, 1.0, 0.0);
		glVertex3f(0.0, 0.0, 2.0);
		glVertex3f(1.0, -1.0, 0.0);
		glVertex3f(1.0, 1.0, 0.0);

		glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0, 0.0, 1.0);
		glVertex3f(0.0, 0.0, 2.0);
		glVertex3f(1.0, 1.0, 0.0);
		glVertex3f(-1.0, 1.0, 0.0);
    glEnd();
	
	glutSwapBuffers();
	//cameraFrame.RotateLocalZ(0.01);  //Obrót kamery
	glutPostRedisplay();

}
int main(int argc, char* argv[]) {
  

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(1024, 768);
    glutCreateWindow("Piramida");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }

    SetupRC();

    glutMainLoop();
    return 0;
}

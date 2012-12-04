#include <windows.h>
#include <GLTools.h>  
#include <GL/glew.h>
#define FREEGLUT_STATIC
#include <GL/freeglut.h>            
//#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "gltools.lib")

#include <GLFrustum.h>
#include <GLFrame.h>
#include <StopWatch.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>

GLuint shader;
GLuint MVMatrixLocation;
GLFrame cameraFrame;
GLFrustum viewFrustum;
CStopWatch timer;
GLMatrixStack modelView;
GLMatrixStack projection;
GLGeometryTransform geometryPipeline;
GLMatrixStack matrixStack;
GLuint MVPMatrixLocation;
GLuint normalMatrixLocation;
GLuint ambientLightLocation;
GLuint shaderPositionLocation;
GLuint shaderColorLocation;
GLuint shaderAngleLocation;
GLuint shaderAttenuation0Location;
GLuint shaderAttenuation1Location;
GLuint shaderAttenuation2Location;
GLuint shaderAmbientColorLocation;
GLuint shaderDiffuseColorLocation;
GLuint shaderSpecularColorLocation;
GLuint shaderSpecularExponentLocation;

void ChangeSize(int w, int h) {
    glViewport(0, 0, w, h);
    viewFrustum.SetPerspective(50.0f, w/h, 0.1f, 1000.0f);
}

void SetUpFrame(GLFrame &frame, const M3DVector3f origin, const M3DVector3f forward, const M3DVector3f up) {
        frame.SetOrigin(origin);
        frame.SetForwardVector(forward);
        M3DVector3f side,oUp;
        m3dCrossProduct3(side,forward,up);
        m3dCrossProduct3(oUp,side,forward);
        frame.SetUpVector(oUp);
        frame.Normalize();
}

void LookAt(GLFrame &frame, const M3DVector3f eye, const M3DVector3f at, const M3DVector3f up) {
    M3DVector3f forward;
    m3dSubtractVectors3(forward, at, eye);
    SetUpFrame(frame, eye, forward, up);
}

void SetupRC() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    shader = gltLoadShaderPairWithAttributes("pass_thru_shader.vp", "pass_thru_shader.fp", 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal");
    fprintf(stdout, "GLT_ATTRIBUTE_VERTEX : %d\nGLT_ATTRIBUTE_COLOR : %d \n", GLT_ATTRIBUTE_VERTEX, GLT_ATTRIBUTE_COLOR);

	shaderColorLocation = glGetUniformLocation(shader, "light1.color");
	shaderAngleLocation = glGetUniformLocation(shader, "light1.angle");
	shaderAttenuation0Location = glGetUniformLocation(shader, "light1.attenuation0");
	shaderAttenuation1Location = glGetUniformLocation(shader, "light1.attenuation1");
	shaderAttenuation2Location = glGetUniformLocation(shader, "light1.attenuation2");


    MVMatrixLocation = glGetUniformLocation(shader,"MVMatrix");
    MVPMatrixLocation = glGetUniformLocation(shader,"MVPMatrix");
    normalMatrixLocation = glGetUniformLocation(shader,"normalMatrix");
    ambientLightLocation = glGetUniformLocation(shader,"ambientLight");
    shaderPositionLocation = glGetUniformLocation(shader, "light1.position");

	

	shaderAmbientColorLocation = glGetUniformLocation(shader, "material.ambientColor");
    shaderDiffuseColorLocation = glGetUniformLocation(shader, "material.diffuseColor");
    shaderSpecularColorLocation = glGetUniformLocation(shader, "material.specularColor");
    shaderSpecularExponentLocation = glGetUniformLocation(shader, "material.specularExponent");
	if(MVMatrixLocation==-1){
        fprintf(stderr,"uniform MVMatrix could not be found\n");
    }
    geometryPipeline.SetMatrixStacks(modelView, projection);
    M3DVector3f eye = {1.0f, 1.0f, 10.0f};
    M3DVector3f at = {0.0f, 0.0f, 0.0f};
    M3DVector3f up = {0.0f, 0.0f, 0.5f};
    LookAt(cameraFrame, eye, at, up);
}

void TriangleFace(M3DVector3f a, M3DVector3f b, M3DVector3f c) {
   M3DVector3f normal, bMa, cMa;
   m3dSubtractVectors3(bMa, b, a);
   m3dSubtractVectors3(cMa, c, a);
   m3dCrossProduct3(normal, bMa, cMa);
   m3dNormalizeVector3(normal);
   glVertexAttrib3fv(GLT_ATTRIBUTE_NORMAL, normal);
   glVertex3fv(a);
   glVertex3fv(b);
   glVertex3fv(c);
}

void rysujPiramide(){
    M3DVector3f a = {-1.0f, -1.0f, 0.0f};
    M3DVector3f b = {1.0f, -1.0f, 0.0f};
    M3DVector3f c = {1.0f, 1.0f, 0.0f};
    M3DVector3f d = {-1.0f, 1.0f, 0.0f};
    M3DVector3f e = {0.0f, 0.0f, 2.0f};

    glBegin(GL_QUADS);
        glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.73f, 0.73f, 0.73f);
        glVertex3fv(a);
        glVertex3fv(b);
        glVertex3fv(c);
        glVertex3fv(d);
    glEnd();
    glBegin(GL_TRIANGLES);
        glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.9f, 1.0f, 0.0f);
        TriangleFace(e, a, b);
    glEnd();
    glBegin(GL_TRIANGLES);
        glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.5f, 1.0f, 0.0f);
        TriangleFace(e, b, c);
    glEnd();
    glBegin(GL_TRIANGLES);
        glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.5f, 0.0f, 1.0f);
        TriangleFace(e, c, d);
    glEnd();
    glBegin(GL_TRIANGLES);
        glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.1f, 0.0f, 0.5f);
        TriangleFace(e, d, a);
    glEnd();
}

void kratka(){
    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    for(int i=-10;i<11;i++){
        for(int j=-10; j<11;j++){
            glVertex3f(i, j, 0.0f);
            glVertex3f(-i, j, 0.0f);
            glVertex3f(i, -j, 0.0f);
            glVertex3f(i, j, 0.0f);
        }
    }
    glEnd();
}
void RenderScene(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glFrontFace(GL_CW);
    glUseProgram(shader);
    M3DVector3f at={0.0f, 0.0f, 0.0f};
    M3DVector3f up={0.0f, 0.0f, 1.0f};
    M3DVector3f eye;
    float angle = timer.GetElapsedSeconds()*3.14159f/8;
    eye[0]= -6.8f * cos(angle);
    eye[1]= 5.0f * sin(angle);
    eye[2]= 20.0f;
    LookAt(cameraFrame,eye,at,up);

    M3DVector3f ambientLight = {1.0f, 1.0f, 1.0f};
    M3DVector3f position = {10.0f, 20.0f, 10.0f};
    M3DVector3f color = {1.0f, 0.0f, 1.0f};
    float l_angle = 90.0f;
    float attenuation0 = 0.01f;
    float attenuation1 = 0.01f;
    float attenuation2 = 0.01f;
    M3DVector3f ambientColor = {0.3f, 0.3f, 0.3f};
    M3DVector3f diffuseColor = {0.0f, 0.0f, 1.0f};
    M3DVector3f specularColor = {1.0f, 1.0f, 1.0f};
    float specularExponent = 10;

    projection.LoadMatrix(viewFrustum.GetProjectionMatrix());
    modelView.PushMatrix();
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    modelView.LoadMatrix(mCamera);
    modelView.PushMatrix();

		glUniform3fv(shaderPositionLocation, 1, position);
		glUniform3fv(shaderColorLocation, 1, color);
		glUniform1f(shaderAngleLocation, angle);
		glUniform1f(shaderAttenuation0Location, attenuation0);
		glUniform1f(shaderAttenuation1Location, attenuation1);
		glUniform1f(shaderAttenuation2Location, attenuation2);

    glUniformMatrix4fv(MVMatrixLocation,1,GL_FALSE,geometryPipeline.GetModelViewProjectionMatrix());
    glUniformMatrix4fv(MVPMatrixLocation,1,GL_FALSE,geometryPipeline.GetModelViewMatrix());
    glUniformMatrix3fv(normalMatrixLocation,1,GL_FALSE,geometryPipeline.GetNormalMatrix());
    glUniform3fv(ambientLightLocation, 1, ambientLight);

    glUniform3fv(shaderAmbientColorLocation, 1, ambientColor);
    glUniform3fv(shaderDiffuseColorLocation, 1, diffuseColor);
    glUniform3fv(shaderSpecularColorLocation, 1, specularColor);
    glUniform1f(shaderSpecularExponentLocation, specularExponent);

    glPolygonOffset(1.0f, 1.0f);
		kratka();
    glEnable(GL_POLYGON_OFFSET_FILL);
		kratka();
    glDisable(GL_POLYGON_OFFSET_FILL);
   
	modelView.Translate(0.0f, 0.0f, 0.0f);
    glUniform3fv(shaderPositionLocation, 1, position);
    glUniform3fv(shaderColorLocation, 1, color);
    glUniform1f(shaderAngleLocation, l_angle);
    glUniform1f(shaderAttenuation0Location, attenuation0);
    glUniform1f(shaderAttenuation1Location, attenuation1);
    glUniform1f(shaderAttenuation2Location, attenuation2);

    glUniformMatrix4fv(MVMatrixLocation,1,GL_FALSE,geometryPipeline.GetModelViewProjectionMatrix());
    glUniformMatrix4fv(MVPMatrixLocation,1,GL_FALSE,geometryPipeline.GetModelViewMatrix());
    glUniformMatrix3fv(normalMatrixLocation,1,GL_FALSE,geometryPipeline.GetNormalMatrix());
    
	glUniform3fv(ambientLightLocation, 1, ambientLight);    
    glUniform3fv(shaderAmbientColorLocation, 1, ambientColor);
    glUniform3fv(shaderDiffuseColorLocation, 1, diffuseColor);
    glUniform3fv(shaderSpecularColorLocation, 1, specularColor);
    glUniform1f(shaderSpecularExponentLocation, specularExponent);
		rysujPiramide();
    modelView.PopMatrix();
    
	modelView.PushMatrix();
    modelView.Translate(5.0f, 3.0f, 5.0f);
    modelView.Rotate(60,1,1,0);
    glUniform3fv(shaderPositionLocation, 1, position);
    glUniform3fv(shaderColorLocation, 1, color);
    glUniform1f(shaderAngleLocation, l_angle);
    glUniform1f(shaderAttenuation0Location, attenuation0);
    glUniform1f(shaderAttenuation1Location, attenuation1);
    glUniform1f(shaderAttenuation2Location, attenuation2);

    glUniformMatrix4fv(MVMatrixLocation,1,GL_FALSE,geometryPipeline.GetModelViewProjectionMatrix());
    glUniformMatrix4fv(MVPMatrixLocation,1,GL_FALSE,geometryPipeline.GetModelViewMatrix());
    glUniformMatrix3fv(normalMatrixLocation,1,GL_FALSE,geometryPipeline.GetNormalMatrix());
    
	glUniform3fv(ambientLightLocation, 1, ambientLight);    
    glUniform3fv(shaderAmbientColorLocation, 1, ambientColor);
    glUniform3fv(shaderDiffuseColorLocation, 1, diffuseColor);
    glUniform3fv(shaderSpecularColorLocation, 1, specularColor);
    glUniform1f(shaderSpecularExponentLocation, specularExponent);
		rysujPiramide();
    modelView.PopMatrix();
   
	modelView.PushMatrix();
    modelView.Translate(-5.0f, -3.0f, -5.0f);
    modelView.Scale(2.5f, 2.5f, 4.0f);
    glUniform3fv(shaderPositionLocation, 1, position);
    glUniform3fv(shaderColorLocation, 1, color);
    glUniform1f(shaderAngleLocation, l_angle);
    glUniform1f(shaderAttenuation0Location, attenuation0);
    glUniform1f(shaderAttenuation1Location, attenuation1);
    glUniform1f(shaderAttenuation2Location, attenuation2);

    glUniformMatrix4fv(MVMatrixLocation,1,GL_FALSE,geometryPipeline.GetModelViewProjectionMatrix());
    glUniformMatrix4fv(MVPMatrixLocation,1,GL_FALSE,geometryPipeline.GetModelViewMatrix());
    glUniformMatrix3fv(normalMatrixLocation,1,GL_FALSE,geometryPipeline.GetNormalMatrix());
    
	glUniform3fv(ambientLightLocation, 1, ambientLight);    
    glUniform3fv(shaderAmbientColorLocation, 1, ambientColor);
    glUniform3fv(shaderDiffuseColorLocation, 1, diffuseColor);
    glUniform3fv(shaderSpecularColorLocation, 1, specularColor);
    glUniform1f(shaderSpecularExponentLocation, specularExponent);
    rysujPiramide();
    modelView.PopMatrix();

	
    modelView.PopMatrix();

    glutSwapBuffers(); 
    glutPostRedisplay(); 
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Piramidycienie");
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
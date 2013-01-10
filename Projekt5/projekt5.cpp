#include <GLTools.h>            // OpenGL toolkit
#include <GLFrustum.h>
#include <GLFrame.h>
#include <StopWatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <iostream>
#include <vector>
#ifdef __APPLE__
#include <glut/glut.h>          // OS X version of GLUT
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>            // Windows FreeGlut equivalent
#endif 

GLuint shader;
GLuint MVMatrixLocation;
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
GLFrame cameraFrame;
M3DMatrix44f mCamera;
GLFrustum viewFrustum;
CStopWatch timer;
GLdouble M_Pi =3.14f;
GLMatrixStack modelView;
GLMatrixStack projection;
GLGeometryTransform geometryPipeline;
int n_faces = 0, n_vertices = 0;

//swiatlo i kolor
M3DVector3f ambientLight = {1.0f, 1.0f, 1.0f};
M3DVector3f position = {9.0f, 9.0f, 3.0f};
M3DVector3f color = {1.0f, 1.0f, 1.0f};
float lightAngle = 45.0f;
float attenuation0 = 0.01f;
float attenuation1 = 0.01f;
float attenuation2 = 0.01f;
M3DVector3f ambientColor = {0.2f, 0.2f, 0.2f};
M3DVector3f diffuseColor = {1.0f, 0.0f, 0.0f};  // global color
M3DVector3f diffuseColor2 = {0.5f, 0.5f, 0.5f}; // floor color
M3DVector3f specularColor = {1.0f, 1.0f, 1.0f};

float specularExponent = 6;
float angle;
float ico_vertices[3 * 12] = {
      0., 0., -0.9510565162951536,
      0., 0., 0.9510565162951536,
      -0.85065080835204, 0., -0.42532540417601994,
      0.85065080835204, 0., 0.42532540417601994,
      0.6881909602355868, -0.5, -0.42532540417601994,
      0.6881909602355868, 0.5, -0.42532540417601994,
      -0.6881909602355868, -0.5, 0.42532540417601994,
      -0.6881909602355868, 0.5, 0.42532540417601994,
      -0.2628655560595668, -0.8090169943749475, -0.42532540417601994,
      -0.2628655560595668, 0.8090169943749475, -0.42532540417601994,
      0.2628655560595668, -0.8090169943749475, 0.42532540417601994,
      0.2628655560595668, 0.8090169943749475, 0.42532540417601994
      };

int ico_faces[3*20]={
      1 ,			 11 ,			 7 ,
      1 ,			 7 ,			 6 ,
      1 ,			 6 ,			 10 ,
      1 ,			 10 ,			 3 ,
      1 ,			 3 ,			 11 ,
      4 ,			 8 ,			 0 ,
      5 ,			 4 ,			 0 ,
      9 ,			 5 ,			 0 ,
      2 ,			 9 ,			 0 ,
      8 ,			 2 ,			 0 ,
      11 ,			 9 ,			 7 ,
      7 ,			 2 ,			 6 ,
      6 ,			 8 ,			 10 ,
      10 ,			 4 ,			 3 ,
      3 ,			 5 ,			 11 ,
      4 ,			 10 ,			 8 ,
      5 ,			 3 ,			 4 ,
      9 ,			 11 ,			 5 ,
      2 ,			 7 ,			 9 ,
      8 ,			 6 ,			 2 };

void ChangeSize(int w, int h) {
    glViewport(0, 0, w, h);
    viewFrustum.SetPerspective( 60.0f, w/h, 5.0f, 3000.0f );
}



void SetUpFrame(GLFrame &frame,const M3DVector3f origin, const M3DVector3f forward, const M3DVector3f up) {
	frame.SetOrigin(origin);
	frame.SetForwardVector(forward);
	M3DVector3f side,oUp;
	m3dCrossProduct3(side,forward,up);
	m3dCrossProduct3(oUp,side,forward);
	frame.SetUpVector(oUp);
	frame.Normalize();
};

void LookAt(GLFrame &frame, const M3DVector3f eye, const M3DVector3f at, const M3DVector3f up) {
    M3DVector3f forward;
    m3dSubtractVectors3(forward, at, eye);
    SetUpFrame(frame, eye, forward, up);
}

void TriangleFace(M3DVector3f a, M3DVector3f b, M3DVector3f c){
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

void drawTriangles(int n_faces, float *vertices, int *faces) {
    for (int i = 0; i < n_faces; i++) {
        glBegin(GL_TRIANGLES);
        TriangleFace(vertices + 3 * faces[3 * i], vertices + 3 * faces[3 * i + 1], vertices + 3 * faces[3 * i + 2]);
        glEnd();
    }
}

void drawGrid()
{
// podloga
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

void drawFloor()
{
    glBegin(GL_TRIANGLES);
    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0, 0.0, 0.0);

        M3DVector3f a = {-10.0f, -10.0f, 0.0f};
        M3DVector3f b = { 10.0f, -10.0f, 0.0f};
        M3DVector3f c = { 10.0f,  10.0f, 0.0f};
        M3DVector3f d = {-10.0f,  10.0f, 0.0f};
        TriangleFace(a, b, c);
        TriangleFace(a, c, d);
    glEnd();
}

 void drawSmoothTriangles(int n_faces, float *vertices, int *faces) {
      M3DVector3f normal;
      for (int i = 0; i < n_faces; i++) {
      glBegin(GL_TRIANGLES);
      for(int j=0;j<3;++j) {
      m3dCopyVector3(normal,vertices+3*faces[i*3+j]);
      m3dNormalizeVector3(normal);
      glVertexAttrib3fv(GLT_ATTRIBUTE_NORMAL, normal);
      glVertex3fv(vertices+3*faces[i*3+j]);
      
      }
      glEnd();
      }
      }

 


void SetupRC() {
    
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
    shader = gltLoadShaderPairWithAttributes("shader.vp", "shader.fp",
            2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal");
    fprintf(stdout, "GLT_ATTRIBUTE_VERTEX : %d\nGLT_ATTRIBUTE_NORMAL : %d \n",
            GLT_ATTRIBUTE_VERTEX, GLT_ATTRIBUTE_COLOR);


	MVMatrixLocation = glGetUniformLocation(shader,"MVMatrix");
	MVPMatrixLocation = glGetUniformLocation(shader, "MVPMatrix");
	normalMatrixLocation = glGetUniformLocation(shader, "normalMatrix");
	ambientLightLocation = glGetUniformLocation(shader, "ambientLight");
	shaderPositionLocation = glGetUniformLocation(shader, "light1.position");
	shaderColorLocation = glGetUniformLocation(shader, "light1.color");
	shaderAngleLocation = glGetUniformLocation(shader, "light1.angle");
	shaderAttenuation0Location = glGetUniformLocation(shader,"light1.attenuation0");
	shaderAttenuation1Location = glGetUniformLocation(shader, "light1.attenuation1");
	shaderAttenuation2Location = glGetUniformLocation(shader, "light1.attenuation2");
	shaderAmbientColorLocation = glGetUniformLocation(shader, "material.ambientColor");
	shaderDiffuseColorLocation = glGetUniformLocation(shader, "material.diffuseColor");
	shaderSpecularColorLocation = glGetUniformLocation(shader, "material.specularColor");
	shaderSpecularExponentLocation = glGetUniformLocation(shader, "material.specularExponent");

	if(MVMatrixLocation==-1){
	fprintf(stderr,"uniform MVMatrix could not be found\n");
	}

	geometryPipeline.SetMatrixStacks(modelView, projection);
	//ustawiamy kamerę
	M3DVector3f eye = {3.0f, 3.0f, 20.0f};
	M3DVector3f at = {0.0f, 0.0f, 0.0f};
	M3DVector3f up = {0.0f, 0.0f, 1.0f};
	LookAt(cameraFrame, eye, at, up);

	cameraFrame.GetCameraMatrix(mCamera);

}


void initializePolygon(){

	FILE *fvertices=fopen("geode_vertices.dat","r");
	if(fvertices==NULL) {
		fprintf(stderr,"cannot open vertices file for reading\n");
		exit(-1);
	}
	char line[120];
	std::vector<float> vertices;
	while(fgets(line,120,fvertices)!=NULL) {
		float x,y,z;
		double norm;
		sscanf(line,"%f %f %f",&x,&y,&z);
		norm=x*x+y*y+z*z;
		norm=sqrt(norm);
		n_vertices++;
		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(z);
		vertices.push_back(1.0f);
		vertices.push_back(x/norm);
		vertices.push_back(y/norm);
		vertices.push_back(z/norm);
	}
	fprintf(stderr,"nv = %u %u\n",n_vertices,vertices.size());


	FILE *ffaces=fopen("geode_faces.dat","r");
	if(ffaces==NULL) {
		fprintf(stderr,"cannot open faces file for reading\n");
		exit(-1);
	}
	std::vector<GLuint> faces;
	while(fgets(line,120,ffaces)!=NULL) {
		GLuint  i,j,k;
		if(3!=sscanf(line,"%u %u %u",&i,&j,&k)){
			fprintf(stderr,"error reading faces\n"); 
			exit(-1);
		}
		//fprintf(stderr,"%u %u %u\n",i-1,j-1,k-1);
		n_faces++;
		faces.push_back(i-1);
		faces.push_back(j-1);
		faces.push_back(k-1);  
	}
 	fprintf(stderr,"nf = %u\n",n_faces);

	GLuint vertex_buffer;
	glGenBuffers(1,&vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	glBufferData(GL_ARRAY_BUFFER,n_vertices*sizeof(float)*7,&vertices[0],GL_STATIC_DRAW);
	if(glGetError()!=GL_NO_ERROR) {
		fprintf(stderr,"error copying vertices\n");
	}

	glVertexAttribPointer(GLT_ATTRIBUTE_VERTEX,4,GL_FLOAT,GL_FALSE,sizeof(float)*7,(const GLvoid *)0);

	glVertexAttribPointer(GLT_ATTRIBUTE_NORMAL,3,GL_FLOAT,GL_FALSE,
	sizeof(float)*7,(const GLvoid *)(4*sizeof(float)) );

	glEnableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);
	glEnableVertexAttribArray(GLT_ATTRIBUTE_NORMAL);

	GLuint faces_buffer;
	glGenBuffers(1,&faces_buffer);
	if(glGetError()!=GL_NO_ERROR) {
		fprintf(stderr,"faces_buffer invalid\n");
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,faces_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,n_faces*sizeof(GLuint)*3,&faces[0],GL_STATIC_DRAW);
	if(glGetError()!=GL_NO_ERROR) {
		fprintf(stderr,"error copying faces\n");
	}
}
void RenderScene(void) {
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glFrontFace(GL_CW);
    glUseProgram(shader);
	
	//animacja sceny
	M3DVector3f at={0,0,0};
	M3DVector3f up={0,0,1};
	M3DVector3f eye;
	angle =timer.GetElapsedSeconds()*(3.14/8);
	eye[0]=6.0f*cos(angle);
	eye[1]=6.0f*sin(angle);
	eye[2]=20.0f; 
	LookAt(cameraFrame,eye,at,up);

	//light
	M3DVector3f ambientLight = {1.0f, 1.0f, 1.0f};
	M3DVector3f position = {10.0f, 10.0f, 3.0f};
	M3DVector3f color = {1.0f, 1.0f, 1.0f};
	float lightAngle = 90.0f;
	float attenuation0 = 0.01f;
	float attenuation1 = 0.01f;
	float attenuation2 = 0.01f;
	M3DVector3f ambientColor = {0.2f, 0.2f, 0.2f};
	M3DVector3f diffuseColor = {5.0f, 0.0f, 5.0f};
	M3DVector3f specularColor = {1.0f, 1.0f, 1.0f};
	float specularExponent = 5;



	projection.LoadMatrix(viewFrustum.GetProjectionMatrix());
	modelView.PushMatrix();
	cameraFrame.GetCameraMatrix(mCamera);
	modelView.LoadMatrix(mCamera);
	modelView.PushMatrix();
	
	glUniformMatrix4fv(MVMatrixLocation, 1, GL_FALSE, geometryPipeline.GetModelViewMatrix());
	glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, geometryPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, geometryPipeline.GetNormalMatrix());
	glUniform3fv(ambientLightLocation, 1, ambientLight);
	glUniform3fv(shaderPositionLocation, 1, position);
	glUniform3fv(shaderColorLocation, 1, color);
	glUniform1f(shaderAngleLocation, lightAngle);
	glUniform1f(shaderAttenuation0Location, attenuation0);
	glUniform1f(shaderAttenuation1Location, attenuation1);
	glUniform1f(shaderAttenuation2Location, attenuation2);
	glUniform3fv(shaderAmbientColorLocation, 1, ambientColor);
	glUniform3fv(shaderDiffuseColorLocation, 1, diffuseColor2);
	glUniform3fv(shaderSpecularColorLocation, 1, specularColor);
	glUniform1f(shaderSpecularExponentLocation, specularExponent);

	//rysujemy podłogę
	glPolygonOffset(1.0f, 1.0f);
	drawGrid(); 
	glEnable(GL_POLYGON_OFFSET_FILL);
	drawFloor();
	glDisable(GL_POLYGON_OFFSET_FILL);

	angle = timer.GetElapsedSeconds()*(3.14f/4);
	modelView.Translate(0.0f, 0.0f, 1.0f);
	glUniformMatrix4fv(MVMatrixLocation, 1, GL_FALSE, geometryPipeline.GetModelViewMatrix());
	glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, geometryPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, geometryPipeline.GetNormalMatrix());
	glUniform3fv(ambientLightLocation, 1, ambientLight);
	glUniform3fv(shaderPositionLocation, 1, position);
	glUniform3fv(shaderColorLocation, 1, color);
	glUniform1f(shaderAngleLocation, lightAngle);
	glUniform1f(shaderAttenuation0Location, attenuation0);
	glUniform1f(shaderAttenuation1Location, attenuation1);
	glUniform1f(shaderAttenuation2Location, attenuation2);
	glUniform3fv(shaderAmbientColorLocation, 1, ambientColor);
	glUniform3fv(shaderDiffuseColorLocation, 1, diffuseColor);
	glUniform3fv(shaderSpecularColorLocation, 1, specularColor);
	glUniform1f(shaderSpecularExponentLocation, specularExponent);
	drawTriangles(20, ico_vertices, ico_faces);

	modelView.PopMatrix();
	modelView.PushMatrix();
	modelView.Scale(1.5f, 1.5f, 1.5f);
	modelView.Translate(5.0f, -1.0f, 0.0f);
	modelView.Rotate(45, 0.0f, 0.0f, 1.0f);
	glUniformMatrix4fv(MVMatrixLocation, 1, GL_FALSE, geometryPipeline.GetModelViewMatrix());
	glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, geometryPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, geometryPipeline.GetNormalMatrix());
	glUniform3fv(ambientLightLocation, 1, ambientLight);
	glUniform3fv(shaderPositionLocation, 1, position);
	glUniform3fv(shaderColorLocation, 1, color);
	glUniform1f(shaderAngleLocation, lightAngle);
	glUniform1f(shaderAttenuation0Location, attenuation0);
	glUniform1f(shaderAttenuation1Location, attenuation1);
	glUniform1f(shaderAttenuation2Location, attenuation2);
	glUniform3fv(shaderAmbientColorLocation, 1, ambientColor);
	glUniform3fv(shaderDiffuseColorLocation, 1, diffuseColor);
	glUniform3fv(shaderSpecularColorLocation, 1, specularColor);
	glUniform1f(shaderSpecularExponentLocation, specularExponent);
	drawTriangles(20, ico_vertices, ico_faces);

	modelView.PopMatrix();
	modelView.PushMatrix();
	modelView.Scale(1.5f, 1.5f, 1.5f);
	modelView.Translate(-5.0f, 1.0f, 0.0f);
	modelView.Rotate(angle*180, 0.0f, 0.0f, 1.0f);
	glUniformMatrix4fv(MVMatrixLocation, 1, GL_FALSE, geometryPipeline.GetModelViewMatrix());
	glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, geometryPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, geometryPipeline.GetNormalMatrix());
	glUniform3fv(ambientLightLocation, 1, ambientLight);
	glUniform3fv(shaderPositionLocation, 1, position);
	glUniform3fv(shaderColorLocation, 1, color);
	glUniform1f(shaderAngleLocation, lightAngle);
	glUniform1f(shaderAttenuation0Location, attenuation0);
	glUniform1f(shaderAttenuation1Location, attenuation1);
	glUniform1f(shaderAttenuation2Location, attenuation2);
	glUniform3fv(shaderAmbientColorLocation, 1, ambientColor);
	glUniform3fv(shaderDiffuseColorLocation, 1, diffuseColor);
	glUniform3fv(shaderSpecularColorLocation, 1, specularColor);
	glUniform1f(shaderSpecularExponentLocation, specularExponent);
	drawSmoothTriangles(20, ico_vertices, ico_faces);

	modelView.PopMatrix();
	modelView.PushMatrix();
	modelView.Scale(2.0f, 2.0f, 2.0f);
	modelView.Translate(2.0f, 3.0f, 0.0f);
	modelView.Rotate(angle*180, 0.0f, 0.0f, 1.0f);
	glUniformMatrix4fv(MVMatrixLocation, 1, GL_FALSE, geometryPipeline.GetModelViewMatrix());
	glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, geometryPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, geometryPipeline.GetNormalMatrix());
	glUniform3fv(ambientLightLocation, 1, ambientLight);
	glUniform3fv(shaderPositionLocation, 1, position);
	glUniform3fv(shaderColorLocation, 1, color);
	glUniform1f(shaderAngleLocation, lightAngle);
	glUniform1f(shaderAttenuation0Location, attenuation0);
	glUniform1f(shaderAttenuation1Location, attenuation1);
	glUniform1f(shaderAttenuation2Location, attenuation2);
	glUniform3fv(shaderAmbientColorLocation, 1, ambientColor);
	glUniform3fv(shaderDiffuseColorLocation, 1, diffuseColor);
	glUniform3fv(shaderSpecularColorLocation, 1, specularColor);
	glUniform1f(shaderSpecularExponentLocation, specularExponent);
	glDrawElements(GL_TRIANGLES,3*n_faces,GL_UNSIGNED_INT,0);

	modelView.PopMatrix();	
	modelView.PopMatrix();
    glutSwapBuffers();
	glutPostRedisplay();
}

int main(int argc, char* argv[]) {

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Projekt5");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }

    SetupRC();
	initializePolygon();
    glutMainLoop();
    return 0;
}

#include <Windows.h>
// Triangle.cpp
// Our first OpenGL program that will just draw a triangle on the screen.
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <GLTools.h>            // OpenGL toolkit
#include <GL/glew.h>

#ifdef __APPLE__
#include <glut/glut.h>          // OS X version of GLUT
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>            // Windows FreeGlut equivalent
#endif

#include <GLFrame.h>
#include <GLFrustum.h>
#include <StopWatch.h>
#include <vector>

CStopWatch timer; 
GLFrame cameraFrame;
GLFrustum viewFrustum;
GLint MVPMatrixLocation;
GLGeometryTransform geoemtryPipeline;
GLuint shader;
GLint shaderNormalMatrixLocation, shaderMVMatrixLocation, shaderAmbientLightLocation;
GLint shaderPositionLocation, shaderColorLocation, shaderAngleLocation, shaderAttenuation0Location, shaderAttenuation1Location, shaderAttenuation2Location;
GLint shaderMatAmbientColorLocation, shaderMatDiffuseColorLocation, shaderMatSpecularColorLocation, shaderMatSpecularExponentLocation; 
std::vector<float> vertices;
GLuint n_vertices;
std::vector<GLuint> faces;
GLuint n_faces;

long start = timeGetTime();

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

///////////////////////////////////////////////////////////////////////////////
// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.
void SetUpFrame(GLFrame &frame,const M3DVector3f origin,
				const M3DVector3f forward,
				const M3DVector3f up) {
					frame.SetOrigin(origin);
					frame.SetForwardVector(forward);
M3DVector3f side,oUp;
m3dCrossProduct3(side,forward,up);
m3dCrossProduct3(oUp,side,forward);
frame.SetUpVector(oUp);
frame.Normalize();
};

void LookAt(GLFrame &frame, const M3DVector3f eye,
        const M3DVector3f at,
        const M3DVector3f up) {
    M3DVector3f forward;
    m3dSubtractVectors3(forward, at, eye);
    SetUpFrame(frame, eye, forward, up);
}
void ChangeSize(int w, int h) {
    glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(30.f, static_cast<float>(w)/h, 3.f, 35.f);
}

void LoadVer()
{
   vertices.clear();
   n_vertices=0;
   FILE *fvertices=fopen("geode_vertices.dat","r");
   if(fvertices==NULL) {
   fprintf(stderr,"cannot open vertices file for reading\n");
   exit(-1);
   }
   char line[120];
   
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
   vertices.push_back(1.0f);   vertices.push_back(x/norm);
   vertices.push_back(y/norm);
   vertices.push_back(z/norm);
   } fprintf(stderr,"nv = %u %u\n",n_vertices,vertices.size());
}

void LoadFaces()
{
   faces.clear();
   n_faces=0;
   FILE *ffaces=fopen("geode_faces.dat","r");
   if(ffaces==NULL) {
   fprintf(stderr,"cannot open faces file for reading\n");
   exit(-1);
   }

   char line[120];
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
   
   } 	fprintf(stderr,"nf = %u\n",n_faces);

}

///////////////////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering context.
// This is the first opportunity to do any OpenGL related tasks.

void SetupRC() {

    // Blue background
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

    shader = gltLoadShaderPairWithAttributes("pass_thru_shader.vp", "pass_thru_shader.fp",
            2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal");
    fprintf(stdout, "GLT_ATTRIBUTE_VERTEX : %d\nGLT_ATTRIBUTE_COLOR : %d \n",
            GLT_ATTRIBUTE_VERTEX, GLT_ATTRIBUTE_COLOR);
	MVPMatrixLocation=glGetUniformLocation(shader,"MVPMatrix");
    if(MVPMatrixLocation==-1)
    {
        fprintf(stderr,"uniform MVPMatrix could not be found\n");
    }

	shaderNormalMatrixLocation = glGetUniformLocation(shader, "normalMatrix");
	if(shaderNormalMatrixLocation==-1) {fprintf(stderr,"uniform normalMatrix could not be found\n");}
	shaderMVMatrixLocation = glGetUniformLocation(shader, "MVMatrix");
	if(shaderMVMatrixLocation==-1) {fprintf(stderr,"uniform MVMatrix could not be found\n");}
	shaderAmbientLightLocation = glGetUniformLocation(shader, "ambientLight");
	if(shaderAmbientLightLocation==-1) {fprintf(stderr,"uniform ambientLight could not be found\n");}
	shaderPositionLocation = glGetUniformLocation(shader, "light1.position");
	if(shaderPositionLocation==-1) {fprintf(stderr,"uniform shaderPositionLocation could not be found\n");}
	shaderColorLocation = glGetUniformLocation(shader, "light1.color");
	if(shaderColorLocation==-1) {fprintf(stderr,"uniform shaderColorLocation could not be found\n");}
	shaderAttenuation0Location = glGetUniformLocation(shader, "light1.attenuation0");
	if(shaderAttenuation0Location==-1) {fprintf(stderr,"uniform shaderAttenuation0Location could not be found\n");}
	shaderAttenuation1Location = glGetUniformLocation(shader, "light1.attenuation1");
	if(shaderAttenuation1Location==-1) {fprintf(stderr,"uniform shaderAttenuation1Location could not be found\n");}
	shaderAttenuation2Location = glGetUniformLocation(shader, "light1.attenuation2");
	if(shaderAttenuation2Location==-1) {fprintf(stderr,"uniform shaderAttenuation2Location could not be found\n");}
	shaderMatAmbientColorLocation = glGetUniformLocation(shader, "material.ambientColor");
	if(shaderMatAmbientColorLocation==-1) {fprintf(stderr,"uniform shaderMatAmbientColorLocation could not be found\n");}
    shaderMatDiffuseColorLocation = glGetUniformLocation(shader, "material.diffuseColor");
	if(shaderMatDiffuseColorLocation==-1) {fprintf(stderr,"uniform shaderMatDiffuseColorLocation could not be found\n");}
    shaderMatSpecularColorLocation = glGetUniformLocation(shader, "material.specularColor");
	if(shaderMatSpecularColorLocation==-1) {fprintf(stderr,"uniform shaderMatSpecularColorLocation could not be found\n");}
    shaderMatSpecularExponentLocation = glGetUniformLocation(shader, "material.specularExponent");
	if(shaderMatSpecularExponentLocation==-1) {fprintf(stderr,"uniform shaderMatSpecularExponentLocation could not be found\n");}

	LoadVer();
	LoadFaces();

	glEnable(GL_DEPTH_TEST);
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

void drawTriangles(int n_faces, float *vertices, int *faces) {
      for (int i = 0; i < n_faces; i++) {
      glBegin(GL_TRIANGLES);
      TriangleFace(vertices + 3 * faces[3 * i], vertices + 3 * faces[3 * i + 1], vertices + 3 * faces[3 * i + 2]);
      glEnd();
   }
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


void SetLight()
{
	glUniform3f(shaderAmbientLightLocation, 0.1f, 0.1f, 0.1f);
	M3DVector3f position={0.f, 3.f, 4.f}, ePosition;
	M3DMatrix44f camMat;
	cameraFrame.GetCameraMatrix(camMat);
	m3dTransformVector3(ePosition, position, camMat);
	glUniform3fv(shaderPositionLocation, 1, ePosition);
	glUniform3f(shaderColorLocation, 1.f, 0.7f, 0.7f);
	glUniform1f(shaderAttenuation0Location, 0.025f);
	glUniform1f(shaderAttenuation1Location, 0.025f);
	glUniform1f(shaderAttenuation2Location, 0.025f);
}

void drawGrid(void)
{
	glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, geoemtryPipeline.GetModelViewProjectionMatrix());

	glUniformMatrix4fv(shaderMVMatrixLocation, 1, GL_FALSE, geoemtryPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(shaderNormalMatrixLocation, 1, GL_FALSE, geoemtryPipeline.GetNormalMatrix());

	glUniform3f(shaderMatAmbientColorLocation, 0.9f, 0.9f, 0.9f);
	glUniform3f(shaderMatDiffuseColorLocation, 0.9f, 0.9f, 0.9f);
	glUniform3f(shaderMatSpecularColorLocation, 0.5f, 0.5f, 0.5f);
	glUniform1f(shaderMatSpecularExponentLocation, 256.f);

	M3DVector3f normal={0.f, 0.f, 1.f};
	glVertexAttrib3fv(GLT_ATTRIBUTE_NORMAL, normal);
	for(int i=0; i<=40; ++i)
	{
		glBegin(GL_LINES);
		glVertex3f(-20+i, -20, 0);
		glVertex3f(-20+i, 20, 0);
		glEnd();
		glBegin(GL_LINES);
		glVertex3f(-20, -20+i, 0);
		glVertex3f(20, -20+i, 0);
		glEnd();
	}
}

void drawFloor(void)
{
	glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, geoemtryPipeline.GetModelViewProjectionMatrix());

	glUniformMatrix4fv(shaderMVMatrixLocation, 1, GL_FALSE, geoemtryPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(shaderNormalMatrixLocation, 1, GL_FALSE, geoemtryPipeline.GetNormalMatrix());

	glUniform3f(shaderMatAmbientColorLocation, 0.8f, 0.2f, 0.2f);
	glUniform3f(shaderMatDiffuseColorLocation, 0.8f, 0.2f, 0.2f);
	glUniform3f(shaderMatSpecularColorLocation, 0.7f, 0.7f, 0.7f);
	glUniform1f(shaderMatSpecularExponentLocation, 256.f);

	M3DVector3f normal={0.f, 0.f, 1.f};
	glVertexAttrib3fv(GLT_ATTRIBUTE_NORMAL, normal);

	glBegin(GL_QUADS);
	glVertex3f(20, 20, 0);
	glVertex3f(-20, 20, 0);
	glVertex3f(-20, -20, 0);
	glVertex3f(20, -20, 0);
	glEnd();
}
void RenderScene(void) {
	GLMatrixStack modelView;
    GLMatrixStack projection;
    geoemtryPipeline.SetMatrixStacks(modelView, projection);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glUseProgram(shader);
	float angle=timer.GetElapsedSeconds()*3.14f;
	M3DVector3f pos;

	pos[0]=6.8f*cos(angle);
	pos[1]=6.f*sin(angle);
	pos[2]=5.f;
	M3DVector3f at={0.f, 0.f, 0.f};
	M3DVector3f up={0.f, 1.f, 0.f};
	LookAt(cameraFrame, pos, at, up); 
	projection.LoadMatrix(viewFrustum.GetProjectionMatrix());
	M3DMatrix44f cam_mat; 
	cameraFrame.GetCameraMatrix(cam_mat); 

	modelView.LoadMatrix(cam_mat);
	
	SetLight();

	modelView.PushMatrix();
	glPolygonOffset(1.f, 1.f);
	drawGrid();
	glEnable(GL_POLYGON_OFFSET_FILL);
	drawFloor();
	glDisable(GL_POLYGON_OFFSET_FILL);
	
	modelView.PopMatrix();

	{
		modelView.PushMatrix();
		modelView.Translate(0.f, 0.f, 0.f);
		glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, geoemtryPipeline.GetModelViewProjectionMatrix());
		glUniformMatrix4fv(shaderMVMatrixLocation, 1, GL_FALSE, geoemtryPipeline.GetModelViewMatrix());
		glUniformMatrix3fv(shaderNormalMatrixLocation, 1, GL_FALSE, geoemtryPipeline.GetNormalMatrix());
		glUniform3f(shaderMatAmbientColorLocation, 0.1f, 0.1f, 0.1f);
		glUniform3f(shaderMatDiffuseColorLocation, 0.1f, 0.1f, 1.f);
		glUniform3f(shaderMatSpecularColorLocation, 0.7f, 0.7f, 0.7f);
		glUniform1f(shaderMatSpecularExponentLocation, 256.f);

		GLuint vertex_buffer;
		glGenBuffers(1,&vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER,n_vertices*sizeof(float)*7,&vertices[0],GL_STATIC_DRAW);
		if(glGetError()!=GL_NO_ERROR) {
		fprintf(stderr,"error copying vertices\n");
		}
		glVertexAttribPointer(GLT_ATTRIBUTE_VERTEX,4,GL_FLOAT,GL_FALSE,sizeof(float)*7,(const GLvoid *)0);
		glVertexAttribPointer(GLT_ATTRIBUTE_NORMAL,3,GL_FLOAT,GL_FALSE,sizeof(float)*7,(const GLvoid *)(4*sizeof(float)) );
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
		glDrawElements(GL_TRIANGLES,3*n_faces,GL_UNSIGNED_INT,0);
		modelView.PopMatrix();
	}

	modelView.PopMatrix();
	glutSwapBuffers();

    glutPostRedisplay();
}


int main(int argc, char* argv[]) {
  

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Triangle");
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

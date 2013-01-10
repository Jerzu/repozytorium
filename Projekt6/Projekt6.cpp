/**
*@ author: Mateusz Fabian
*@ title: Projekt6
**/
#include <Windows.h>
#include <GL/glew.h> 
#include <GLTools.h>            
#include <GLFrustum.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>


#ifdef __APPLE__
#include <glut/glut.h>          
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>          
#endif


GLuint shader;
GLMatrixStack modelViewMatrix;
GLMatrixStack projectionMatrix;
GLGeometryTransform transformPipeline;
GLFrustum viewFrustum;
GLFrame cameraFrame;
GLint MVPMatrixLocation;
GLint MVMatrixLocation;
GLint normalMatrixLocation;
GLint textureLocation;
GLint lightPositionLocation;
GLint diffuseColorLocation;
GLint ambientColorLocation;
GLint specularColorLocation;
GLint alphaLocation;
GLuint textureID[2];

void Texture2f(float s, float t) {
    glVertexAttrib2f(GLT_ATTRIBUTE_TEXTURE0, s, t);
}

void Color3f(float r, float g, float b) {
    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, r, g, b);
}

void DrawFace(float *points, GLuint p0, float s0, float t0,
        GLuint p1, float s1, float t1,
        GLuint p2, float s2, float t2,
        GLuint p3, float s3, float t3
        ) {
    Texture2f(s0, t0);
    glVertex3fv(points + 3 * p0);

    Texture2f(s1, t1);
    glVertex3fv(points + 3 * p1);

    Texture2f(s2, t2);
    glVertex3fv(points + 3 * p2);

    Texture2f(s3, t3);
    glVertex3fv(points + 3 * p3);
}

void DrawCube(float *cube) {
    glBegin(GL_QUADS);
    glVertexAttrib3f(GLT_ATTRIBUTE_NORMAL, 0.0f, 0.0f, -1.0f);
    DrawFace(cube, 7, 0.0f, 0.0f,
            6, 0.0f, 1.0f,
            5, 1.0f, 1.0f,
            4, 1.0f, 0.0f);

    glVertexAttrib3f(GLT_ATTRIBUTE_NORMAL, 1.0f, 0.0f, 0.0f);
    DrawFace(cube, 7, 0.0f, 0.0f,
            4, 1.0f, 0.0f,
            0, 1.0f, 1.0f,
            3, 0.0f, 1.0f);

    glVertexAttrib3f(GLT_ATTRIBUTE_NORMAL, -1.0f, 0.0f, 0.0f);
    DrawFace(cube, 5, 0.0f, 0.0f,
            6, 1.0f, 0.0f,
            2, 1.0f, 1.0f,
            1, 0.0f, 1.0f);

    glVertexAttrib3f(GLT_ATTRIBUTE_NORMAL, 0.0f, -1.0f, 0.0f);
    DrawFace(cube, 6, 0.0f, 0.0f,
            7, 1.0f, 0.0f,
            3, 1.0f, 1.0f,
            2, 0.0f, 1.0f);

    glVertexAttrib3f(GLT_ATTRIBUTE_NORMAL, 0.0f, 1.0f, 0.0f);
    DrawFace(cube, 4, 0.0f, 0.0f,
            5, 1.0f, 0.0f,
            1, 1.0f, 1.0f,
            0, 0.0f, 1.0f);
    glVertexAttrib3f(GLT_ATTRIBUTE_NORMAL, 0.0f, 0.0f, 1.0f);
    DrawFace(cube, 3, 0.0f, 0.0f,
            0, 1.0f, 0.0f,
            1, 1.0f, 1.0f,
            2, 0.0f, 1.0f);

    glEnd();

}

float cube[24] = {1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f};

GLint GetUniformLocation(GLuint shader_a, const char *name) {
    GLint location = glGetUniformLocation(shader_a, name);
    if (location == -1) {
        fprintf(stderr, "uniform %s could not be found\n", name);
    }
    return location;
};


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

bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode) {
    GLbyte *pBits;
    int nWidth, nHeight, nComponents;
    GLenum eFormat;

    pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
    if (pBits == NULL)
        return false;

    fprintf(stderr, "read texture from %s %dx%d\n", szFileName, nWidth, nHeight);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0,
            eFormat, GL_UNSIGNED_BYTE, pBits);

    free(pBits);

    if (minFilter == GL_LINEAR_MIPMAP_LINEAR ||
            minFilter == GL_LINEAR_MIPMAP_NEAREST ||
            minFilter == GL_NEAREST_MIPMAP_LINEAR ||
            minFilter == GL_NEAREST_MIPMAP_NEAREST)
        glGenerateMipmap(GL_TEXTURE_2D);

    return true;
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



void ChangeSize(int w, int h) {

    glViewport(0, 0, w, h);
    viewFrustum.SetPerspective(50.0f, float(w) / float(h), 1.0, 300.0);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
   
}

void SetupRC() {

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    shader = gltLoadShaderPairWithAttributes("shader.vp", "shader.fp",
            4,
            GLT_ATTRIBUTE_VERTEX, "vVertex",
            GLT_ATTRIBUTE_COLOR, "vColor",
            GLT_ATTRIBUTE_TEXTURE0, "texCoord0",
            GLT_ATTRIBUTE_NORMAL, "vNormal");

    fprintf(stdout, "GLT_ATTRIBUTE_VERTEX : %d\nGLT_ATTRIBUTE_COLOR : %d \nGLT_ATTRIBUTE_TEXTURE0 : %d\n",
            GLT_ATTRIBUTE_VERTEX, GLT_ATTRIBUTE_COLOR, GLT_ATTRIBUTE_TEXTURE0);

    glGenTextures(2, textureID);
    glBindTexture(GL_TEXTURE_2D, textureID[0]);
    if (!LoadTGATexture("tekstura2b.tga", GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE)) {
        fprintf(stderr, "error loading texture\n");
    }

    glBindTexture(GL_TEXTURE_2D, textureID[1]);
    if (!LoadTGATexture("tekstura1a.tga", GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE)) {
        fprintf(stderr, "error loading texture\n");
    }


    MVPMatrixLocation = GetUniformLocation(shader, "MVPMatrix");
    textureLocation = GetUniformLocation(shader, "texture0");
    lightPositionLocation = GetUniformLocation(shader, "lightPosition");
    diffuseColorLocation = GetUniformLocation(shader, "diffuseColor");
    ambientColorLocation = GetUniformLocation(shader, "ambientColor");
    specularColorLocation = GetUniformLocation(shader, "specularColor");
    MVMatrixLocation = GetUniformLocation(shader, "MVMatrix");
    normalMatrixLocation = GetUniformLocation(shader, "normalMatrix");
    alphaLocation=GetUniformLocation(shader, "alpha");

    M3DVector3f eye = {-25.0f, -18.0f, 10.0f};
	M3DVector3f at = {0.0f, 0.0f, 0.0f};
	M3DVector3f up = {0.0f, 0.0f, 1.0f};

    LookAt(cameraFrame, eye, at, up);
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
 
}

void ShutdownRC() {

	  glDeleteTextures(1, &textureID[0]);
	  glDeleteTextures(1, &textureID[1]);
}

void UpdateMatrixUniforms() {
    glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
    glUniformMatrix4fv(MVMatrixLocation, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
    glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, transformPipeline.GetNormalMatrix());
}


void RenderScene() {
    modelViewMatrix.PushMatrix();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, textureID[0]);
    glUseProgram(shader);

    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    modelViewMatrix.PushMatrix(mCamera);
    glUniform4f(diffuseColorLocation, 1.0f, 1.0f, 1.0f, 1.0f);
    glUniform4f(ambientColorLocation, .3f, .30f, 0.30f, 1.0f);
    glUniform4f(specularColorLocation, .4f, .4f, 0.4f, 1.0f);
    M3DVector4f lightPosition={-10.0f,20.0f, 50.0f,1.0f};
    M3DVector4f eyeLightPosition;
    m3dTransformVector4(eyeLightPosition,lightPosition,mCamera);
    glUniform3fv(lightPositionLocation,1,&eyeLightPosition[0] );
    glUniform1i(textureLocation, 0);
    modelViewMatrix.PushMatrix();
	modelViewMatrix.Translate(0.0f,0.0f,-4.0f);
	modelViewMatrix.Scale(2.0f,2.0f,2.0f);
	glBindTexture(GL_TEXTURE_2D, textureID[1]);
	modelViewMatrix.PushMatrix();
    UpdateMatrixUniforms();
    DrawCube(cube);
    modelViewMatrix.PopMatrix();
	modelViewMatrix.PopMatrix();
	modelViewMatrix.PushMatrix();
	modelViewMatrix.Translate(4.0f,4.0f,-4.0f);
	modelViewMatrix.Scale(2.0f,2.0f,2.0f);
    modelViewMatrix.PushMatrix();
	UpdateMatrixUniforms();
    DrawCube(cube);
    modelViewMatrix.PopMatrix();
    modelViewMatrix.PopMatrix();
    modelViewMatrix.Translate(0.0f,0.0f,-6.0f);
    UpdateMatrixUniforms();
    glUniform1f(alphaLocation,.45f);
    glUniform4f(specularColorLocation,0.0,0.0,0.0,0.0);
    glBindTexture(GL_TEXTURE_2D, textureID[0]);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    Texture2f (0.0,1.0);
    glVertex3f(-10.0f,-10.0f,0.0f);
    Texture2f (1.0,1.0);
    glVertex3f(-10.0f,20.0f,0.0f);
    Texture2f (1.0,0.0);
    glVertex3f(20.0f,20.0f,0.0f);
    Texture2f (0.0,0.0);
    glVertex3f(20.0f,-10.0f,0.0f);
    glEnd();
	glDisable(GL_BLEND);
    modelViewMatrix.PopMatrix();
    modelViewMatrix.PopMatrix();
    glutSwapBuffers();
}

int main(int argc, char * argv[]) {

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Projekt6");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
    }

    SetupRC();
    glutMainLoop();
    ShutdownRC();
    return 0;

}
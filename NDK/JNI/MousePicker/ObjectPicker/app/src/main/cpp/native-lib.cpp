#include <jni.h>
#include <string>
#include <vector>
#include <GLES/gl.h>
#include <GLES3/gl32.h>

#include "vmath.h"

#include <android/log.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

float gWidth, gHeight;

#include "MousePicker.h"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define LOG_TAG "MPD: ObjectPicker"
#define LOGINFO(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGERROR(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

enum {
    MPD_ATTRIBUTE_VERTEX = 0,
    MPD_ATTRIBUTE_COLOR,
    MPD_ATTRIBUTE_NORMAL,
    MPD_ATTRIBUTE_TEXTURE0,
};

GLuint gShaderProgramObject;

GLuint gVao_triangle;
GLuint gVbo_color_triangle;
GLuint gVbo_position_triangle;
GLuint gVbo_texcoord_triangle;

GLuint gVao_pyramid;
GLuint gVbo_color_pyramid;
GLuint gVbo_position_pyramid;
GLuint gVbo_texcoord_pyramid;

GLuint gVao_cube;
GLuint gVbo_color_cube;
GLuint gVbo_position_cube;
GLuint gVbo_texcoord_cube;

GLint gMUniform,gVUniform,gPUniform;
glm::mat4 perspectiveProj;

float gAngle;

MousePicker picker;
float RAY_RANGE = 600;

//std::vector<glm::vec2> mouseCoordinateStore;

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_objectpicker_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_objectpicker_GLESNativeLib_init(
        JNIEnv* env,
        jclass clazz) {


    void initialize();

    initialize();
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_objectpicker_GLESNativeLib_resize(
        JNIEnv* env,
        jclass clazz,
        jint width,
        jint height) {
    void resize(int , int);
    if(!width || !height)
    {
        width = WIN_WIDTH;
        height = WIN_HEIGHT;
    }

    gWidth = width;
    gHeight = height;

    resize((float)width, (float)height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_objectpicker_GLESNativeLib_display(
        JNIEnv* env,
        jclass clazz,
        jfloat x,
        jfloat y) {
    void display(float x, float y);
    display(x, y);

}

extern "C" JNIEXPORT void JNICALL
Java_com_example_objectpicker_GLESNativeLib_AddingTouchInputCoordinates(
        JNIEnv* env,
        jclass type,
        jfloat xCoordinate,
        jfloat yCoordinate)
{
    glm::vec2 pos = glm::vec2(xCoordinate, yCoordinate);
    LOGINFO("Ohh...touch happen....pos = %f ,%f", xCoordinate, yCoordinate);
    picker.addMouseCoordinatesToVector(xCoordinate, yCoordinate);
   // mouseCoordinateStore.push_back(pos);

}

void initialize()
{
    GLuint loadAndCompileShader(GLenum shaderType, const char *sourceCode);
    GLuint createProgramObjectAndLinkShader(GLuint vertexShaderID, GLuint fragmentShaderID);
    GLuint createProgramObject(const char *vertexSource, const char *fragmentSource);

    const GLchar *vertexShaderSource =
            "#version 320 es"\
            "\n"\
            "in vec4 vPosition;"\
            "in vec4 vColor;"\

             "uniform mat4 u_m_matrix;"\
            "uniform mat4 u_v_matrix;"\
            "uniform mat4 u_p_matrix;"\

            "out vec4 out_color;"\

            "void main(void)"\
            "{"\
            "gl_Position = u_p_matrix * u_v_matrix * u_m_matrix * vPosition;"\
            "out_color = vColor;"\
            "}";

    const GLchar *fragmentShaderSource =
            "#version 320 es"\
            "\n"\
            "precision highp float;"\
            "in vec4 out_color;"\
            "out vec4 FragColor;"\
            "void main(void){"\
             "FragColor = out_color;"\
            "}";
    gShaderProgramObject = createProgramObject(vertexShaderSource,fragmentShaderSource);

    gMUniform = glGetUniformLocation(gShaderProgramObject,"u_m_matrix");
    gVUniform = glGetUniformLocation(gShaderProgramObject,"u_v_matrix");
    gPUniform = glGetUniformLocation(gShaderProgramObject,"u_p_matrix");

    const GLfloat pyramidVertices[] =
            {
                    0, 1, 0,
                    -1, -1, 1,
                    1, -1, 1,

                    0, 1, 0,
                    1, -1, 1,
                    1, -1, -1,

                    0, 1, 0,
                    1, -1, -1,
                    -1, -1, -1,

                    0, 1, 0,
                    -1, -1, -1,
                    -1, -1, 1
            };

    const GLfloat pyramidColors[] =
            {
                    1, 0, 0,
                    0, 1, 0,
                    0, 0, 1,

                    1, 0, 0,
                    0, 0, 1,
                    0, 1, 0,

                    1, 0, 0,
                    0, 1, 0,
                    0, 0, 1,

                    1, 0, 0,
                    0, 0, 1,
                    0, 1, 0,
            };
    const GLfloat pyramidTexcoords[] =
            {
                    0.5, 1.0, // front-top
                    0.0, 0.0, // front-left
                    1.0, 0.0, // front-right

                    0.5, 1.0, // right-top
                    1.0, 0.0, // right-left
                    0.0, 0.0, // right-right

                    0.5, 1.0, // back-top
                    1.0, 0.0, // back-left
                    0.0, 0.0, // back-right

                    0.5, 1.0, // left-top
                    0.0, 0.0, // left-left
                    1.0, 0.0, // left-right
            };
    GLfloat cubeVertices[] =
            {
                    1.0f, 1.0f, -1.0f,
                    -1.0f, 1.0f, -1.0f,
                    -1.0f, 1.0f, 1.0f,
                    1.0f, 1.0f, 1.0f,

                    1.0f, -1.0f, 1.0f,
                    -1.0f, -1.0f, 1.0f,
                    -1.0f, -1.0f, -1.0f,
                    1.0f, -1.0f, -1.0f,

                    1.0f, 1.0f, 1.0f,
                    -1.0f, 1.0f, 1.0f,
                    -1.0f, -1.0f, 1.0f,
                    1.0f, -1.0f, 1.0f,

                    1.0f, -1.0f, -1.0f,
                    -1.0f, -1.0f, -1.0f,
                    -1.0f, 1.0f, -1.0f,
                    1.0f, 1.0f, -1.0f,

                    -1.0f, 1.0f, 1.0f,
                    -1.0f, 1.0f, -1.0f,
                    -1.0f, -1.0f, -1.0f,
                    -1.0f, -1.0f, 1.0f,

                    1.0f, 1.0f, -1.0f,
                    1.0f, 1.0f, 1.0f,
                    1.0f, -1.0f, 1.0f,
                    1.0f, -1.0f, -1.0f,
            };

    const GLfloat cubeColors[] =
            {
                    0.0f, 1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,

                    1.0f, 0.5f, 0.0f,
                    1.0f, 0.5f, 0.0f,
                    1.0f, 0.5f, 0.0f,
                    1.0f, 0.5f, 0.0f,

                    1.0f, 0.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,

                    1.0f, 1.0f, 0.0f,
                    1.0f, 1.0f, 0.0f,
                    1.0f, 1.0f, 0.0f,
                    1.0f, 1.0f, 0.0f,

                    0.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f,

                    1.0f, 0.0f, 1.0f,
                    1.0f, 0.0f, 1.0f,
                    1.0f, 0.0f, 1.0f,
                    1.0f, 0.0f, 1.0f,
            };

    const GLfloat cubeTexcoords[] =
            {
                    0.0f, 0.0f,
                    1.0f, 0.0f,
                    1.0f, 1.0f,
                    0.0f, 1.0f,

                    0.0f, 0.0f,
                    1.0f, 0.0f,
                    1.0f, 1.0f,
                    0.0f, 1.0f,

                    0.0f, 0.0f,
                    1.0f, 0.0f,
                    1.0f, 1.0f,
                    0.0f, 1.0f,

                    0.0f, 0.0f,
                    1.0f, 0.0f,
                    1.0f, 1.0f,
                    0.0f, 1.0f,

                    0.0f, 0.0f,
                    1.0f, 0.0f,
                    1.0f, 1.0f,
                    0.0f, 1.0f,

                    0.0f, 0.0f,
                    1.0f, 0.0f,
                    1.0f, 1.0f,
                    0.0f, 1.0f,
            };
    glGenVertexArrays(1,&gVao_pyramid);
    glBindVertexArray(gVao_pyramid);
    glGenBuffers(1,&gVbo_position_pyramid);
    glBindBuffer(GL_ARRAY_BUFFER,gVbo_position_pyramid);
    glBufferData(GL_ARRAY_BUFFER,sizeof(pyramidVertices),NULL,GL_DYNAMIC_DRAW);
    glVertexAttribPointer(MPD_ATTRIBUTE_VERTEX,3,GL_FLOAT,GL_FALSE,0,NULL);
    glEnableVertexAttribArray(MPD_ATTRIBUTE_VERTEX);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    /*glGenBuffers(1,&gVbo_texcoord_pyramid);
     glBindBuffer(GL_ARRAY_BUFFER,gVbo_texcoord_pyramid);
     glBufferData(GL_ARRAY_BUFFER,sizeof(pyramidTexcoords),pyramidTexcoords,GL_STATIC_DRAW);
     glVertexAttribPointer(MPD_ATTRIBUTE_TEXTURE0,2,GL_FLOAT,GL_FALSE,0,NULL);
     glEnableVertexAttribArray(MPD_ATTRIBUTE_TEXTURE0);
     glBindBuffer(GL_ARRAY_BUFFER,0);
     glBindVertexArray(0);*/

    glGenBuffers(1,&gVbo_color_pyramid);
    glBindBuffer(GL_ARRAY_BUFFER,gVbo_color_pyramid);
    glBufferData(GL_ARRAY_BUFFER,sizeof(pyramidColors),pyramidColors,GL_STATIC_DRAW);
    glVertexAttribPointer(MPD_ATTRIBUTE_COLOR,3,GL_FLOAT,GL_FALSE,0,NULL);
    glEnableVertexAttribArray(MPD_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);

    glGenVertexArrays(1,&gVao_cube);
    glBindVertexArray(gVao_cube);
    glGenBuffers(1,&gVbo_position_cube);
    glBindBuffer(GL_ARRAY_BUFFER,gVbo_position_cube);
    glBufferData(GL_ARRAY_BUFFER,sizeof(cubeVertices),cubeVertices,GL_STATIC_DRAW);
    glVertexAttribPointer(MPD_ATTRIBUTE_VERTEX,3,GL_FLOAT,GL_FALSE,0,NULL);
    glEnableVertexAttribArray(MPD_ATTRIBUTE_VERTEX);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    /*glGenBuffers(1,&gVbo_texcoord_cube);
     glBindBuffer(GL_ARRAY_BUFFER,gVbo_texcoord_cube);
     glBufferData(GL_ARRAY_BUFFER,sizeof(cubeTexcoords),cubeTexcoords,GL_STATIC_DRAW);
     glVertexAttribPointer(MPD_ATTRIBUTE_TEXTURE0,2,GL_FLOAT,GL_FALSE,0,NULL);
     glEnableVertexAttribArray(MPD_ATTRIBUTE_TEXTURE0);
     glBindBuffer(GL_ARRAY_BUFFER,0);
     glBindVertexArray(0);*/

    glGenBuffers(1,&gVbo_color_cube);
    glBindBuffer(GL_ARRAY_BUFFER,gVbo_color_cube);
    glBufferData(GL_ARRAY_BUFFER,sizeof(cubeColors),cubeColors,GL_STATIC_DRAW);
    glVertexAttribPointer(MPD_ATTRIBUTE_COLOR,3,GL_FLOAT,GL_FALSE,0,NULL);
    glEnableVertexAttribArray(MPD_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);

    /*Triangle*/
    const GLfloat triangleVertices[] = {
            0.0f,1.0f,0.0f,//apex
            -1.0f,-1.0f,0.0f,//lb
            1.0f,-1.0f,0.0f//rb
    };
    const GLfloat triangleColors[] = {
            1.0f,0.0f,0.0f,
            0.0f,1.0f,0.0f,
            0.0f,0.0f,1.0f
    };
    const GLfloat triangleTexcoords[] ={
            0.5, 1.0, // front-top
            0.0, 0.0, // front-left
            1.0, 0.0, // front-right
    };

    glGenVertexArrays(1,&gVao_triangle);
    glBindVertexArray(gVao_triangle);

    glGenBuffers(1,&gVbo_position_triangle);
    glBindBuffer(GL_ARRAY_BUFFER,gVbo_position_triangle);
    glBufferData(GL_ARRAY_BUFFER,sizeof(triangleVertices),triangleVertices,GL_STATIC_DRAW);
    glVertexAttribPointer(MPD_ATTRIBUTE_VERTEX,3,GL_FLOAT,GL_FALSE,0,NULL);
    glEnableVertexAttribArray(MPD_ATTRIBUTE_VERTEX);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    /*glGenBuffers(1,&gVbo_texcoord_triangle);
     glBindBuffer(GL_ARRAY_BUFFER,gVbo_texcoord_triangle);
     glBufferData(GL_ARRAY_BUFFER,sizeof(triangleTexcoords),triangleTexcoords,GL_STATIC_DRAW);
     glVertexAttribPointer(MPD_ATTRIBUTE_TEXTURE0,2,GL_FLOAT,GL_FALSE,0,NULL);
     glEnableVertexAttribArray(MPD_ATTRIBUTE_TEXTURE0);
     glBindBuffer(GL_ARRAY_BUFFER,0);
     glBindVertexArray(0);*/

    glGenBuffers(1,&gVbo_color_triangle);
    glBindBuffer(GL_ARRAY_BUFFER,gVbo_color_triangle);
    glBufferData(GL_ARRAY_BUFFER,sizeof(triangleColors),triangleColors,GL_STATIC_DRAW);
    glVertexAttribPointer(MPD_ATTRIBUTE_COLOR,3,GL_FLOAT,GL_FALSE,0,NULL);
    glEnableVertexAttribArray(MPD_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);

    glClearDepthf(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);

    glClearColor(0.1f,0.2f,0.3f,0.0f);

    perspectiveProj = glm::mat4(1.0);
}

void resize(int width, int height)
{
    glViewport(0,0,(GLsizei)width,(GLsizei)height);

    if(height==0)
        height=1;

    perspectiveProj = glm::perspective(45.0f,(GLfloat)gWidth / (GLfloat)gHeight,0.1f,1000.0f);
    gWidth = width;
    gHeight = height;

}

void display(float xCoord, float yCoord) {
    void update();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gShaderProgramObject);

    glm::mat4 view = glm::mat4(1.0);

    picker.setCurrentMatrices(view, perspectiveProj);
    glUniformMatrix4fv(gVUniform, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(gPUniform, 1, GL_FALSE, &perspectiveProj[0][0]);
    for (int i = 0; i < picker.mouseCoordinateStore.size(); i++)
    {
        glm::vec3 currentRay  = picker.calculateMouseRay(picker.mouseCoordinateStore[i].x, picker.mouseCoordinateStore[i].y);
        glm::vec3 currentIntersectionPoint;
        if(picker.intersectionInRange(0, RAY_RANGE, currentRay))
        {
            currentIntersectionPoint = picker.binarySearch(0, 0, RAY_RANGE, currentRay);
        } else{
            currentIntersectionPoint = glm::vec3(0);
        }

        glm::mat4 modelMatrixNew = glm::mat4(1.0f);
        modelMatrixNew = glm::translate(modelMatrixNew, glm::vec3(currentIntersectionPoint.x,  currentIntersectionPoint.y, currentIntersectionPoint.z));
        //modelMatrixNew = glm::rotate(modelMatrixNew, gAngle, glm::vec3(0.0f,1.0f,0.0f));
        modelMatrixNew = glm::scale(modelMatrixNew, glm::vec3(0.75, 0.75, 0.75));
        //modelMatrixNew *= scaleMatrix;

        glUniformMatrix4fv(gMUniform, 1, GL_FALSE, &modelMatrixNew[0][0]);

       glBindVertexArray(gVao_triangle);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        /*glBindVertexArray(gVao_cube);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

        glBindVertexArray(0);*/
    }
    glUseProgram(0);

    update();
}

void update()
{
    gAngle +=0.1f;
    if(gAngle >= 360.0f)
        gAngle =0.0f;
}

GLuint loadAndCompileShader(GLenum shaderType, const char *sourceCode) {
    GLuint shaderID = glCreateShader(shaderType);
    if(shaderID){
        glShaderSource(shaderID,1,&sourceCode,NULL);
        glCompileShader(shaderID);

        GLint iInfoLogLength = 0;
        GLint iShaderCompiledStatus= 0;
        char *szInfoLog = NULL;
        glGetShaderiv(shaderID,GL_COMPILE_STATUS,&iShaderCompiledStatus);
        if(iShaderCompiledStatus==GL_FALSE){
            glGetShaderiv(shaderID,GL_INFO_LOG_LENGTH,&iInfoLogLength);
            if(iInfoLogLength>0){
                szInfoLog =(char*)malloc(iInfoLogLength);
                if(szInfoLog!=NULL){
                    GLsizei written;
                    glGetShaderInfoLog(shaderID,iInfoLogLength,&written,szInfoLog);
                    LOGERROR("Could not compile shader %d:\n%s\n",shaderType,szInfoLog);
                    free(szInfoLog);
                }
                glDeleteShader(shaderID);
                shaderID=0;
            }
        }

    }
    return shaderID;
}

GLuint createProgramObjectAndLinkShader(GLuint vertexShaderID, GLuint fragmentShaderID) {
    if(!vertexShaderID || !fragmentShaderID)
        return 0;

    GLuint program = glCreateProgram();

    if(program){
        glAttachShader(program,vertexShaderID);
        glAttachShader(program,fragmentShaderID);
        glBindAttribLocation(program, MPD_ATTRIBUTE_VERTEX, "vPosition");
        // glBindAttribLocation(program, MPD_ATTRIBUTE_TEXTURE0, "vTexture0_Coord");
        glBindAttribLocation(program, MPD_ATTRIBUTE_COLOR, "vColor");
        glLinkProgram(program);

        GLint iInfoLogLength = 0;
        GLint iShaderProgramLinkStatus = 0;
        char *szInfoLog = NULL;

        glGetShaderiv(program,GL_LINK_STATUS,&iShaderProgramLinkStatus);
        if(iShaderProgramLinkStatus ==GL_FALSE){
            glGetShaderiv(program,GL_INFO_LOG_LENGTH,&iInfoLogLength);
            if(iInfoLogLength>0){
                szInfoLog = (char*)malloc(iInfoLogLength);
                if(szInfoLog!=NULL){
                    GLsizei written;
                    glGetShaderInfoLog(program,iInfoLogLength,&written,szInfoLog);
                    LOGERROR("Could not link program : \n%s\n",szInfoLog);
                    free(szInfoLog);
                }
                glDeleteProgram(program);
                program=0;
            }

        }
    }
    return program;
}

GLuint createProgramObject(const char *vertexSource, const char *fragmentSource) {
    GLuint vsID = loadAndCompileShader(GL_VERTEX_SHADER,vertexSource);
    GLuint fsID = loadAndCompileShader(GL_FRAGMENT_SHADER,fragmentSource);
    return createProgramObjectAndLinkShader(vsID,fsID);
}

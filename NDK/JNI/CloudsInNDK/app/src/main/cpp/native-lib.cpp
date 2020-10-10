#include <jni.h>
#include <string>

#include <stdio.h>
#include "vmath.h"


#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>


FILE *gpFile = NULL;

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#include <android/log.h>

#define APPNAME "MyApp"


GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint gVao;
GLuint gVbo;

GLuint gVao_Cube;
GLuint gVbo_CubePosition;
GLuint gVbo_Cube_Normal;

GLuint gMVPUniform;
using namespace vmath;

float gWidth, gHeight;
mat4 gPerspectiveProjectionMatrix;

enum
{
    MALATI_ATTRIBUTE_VERTEX = 0,
    MALATI_ATTRIBUTE_COLOR,
    MALATI_ATTRIBUTE_NORMAL,
    MALATI_ATTRIBUTE_TEXTURE
};

GLuint timeUniform;

GLuint iResolutionUniform;

extern "C" JNIEXPORT void JNICALL
Java_com_example_clouds_BlueScreen_initialize(
        JNIEnv* env,
        jobject /* this */) {
    
    void initialize();

    initialize();

}

void uninitialize(void)
{
    if (gVao_Cube)
    {
        glDeleteVertexArrays(1, &gVao_Cube);
        gVao_Cube = 0;
    }

    if (gVbo_CubePosition)
    {
        glDeleteBuffers(1, &gVbo_CubePosition);
        gVbo_CubePosition = 0;
    }

    if (gVbo_Cube_Normal)
    {
        glDeleteBuffers(1, &gVbo_Cube_Normal);
        gVbo_Cube_Normal = 0;
    }
    if (gVao)
    {
        glDeleteVertexArrays(1, &gVao);
        gVao = 0;
    }
    if (gVbo)
    {
        glDeleteBuffers(1, &gVbo);
        gVbo = 0;
    }

    glDetachShader(gShaderProgramObject, gVertexShaderObject);
    glDetachShader(gShaderProgramObject, gFragmentShaderObject);

    glDeleteShader(gVertexShaderObject);
    gVertexShaderObject = 0;
    glDeleteShader(gFragmentShaderObject);
    gFragmentShaderObject = 0;

    glDeleteProgram(gShaderProgramObject);
    gShaderProgramObject = 0;

    glUseProgram(0);

}


void initialize(void)
{
    void resize(int width, int height);

    //VERTEX SHADER
    gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    const GLchar *vertextShaderSourceCode =
        "#version 320 es" \
		"\n" \
		"in vec4 vPosition;" \
		"uniform mat4 u_mvp_matrix;" \
		"void main(void)"\
		"{"\
		"gl_Position = u_mvp_matrix * vPosition;" \
		"}";

    glShaderSource(gVertexShaderObject, 1, (const GLchar**)&vertextShaderSourceCode, NULL);

    //compile shader
    glCompileShader(gVertexShaderObject);

    GLint iInfoLogLength = 0;
    GLint iShaderCompiledStatus = 0;
    char *szInfoLog = NULL;

    glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
    if (iShaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if (iInfoLogLength > 0)
        {
            szInfoLog = (char *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(gVertexShaderObject, iInfoLogLength, &written, szInfoLog);
              
                printf("MPD : Vertex Shader Compilation Log : %s\n", szInfoLog);
              
                free(szInfoLog);
                uninitialize();
                exit(0);
            }
        }
    }
    //FRAGMENT SHADER

    gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    const GLchar *fragmentShaderSourceCode =
        "#version 320 es" \
		"\n" \
		"precision lowp int;" \
		"precision highp float;" \
		/*"precision mediump float;" \*/
		"const float cloudscale = 1.0;" \
		"const float speed = 0.03;"\
		"const float clouddark = 0.5;"\
		"const float cloudlight = 0.3;"\
		"const float cloudcover = 0.2;"\
		"const float cloudalpha = 8.0;"\
		"const float skytint = 0.5;"\
		"const vec3 skycolour1 = vec3(0.2, 0.4, 0.6);"\
		"const vec3 skycolour2 = vec3(0.4, 0.7, 1.0);"\

		"const mat2 m = mat2(1.6, 1.2, -1.2, 1.6);"\
		"uniform vec2 iResolution;" \
		"uniform float iTime; " \

		"vec2 hash(vec2 p)"\
		"{"\
		"p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));"\
		"return -1.0 + 2.0*fract(sin(p)*43758.5453123);"\
		"}"\

		"float noise(in vec2 p)"\
		"{"\
		"const float K1 = 0.366025404;"\
		"const float K2 = 0.211324865;"\
		"vec2 i = floor(p + (p.x + p.y)*K1);"\
		"vec2 a = p - i + (i.x + i.y)*K2;"\
		"vec2 o = (a.x>a.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);"\
		"vec2 b = a - o + K2;"\
		"vec2 c = a - 1.0 + 2.0*K2;"\
		"vec3 h = max(0.5 - vec3(dot(a, a), dot(b, b), dot(c, c)), 0.0);"\
		"vec3 n = h*h*h*h*vec3(dot(a, hash(i + 0.0)), dot(b, hash(i + o)), dot(c, hash(i + 1.0)));"\
		"return dot(n, vec3(70.0));"\
		"}"\

		"float fbm(vec2 n)"\
		"{"\
		"float total = 0.0, amplitude = 0.1;"\
		"for (int i = 0; i < 7; i++)"\
		"{"\
		"total += noise(n) * amplitude;"\
		"n = m * n;"\
		"amplitude *= 0.4;"\
		"}"\
		"return total;"\
		"}"\
		//"in vec2 fragCoord;"
            "out vec4 fragColor;"\

            "void main(void)"\
		"{"\
		"vec2 p = gl_FragCoord.xy / iResolution.xy;"\
	"vec2 uv = p*vec2(iResolution.x / iResolution.y, 1.0);"\
	"float time = iTime * speed;"\
	"float q = fbm(uv * cloudscale * 0.5);"\
	"float r = 0.0;"\
	"uv *= cloudscale;"\
	"uv -= q - time;"\
	"float weight = 0.8;"\
	"for (int i = 0; i<8; i++)"\
	"{"\
		"r += abs(weight*noise(uv));"\
		"uv = m*uv + time;"\
		"weight *= 0.7;"\
	"}"\
	"float f = 0.0;"\
	"uv = p*vec2(iResolution.x / iResolution.y, 1.0);"\
	"uv *= cloudscale;"\
	"uv -= q - time;"\
	"weight = 0.7;"\
	"for (int i = 0; i<8; i++)"\
	"{"\
		"f += weight*noise(uv);"\
		"uv = m*uv + time;"\
		"weight *= 0.6;"\
	"}"\
	"f *= r + f;"\
	"float c = 0.0;"\
	"time = iTime * speed * 2.0;"\
	"uv = p*vec2(iResolution.x / iResolution.y, 1.0);"\
	"uv *= cloudscale*2.0;"\
	"uv -= q - time;"\
	"weight = 0.4;"\
	"for (int i = 0; i<7; i++)"\
	"{"\
		"c += weight*noise(uv);"\
		"uv = m*uv + time;"\
		"weight *= 0.6;"\
	"}"\
	"float c1 = 0.0;"\
	"time = iTime * speed * 3.0;"\
	"uv = p*vec2(iResolution.x / iResolution.y, 1.0);"\
	"uv *= cloudscale*3.0;"\
	"uv -= q - time;"\
	"weight = 0.4;"\
	"for (int i = 0; i<7; i++)"\
	"{"
        "c1 += abs(weight*noise(uv));"\
		"uv = m*uv + time;"\
		"weight *= 0.6;"\
	"}"\
	"c += c1;"\
	"vec3 skycolour = mix(skycolour2, skycolour1, p.y);"\
	"vec3 cloudcolour = vec3(1.1, 1.1, 0.9) * clamp((clouddark + cloudlight*c), 0.0, 1.0);"\
	"f = cloudcover + cloudalpha*f*r;"\
	"vec3 result = mix(skycolour, clamp(skytint * skycolour + cloudcolour, 0.0, 1.0), clamp(f + c, 0.0, 1.0));"\
	"fragColor = vec4(result, 1.0);"\

            "}";
    glShaderSource(gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);

    glCompileShader(gFragmentShaderObject);

    iInfoLogLength = 0;
    iShaderCompiledStatus = 0;
    szInfoLog = NULL;

    glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
    if (iShaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if (iInfoLogLength > 0)
        {
            szInfoLog = (char *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLength, &written, szInfoLog);
               
                printf("MPD : Fragment Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                uninitialize();
                exit(0);
            }
        }
    }
    //Shader Program

    gShaderProgramObject = glCreateProgram();

    glAttachShader(gShaderProgramObject, gVertexShaderObject);

    glAttachShader(gShaderProgramObject, gFragmentShaderObject);

    glBindAttribLocation(gShaderProgramObject, MALATI_ATTRIBUTE_VERTEX, "vPosition");

    glLinkProgram(gShaderProgramObject);

    GLint iShaderProgramLinkStatus = 0;
    glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);
    if (iShaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if (iInfoLogLength>0)
        {
            szInfoLog = (char *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(gShaderProgramObject, iInfoLogLength, &written, szInfoLog);
               
                printf("MPD : Shader Program Link Log : %s\n", szInfoLog);
               
                free(szInfoLog);
                uninitialize();
                exit(0);
            }
        }
    }

    iResolutionUniform = glGetUniformLocation(gShaderProgramObject, "iResolution");
    gMVPUniform = glGetUniformLocation(gShaderProgramObject, "u_mvp_matrix");
    timeUniform = glGetUniformLocation(gShaderProgramObject, "iTime");
  
    const GLfloat squareVertices[] =
            {
                    ////front
                    1.0f, 1.0f, 1.0f,
                    -1.0f, 1.0f, 1.0f,
                    -1.0f, -1.0f, 1.0f,
                    1.0f, -1.0f, 1.0f,


            };

    glGenVertexArrays(1, &gVao_Cube);
    glBindVertexArray(gVao_Cube);

    // position vbo
    glGenBuffers(1, &gVbo_CubePosition);
    glBindBuffer(GL_ARRAY_BUFFER, gVbo_CubePosition);
    glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(MALATI_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(MALATI_ATTRIBUTE_VERTEX);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    const GLfloat triangleVertices[] =
            {	0.0f, 0.5f, 0.0f, // Apex
                 -0.5f, -0.5f, 0.0f, // left bottom
                 0.5f, -0.5f, 0.0f // right bottom
            };

    glGenVertexArrays(1, &gVao);
    glBindVertexArray(gVao);

    glGenBuffers(1, &gVbo);
    glBindBuffer(GL_ARRAY_BUFFER, gVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(MALATI_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(MALATI_ATTRIBUTE_VERTEX);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glClearDepthf(1.0);
    // enable depth testing
    glEnable(GL_DEPTH_TEST);
    // depth test to do
    glDepthFunc(GL_LEQUAL);

    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_CULL_FACE);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // blue

    gPerspectiveProjectionMatrix  = mat4::identity();

    // resize
    resize(WIN_WIDTH, WIN_HEIGHT);
}

void display(void)
{
    static float itime;

    itime = itime + 0.04f;
    
    if (itime > 360.0)
        itime = 0.0f;


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ///////////////////////////

    glUseProgram(gShaderProgramObject);

    glUniform2f(iResolutionUniform, 3040, 1440.0f);
    glUniform1f(timeUniform, itime);


    //OpenGL Drawing
    mat4 modelmatrix = mat4::identity();
    mat4 modelViewMatrix = mat4::identity();
    mat4 rotationMatrix = mat4::identity();
    mat4 viewmatrix = mat4::identity();
    mat4 modelviewProjectionMatrix = mat4::identity();



    modelviewProjectionMatrix = gPerspectiveProjectionMatrix * viewmatrix;	//ORDER IS IMPORTANT

    glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelviewProjectionMatrix);

    glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, viewmatrix);

    glBindVertexArray(gVao_Cube);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
   
    glBindVertexArray(0);

    printf("MALATI : In Display..\n");

    glUseProgram(0);

}

void resize(int width, int height)
{
    if (height == 0)
        height = 1;
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    gWidth = width;
    gHeight = height;
/*	if (width <= height)
	{
		gOrthographicProjectionMatrix = ortho(-100.0f, 100.0f, (-100.0f *(height / width)), (100.0f * (height / width)), -100.0f, 100.0f);
	}
	else
	{
		gOrthographicProjectionMatrix = ortho((-100.0f *(width/height)), (100.0f * (width/height)),-100.0f,100.0f, -100.0f, 100.0f);
	}*/

    gPerspectiveProjectionMatrix = perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);


}

extern "C" JNIEXPORT void JNICALL
Java_com_example_clouds_BlueScreen_resize(
        JNIEnv* env,
        jobject /* this */,
        jint width,
        jint height) {
   
    if(!width || !height)
    {
        width = WIN_WIDTH;
        height = WIN_HEIGHT;
    }
    resize((float)width, (float)height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_clouds_BlueScreen_draw(
        JNIEnv* env,
        jobject /* this */) {
   

    display();
}

#include <jni.h>
#include <string>

#include "vmath.h"
#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

using namespace vmath;

enum
{
	MALATI_ATTRIBUTE_VERTEX = 0,
	MALATI_ATTRIBUTE_COLOR,
	MALATI_ATTRIBUTE_NORMAL,
	MALATI_ATTRIBUTE_TEXTURE,
	
	MALATI_ATTRIBUTE_BIRD_POSITION,
	MALATI_ATTRIBUTE_BIRD_VELOCITY,
};

enum
{
	PARTICLE_GROUP_SIZE = 1024,
	PARTICLE_GROUP_COUNT = 8192,
	PARTICLE_COUNT = (PARTICLE_GROUP_SIZE * PARTICLE_GROUP_COUNT),
	MAX_ATTRACTOR = 64
};

GLuint gComputeShaderObject;
GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;

GLuint gShaderProgramObject;
GLuint gComputeProgramObject;

GLuint gVao;
GLuint gVbo;
GLuint gMVPUniform;

GLuint render_vao;
GLuint render_vbo;

mat4 gPerspectiveProjectionMatrix;


GLuint gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;
GLuint gLdUniform, gLaUniform, gLsUniform;
GLuint gLightPositionUniform;
GLuint gKdUniform, gKaUniform, gKsUniform;
GLuint gMaterialShininessUniform;
GLuint gTextureSamplerOutputImageUniform;
GLuint gLKeyPressedUniform;
GLuint gdtUniform;

GLfloat gAngle = 0.0f;
GLfloat zTrans = 3.0f;
//bool gbAnimate;
bool gbLight;

GLfloat lightAmbient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat lightDiffuse[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightPosition[] = { 100.0f,100.0f,100.0f,1.0f };

GLfloat material_ambient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat material_diffuse[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat material_specular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat material_shininess = 50.0f;

float gWidth, gHeight;



GLuint output_image;
extern "C" JNIEXPORT void JNICALL
Java_com_astromedicomp_geometryShaderByNDK_BlueScreen_initialize(
        JNIEnv *env,
        jobject /* this */) {
   void initialize();
	
	initialize();
}

void initialize(void)
{
	//function prototypes
	void uninitialize(void);
	void resize(int, int);

	gComputeShaderObject = glCreateShader(GL_COMPUTE_SHADER);
	const GLchar* computeShaderSourceCode =
		"#version 320 es" \
		"\n"
		"layout (local_size_x = 32, local_size_y = 16) in;" \
		"layout (rgba32f, binding = 0) writeonly uniform highp image2D output_image;" \

		"void main(void)" \
		"{" \
		"imageStore(output_image, " \
		"ivec2(gl_GlobalInvocationID.xy)," \
		"vec4(vec2(gl_LocalInvocationID.xy) / vec2(gl_WorkGroupSize.xy), 0.0, 0.0));" \
		"}";
		
	glShaderSource(gComputeShaderObject, 1, (const GLchar * *)& computeShaderSourceCode, NULL);

	//compile shader
	glCompileShader(gComputeShaderObject);

	GLint iInfoLogLength = 0;
	GLint iShaderCompiledStatus = 0;
	char* szInfoLog = NULL;

	glGetShaderiv(gComputeShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(gComputeShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char*)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gComputeShaderObject, iInfoLogLength, &written, szInfoLog);
				/*fopen_s(&gpFile, "Log.txt", "a+");
				fprintf(gpFile, "Compute Shader Compilation Log : %s\n", szInfoLog);
				fclose(gpFile);*/
				printf("MPD : Compute Shader Compilation Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}

	//Shader Program

	gComputeProgramObject = glCreateProgram();

	glAttachShader(gComputeProgramObject, gComputeShaderObject);


	//glBindAttribLocation(flock_render_program, MALATI_ATTRIBUTE_VERTEX, "vPosition");
	//glBindAttribLocation(flock_render_program, MALATI_ATTRIBUTE_NORMAL, "vNormal");

	glLinkProgram(gComputeProgramObject);

	GLint iShaderProgramLinkStatus = 0;
	glGetProgramiv(gComputeProgramObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);
	if (iShaderProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(gComputeProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char*)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gComputeProgramObject, iInfoLogLength, &written, szInfoLog);
				/*fopen_s(&gpFile, "Log.txt", "a+");
				fprintf(gpFile, "Compute Shader  Program Link Log : %s\n", szInfoLog);
				fclose(gpFile);*/
				printf("MPD : Compute Shader  Program Link Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}

	//gdtUniform = glGetUniformLocation(gComputeProgramObject, "dt");
	//gIncUniform = glGetUniformLocation(flock_update_program, "inc");
	glGenTextures(1, &output_image);
	glBindTexture(GL_TEXTURE_2D, output_image);
	glTexStorage2D(GL_TEXTURE_2D, 8, GL_RGBA32F, 256, 256);

///////////////////////////////////////////////////////////////////////////////////
	//VERTEX SHADER
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	const GLchar* vertextShaderSourceCode =
		"#version 320 es" \
		"\n" \
		"in vec4 vPosition;" \
		"void main(void)" \
		"{" \
		
		/*"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vec4(vert.xyz, 1.0);" \*/
		"gl_Position = vPosition;" \
		"}";


	glShaderSource(gVertexShaderObject, 1, (const GLchar * *)& vertextShaderSourceCode, NULL);

	//compile shader
	glCompileShader(gVertexShaderObject);

	iInfoLogLength = 0;
	iShaderCompiledStatus = 0;
	szInfoLog = NULL;

	glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char*)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObject, iInfoLogLength, &written, szInfoLog);
				/*fopen_s(&gpFile, "Log.txt", "a+");
				fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", szInfoLog);
				fclose(gpFile);*/
				
				printf("MPD : Vertex Shader Compilation Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	//FRAGMENT SHADER

	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* fragmentShaderSourceCode =
		"#version 320 es" \
		"\n" \
		"precision highp float;"\
		"out vec4 FragColor;" \
		"uniform sampler2D output_image;" \
		"void main (void)" \
		"{" \
		"FragColor = texture(output_image, vec2(gl_FragCoord.xy) / vec2(textureSize(output_image, 0)));" \
		"}";

	

	glShaderSource(gFragmentShaderObject, 1, (const GLchar * *)& fragmentShaderSourceCode, NULL);

	glCompileShader(gFragmentShaderObject);

	glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char*)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLength, &written, szInfoLog);
				/*fopen_s(&gpFile, "Log.txt", "a+");
				fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", szInfoLog);
				fclose(gpFile);*/
				printf("MPD : Fragment Shader Compilation Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	/*fopen_s(&gpFile, "Log.txt", "a+");
	fprintf(gpFile, "Initialise 1\n");
	fclose(gpFile);*/
	//Shader Program

	gShaderProgramObject = glCreateProgram();

	glAttachShader(gShaderProgramObject, gVertexShaderObject);

	glAttachShader(gShaderProgramObject, gFragmentShaderObject);

	glBindAttribLocation(gShaderProgramObject, MALATI_ATTRIBUTE_VERTEX, "vPosition");
	//glBindAttribLocation(gShaderProgramObject, MALATI_ATTRIBUTE_NORMAL, "vNormal");


	//glBindAttribLocation(gShaderProgramObject, MALATI_ATTRIBUTE_BIRD_POSITION, "bird_position");
	//glBindAttribLocation(gShaderProgramObject, MALATI_ATTRIBUTE_BIRD_VELOCITY, "bird_velocity");

	glLinkProgram(gShaderProgramObject);

	iShaderProgramLinkStatus = 0;
	glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);
	if (iShaderProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char*)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObject, iInfoLogLength, &written, szInfoLog);
				/*fopen_s(&gpFile, "Log.txt", "a+");
				fprintf(gpFile, "Shader Program Link Log : %s\n", szInfoLog);
				fclose(gpFile);*/
				printf("MPD : Shader Program Link Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}

	/*gModelMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
	gViewMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
	gProjectionMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");*/

	/*gLKeyPressedUniform = glGetUniformLocation(flock_render_program, "u_LKeyPressed");

	gLdUniform = glGetUniformLocation(flock_render_program, "u_Ld");
	gLaUniform = glGetUniformLocation(flock_render_program, "u_La");
	gLsUniform = glGetUniformLocation(flock_render_program, "u_Ls");

	gKdUniform = glGetUniformLocation(flock_render_program, "u_Kd");
	gKsUniform = glGetUniformLocation(flock_render_program, "u_Ks");
	gKaUniform = glGetUniformLocation(flock_render_program, "u_Ka");

	gLightPositionUniform = glGetUniformLocation(flock_render_program, "u_light_position");
	gMaterialShininessUniform = glGetUniformLocation(flock_render_program, "u_material_shininess");
	*/
	//Sphere sphere;
	/*getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
	gNumVertices = getNumberOfSphereVertices();
	gNumElements = getNumberOfSphereElements();
	*/
	/*fopen_s(&gpFile, "Log.txt", "a+");
	fprintf(gpFile, "Initialise 2\n");
	fclose(gpFile);*/

	static const float verts[] =
	{
		-1.0f, -1.0f, 0.5f, 1.0f,
		1.0f, -1.0f, 0.5f, 1.0f,
		1.0f, 1.0f, 0.5f, 1.0f,
		-1.0f, 1.0f, 0.5f, 1.0f,
	};
	glGenVertexArrays(1, &render_vao);
	glBindVertexArray(render_vao);

	//position
	glGenBuffers(1, &render_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, render_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	glVertexAttribPointer(MALATI_ATTRIBUTE_VERTEX, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(MALATI_ATTRIBUTE_VERTEX);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/*fopen_s(&gpFile, "Log.txt", "a+");
	fprintf(gpFile, "Initialise 4\n");
	fclose(gpFile);*/

	//glShadeModel(GL_SMOOTH);

	glClearDepthf(1.0f);
	// enable depth testing
	glEnable(GL_DEPTH_TEST);
	// depth test to do
	glDepthFunc(GL_LEQUAL);

	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_CULL_FACE);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // blue

	gPerspectiveProjectionMatrix = mat4::identity();


	gbLight = false;
	// resize
	resize(WIN_WIDTH, WIN_HEIGHT);
	gWidth = WIN_WIDTH;
	gHeight = WIN_HEIGHT;
	//ToggleFullscreen();

	/*fopen_s(&gpFile, "Log.txt", "a+");
	fprintf(gpFile, "Ending Initialise\n");
	fclose(gpFile);*/
	printf("MPD : Ending Initialise\n");

	
}

//public native void resize(int width, int height);
void resize(int width, int height)
{
	//code
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	/*
	if (width <= height)
	{
	gOrthographicProjectionMatrix = ortho(-100.0f, 100.0f, (-100.0f *(height / width)), (100.0f * (height / width)), -100.0f, 100.0f);
	}
	else
	{
	gOrthographicProjectionMatrix = ortho((-100.0f *(width / height)), (100.0f * (width / height)), -100.0f, 100.0f, -100.0f, 100.0f);
	}*/
	gPerspectiveProjectionMatrix = perspective(45.0f, (GLfloat)width / (GLfloat)height,
		0.1f, 1000.0f); //perspective(3rd change)
}
/*Initialization Done
Now Its time to Resize*/

extern "C" JNIEXPORT void JNICALL
Java_com_astromedicomp_geometryShaderByNDK_BlueScreen_resize(
        JNIEnv *env,
        jobject /* this */,
		jint width,
		jint height) {
	//void resize(int , int);
    if(!width || !height)
	{
		width = WIN_WIDTH;
		height = WIN_HEIGHT;
	}
	resize((float)width, (float)height);
}




/*Display*/
extern "C" JNIEXPORT void JNICALL
Java_com_astromedicomp_geometryShaderByNDK_BlueScreen_draw(
        JNIEnv *env,
        jobject /* this */) {
			void display();
   display();
}

void display(void)
{
	//code
	
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glUseProgram(gComputeProgramObject);
	glBindImageTexture(0, output_image, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glDispatchCompute(8, 16, 1);

	glBindTexture(GL_TEXTURE_2D, output_image);

	glUseProgram(gShaderProgramObject);

	/*mat4 modelMatrix = mat4::identity();
	mat4 rotationMatrix = mat4::identity();

	mat4 viewMatrix = mat4::identity();
	modelMatrix = vmath::translate(0.0f, 0.0f, -160.0f);
	rotationMatrix = vmath::rotate(time * 1000.0f, vec3(0.0f, 1.0f, 0.0f));

	modelMatrix = modelMatrix * rotationMatrix;

	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);

	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);

	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);*/

glUniform1i(gTextureSamplerOutputImageUniform, 0);
	glBindVertexArray(render_vao);

	//glPointSize(2.0f);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glBindVertexArray(0);

	//SwapBuffers(ghdc);
}


void uninitialize(void)
{
	//UNINITIALIZATION CODE
/*	if (gbFullscreen == true)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
	}
*/
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

	/*wglMakeCurrent(NULL, NULL);
	wglDeleteContext(ghrc);
	ghrc = NULL;

	//Delete the device context
	ReleaseDC(ghwnd, ghdc);
	ghdc = NULL;

	if (gpFile)
	{
		fprintf(gpFile, "Log File Is Successfully Closed.\n");
		fclose(gpFile);
		gpFile = NULL;
	}*/
}
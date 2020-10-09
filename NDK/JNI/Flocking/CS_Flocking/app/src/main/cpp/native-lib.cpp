#include <jni.h>
#include <string>

#include "vmath.h"
#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>


//JNI information and error logs
#include <android/log.h>
#include <sys/time.h>
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

/*#define LOG_TAG "libNative"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)*/

#define LOG_TAG "MPD: native-lib"
#define LOGINFO(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
//#define LOGERROR(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

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

/*WORKGROUP_SIZE = 256,
        NUM_WORKGROUPS = 64,*/
enum
{
    WORKGROUP_SIZE = 32,
    NUM_WORKGROUPS = 64,
    FLOCK_SIZE = (NUM_WORKGROUPS * WORKGROUP_SIZE)
};

GLuint gVertexShaderObject;
GLuint gComputeShaderObject;
GLuint gFragmentShaderObject;
//GLuint gShaderProgramObject;
GLuint flock_update_program;
GLuint flock_render_program;

GLuint flock_buffer[2];
GLuint flock_render_vao[2];
GLuint geometry_buffer;

struct flock_member
{
    vec3 position;
    unsigned int : 32;
    vec3 velocity;
    unsigned int : 32;
};

GLuint frame_index;

GLuint gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;
GLuint gLdUniform, gLaUniform, gLsUniform;
GLuint gLightPositionUniform;
GLuint gKdUniform, gKaUniform, gKsUniform;
GLuint gMaterialShininessUniform;
GLuint gGoalUniform, gIncUniform;
GLuint gLKeyPressedUniform;

mat4 gPerspectiveProjectionMatrix;

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


extern "C" JNIEXPORT void JNICALL
Java_com_example_flocking_BlueScreen_initialize(
        JNIEnv *env,
        jobject /* this*/ )
{
    //printf("MALATI : BlueScreen Initialize : \n");
    void initialize();

    initialize();
}


void initialize(void) {
    //function prototypes
    void uninitialize(void);
    void resize(int, int);
    void startTimer();
    startTimer();

    gComputeShaderObject = glCreateShader(GL_COMPUTE_SHADER);
    const GLchar* computeShaderSourceCode =
            "#version 320 es" \
		    "\n"
            "layout (local_size_x =256) in;" \
            /*"precision highp float;"\*/
            "float closest_allowed_dist = 50.0;" \
		    "float rule1_weight = 0.18;" \
		    "float rule2_weight = 0.05;" \
		    "float rule3_weight = 0.17;" \
		    "float rule4_weight = 0.02;" \
		    "uniform mediump vec3 goal;" \
		    "float timestep = 0.4;" \
		/*"uniform highp float inc;" \*/
		    "struct flock_member" \
		    "{" \
		    "vec3 position;" \
		    "vec3 velocity;" \
		    "};" \

		    "layout (std430, binding = 0) readonly buffer members_in" \
		    "{" \
		    "flock_member member[];" \
		    "} input_data;" \

		    "layout (std430, binding = 1) buffer members_out" \
		    "{" \
		    "flock_member member[];" \
		    "} output_data;" \

		    "shared flock_member shared_member[gl_WorkGroupSize.x];" \

		    "vec3 rule1(vec3 my_position, vec3 my_velocity, vec3 their_position, vec3 their_velocity)" \
		    "{" \
		    "vec3 d = my_position - their_position;" \
		    "if(dot(d, d) < closest_allowed_dist)" \
		    "{" \
		    "return d;" \
		    "}" \
		    "return vec3(0.0);" \
		    "}" \

		    "vec3 rule2(vec3 my_position, vec3 my_velocity, vec3 their_position, vec3 their_velocity)" \
		    "{" \
		    "vec3 d = their_position - my_position;" \
		    "vec3 dv = their_velocity - my_velocity;" \
		    "return dv/ (dot(d, d) + 10.0);" \
		    "}" \

		    "void main(void)" \
		    "{" \
		    "int i, j;" \
		    "int global_id = int(gl_GlobalInvocationID.x);" \
		    "int local_id = int(gl_LocalInvocationID.x);" \

		    "flock_member me = input_data.member[global_id];" \
		    "flock_member new_me;" \
		    "vec3 accelleration = vec3(0.0);" \
		    "vec3 flock_center = vec3(0.0);" \
		    "vec3 gravity = vec3(0.0f, -9.8f, 0.0f);" \
		    "for(i = 0; i < int(gl_NumWorkGroups.x); i++)" \
		    "{" \
		    "flock_member them = input_data.member[i * int(gl_WorkGroupSize.x) + local_id]; "\
		    "shared_member[local_id] = them;" \
		    "memoryBarrierShared();" \
		    "barrier();" \
		    "for(j = 0; j < int(gl_WorkGroupSize.x); j++)" \
		    "{" \
		    "them = shared_member[j];" \
		    "flock_center += them.position;" \
		    "if(i * int(gl_WorkGroupSize.x) + j != global_id)" \
		    "{" \
		    "accelleration += rule1(me.position, me.velocity, them.position, them.velocity) * rule1_weight;" \

		    "accelleration += rule2(me.position, me.velocity, them.position, them.velocity) * rule2_weight;" \
		    "}" \
		    "}" \
		    "barrier();" \
		    "}" \

		    "flock_center /= float(gl_NumWorkGroups.x * gl_WorkGroupSize.x);" \
		    "new_me.position = me.position + me.velocity * timestep;" \
		    "accelleration += normalize(goal - me.position) * rule3_weight;" \
		    "accelleration += normalize(flock_center - me.position) * rule4_weight;" \


		    "new_me.velocity = me.velocity + accelleration * timestep;" \
		    "if(length(new_me.velocity) > 10.0)" \
		    "{" \
		    "new_me.velocity = normalize(new_me.velocity) * 10.0;" \
		/*"accelleration += gravity;" \*/
		    "}" \
		    "new_me.velocity = mix(me.velocity, new_me.velocity, 0.4);" \
		    "output_data.member[global_id] = new_me;" \
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
      //          printf("MPD : Compute Shader Compilation Log : %s\n", szInfoLog);
                LOGINFO("Compute Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                uninitialize();
                exit(0);
            }
        }
    }

    //Shader Program

    flock_update_program = glCreateProgram();

    glAttachShader(flock_update_program, gComputeShaderObject);


    //glBindAttribLocation(flock_render_program, MALATI_ATTRIBUTE_VERTEX, "vPosition");
    //glBindAttribLocation(flock_render_program, MALATI_ATTRIBUTE_NORMAL, "vNormal");

    glLinkProgram(flock_update_program);

    GLint iShaderProgramLinkStatus = 0;
    glGetProgramiv(flock_update_program, GL_LINK_STATUS, &iShaderProgramLinkStatus);
    if (iShaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(flock_update_program, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if (iInfoLogLength > 0)
        {
            szInfoLog = (char*)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(flock_update_program, iInfoLogLength, &written, szInfoLog);
                /*fopen_s(&gpFile, "Log.txt", "a+");
                fprintf(gpFile, "Compute Shader Update Program Link Log : %s\n", szInfoLog);
                fclose(gpFile);*/
                //printf("MPD : Compute Shader  Program Link Log : %s\n", szInfoLog);
                LOGINFO("Compute Shader  Program Link Log : %s\n", szInfoLog);
                free(szInfoLog);
                uninitialize();
                exit(0);
            }
        }
    }

    gGoalUniform = glGetUniformLocation(flock_update_program, "goal");
    gIncUniform = glGetUniformLocation(flock_update_program, "inc");
    //VERTEX SHADER
    gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    const GLchar* vertextShaderSourceCode =
            "#version 320 es" \
		    "\n" \
		    "in vec4 vPosition;" \
		    "in vec3 vNormal;" \
		    "in vec3 bird_position;" \
		    "in vec3 bird_velocity;" \

		    "out flat vec3 color;" \

		    "uniform mat4 u_model_matrix;" \
		    "uniform mat4 u_view_matrix;" \
		    "uniform mat4 u_projection_matrix;" \
		    "uniform vec4 u_light_position;"\

		    "out vec3 transformed_normals;" \
		    "out vec3 light_direction;" \
		    "out vec3 viewer_vector;" \

            "mat4 make_lookat(vec3 forward, vec3 up)" \
		    "{" \
		    "vec3 side = cross(forward, up);" \
		    "vec3 u_frame = cross(side, forward);" \
		    "return mat4(vec4(side, 0.0), vec4(u_frame, 0.0), vec4(forward, 0.0), vec4(0.0, 0.0, 0.0, 1.0));" \
		    "}" \

		    "vec3 choose_color(float f)" \
		    "{" \
		    "float R = sin(f * 6.2831853);" \
		    "float G = sin((f + 0.3333) * 6.2831853);" \
		    "float B = sin((f + 0.6666) * 6.2831853);" \

		    "return vec3(R, G, B) * 0.25 + vec3(0.75);" \
		    "}" \


            "void main(void)" \
		    "{" \

		    "vec4 eye_coordinates=u_view_matrix * u_model_matrix * vPosition;" \
		    "transformed_normals=mat3(u_view_matrix * u_model_matrix) * vNormal;" \
		    "light_direction = vec3(u_light_position) - eye_coordinates.xyz;" \
		    "viewer_vector = -eye_coordinates.xyz;" \


            "mat4 lookat = make_lookat(normalize(bird_velocity), vec3(0.0, 1.0f, 0.0));" \
		    "vec4 obj_coord = lookat * vec4(-vPosition.xyz, 1.0);" \
		    "gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * (obj_coord - vec4(bird_position , 0.0));" \

            "vec3 N = mat3(lookat) * vNormal;" \
		    "vec3 C = choose_color(fract(float(gl_InstanceID) / float(1237.0)));" \


            "color = mix(C * 0.2, C, smoothstep(0.0, 0.8, abs(N).z));" \

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
               /* fopen_s(&gpFile, "Log.txt", "a+");
                fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", szInfoLog);
                fclose(gpFile);*/
                LOGINFO("Vertex Shader Compilation Log : %s\n", szInfoLog);
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
		    "in flat vec3 color;" \

		    "in vec3 transformed_normals;" \
		    "in vec3 light_direction;" \
		    "in vec3 viewer_vector;" \
		    "out vec4 FragColor;" \

		    "void main(void)" \
		    "{" \
		    "vec3 phong_ads_color;" \

		    "phong_ads_color=color.rgb;" \

		    "FragColor = vec4(phong_ads_color, 1.0) ;" \
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
                LOGINFO("Fragment Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                uninitialize();
                exit(0);
            }
        }
    }

    //Shader Program

    flock_render_program = glCreateProgram();

    glAttachShader(flock_render_program, gVertexShaderObject);

    glAttachShader(flock_render_program, gFragmentShaderObject);

    glBindAttribLocation(flock_render_program, MALATI_ATTRIBUTE_VERTEX, "vPosition");
    glBindAttribLocation(flock_render_program, MALATI_ATTRIBUTE_NORMAL, "vNormal");


    glBindAttribLocation(flock_render_program, MALATI_ATTRIBUTE_BIRD_POSITION, "bird_position");
    glBindAttribLocation(flock_render_program, MALATI_ATTRIBUTE_BIRD_VELOCITY, "bird_velocity");

    glLinkProgram(flock_render_program);

    iShaderProgramLinkStatus = 0;
    glGetProgramiv(flock_render_program, GL_LINK_STATUS, &iShaderProgramLinkStatus);
    if (iShaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(flock_render_program, GL_INFO_LOG_LENGTH, &iInfoLogLength);
        if (iInfoLogLength > 0)
        {
            szInfoLog = (char*)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(flock_render_program, iInfoLogLength, &written, szInfoLog);
               /* fopen_s(&gpFile, "Log.txt", "a+");
                fprintf(gpFile, "Shader Render Program Link Log : %s\n", szInfoLog);
                fclose(gpFile);*/
                LOGINFO("Shader Program Link Log : %s\n", szInfoLog);
                free(szInfoLog);
                uninitialize();
                exit(0);
            }
        }
    }

    gModelMatrixUniform = glGetUniformLocation(flock_render_program, "u_model_matrix");
    gViewMatrixUniform = glGetUniformLocation(flock_render_program, "u_view_matrix");
    gProjectionMatrixUniform = glGetUniformLocation(flock_render_program, "u_projection_matrix");

    gLKeyPressedUniform = glGetUniformLocation(flock_render_program, "u_LKeyPressed");

    gLdUniform = glGetUniformLocation(flock_render_program, "u_Ld");
    gLaUniform = glGetUniformLocation(flock_render_program, "u_La");
    gLsUniform = glGetUniformLocation(flock_render_program, "u_Ls");

    gKdUniform = glGetUniformLocation(flock_render_program, "u_Kd");
    gKsUniform = glGetUniformLocation(flock_render_program, "u_Ks");
    gKaUniform = glGetUniformLocation(flock_render_program, "u_Ka");

    gLightPositionUniform = glGetUniformLocation(flock_render_program, "u_light_position");
    gMaterialShininessUniform = glGetUniformLocation(flock_render_program, "u_material_shininess");

    //Sphere sphere;
    /*getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
    gNumVertices = getNumberOfSphereVertices();
    gNumElements = getNumberOfSphereElements();
    */


    /*glGenVertexArrays(1, &gVao_sphere);
    glBindVertexArray(gVao_sphere);

    //position
    glGenBuffers(1, &gVbo_sphere_position);
    glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(MALATI_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(MALATI_ATTRIBUTE_VERTEX);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //normal
    glGenBuffers(1, &gVbo_sphere_normal);
    glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_normal);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);

    glVertexAttribPointer(MALATI_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(MALATI_ATTRIBUTE_NORMAL);


    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // element vbo
    glGenBuffers(1, &gVbo_sphere_element);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    glBindVertexArray(0);
    */

    // This is position and normal data for a paper airplane
    static const vmath::vec3 geometry[] =
            {
                    // Positions
                    vmath::vec3(-5.0f, 1.0f, 0.0f),
                    vmath::vec3(-1.0f, 1.5f, 0.0f),
                    vmath::vec3(-1.0f, 1.5f, 7.0f),
                    vmath::vec3(0.0f, 0.0f, 0.0f),
                    vmath::vec3(0.0f, 0.0f, 10.0f),
                    vmath::vec3(1.0f, 1.5f, 0.0f),
                    vmath::vec3(1.0f, 1.5f, 7.0f),
                    vmath::vec3(5.0f, 1.0f, 0.0f),

                    // Normals
                    vmath::vec3(0.0f),
                    vmath::vec3(0.0f),
                    vmath::vec3(0.107f, -0.859f, 0.00f),
                    vmath::vec3(0.832f, 0.554f, 0.00f),
                    vmath::vec3(-0.59f, -0.395f, 0.00f),
                    vmath::vec3(-0.832f, 0.554f, 0.00f),
                    vmath::vec3(0.295f, -0.196f, 0.00f),
                    vmath::vec3(0.124f, 0.992f, 0.00f),
            };


    glGenBuffers(2, flock_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, flock_buffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, FLOCK_SIZE * sizeof(flock_member), NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, flock_buffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, FLOCK_SIZE * sizeof(flock_member), NULL, GL_DYNAMIC_COPY);

    int i;

    glGenBuffers(1, &geometry_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometry_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW);
    glGenVertexArrays(2, flock_render_vao);

    for (i = 0; i < 2; i++)
    {
        glBindVertexArray(flock_render_vao[i]);
        glBindBuffer(GL_ARRAY_BUFFER, geometry_buffer);
        glVertexAttribPointer(MALATI_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glVertexAttribPointer(MALATI_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, (void*)(8 * sizeof(vmath::vec3)));
        glBindBuffer(GL_ARRAY_BUFFER, flock_buffer[i]);
        glVertexAttribPointer(MALATI_ATTRIBUTE_BIRD_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(flock_member), NULL);
        glVertexAttribPointer(MALATI_ATTRIBUTE_BIRD_VELOCITY, 3, GL_FLOAT, GL_FALSE, sizeof(flock_member), (void*)sizeof(vmath::vec4));

        glVertexAttribDivisor(MALATI_ATTRIBUTE_BIRD_POSITION, 1);
        glVertexAttribDivisor(MALATI_ATTRIBUTE_BIRD_VELOCITY, 1);

        glEnableVertexAttribArray(MALATI_ATTRIBUTE_VERTEX);
        glEnableVertexAttribArray(MALATI_ATTRIBUTE_NORMAL);
        glEnableVertexAttribArray(MALATI_ATTRIBUTE_BIRD_POSITION);
        glEnableVertexAttribArray(MALATI_ATTRIBUTE_BIRD_VELOCITY);

    }

    glBindBuffer(GL_ARRAY_BUFFER, flock_buffer[0]);
    flock_member* ptr = reinterpret_cast<flock_member*>
    (glMapBufferRange(GL_ARRAY_BUFFER, 0, FLOCK_SIZE * sizeof(flock_member),
                      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

    for (i = 0; i < FLOCK_SIZE; i++)
    {
        ptr[i].position = (vmath::vec3::random() - vmath::vec3(0.5f) * 300.0f);
        ptr[i].velocity = (vmath::vec3::random() - vmath::vec3(0.5f));

    }

    glUnmapBuffer(GL_ARRAY_BUFFER);

  //  glShadeModel(GL_SMOOTH);

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
    gPerspectiveProjectionMatrix = perspective(60.0f, (GLfloat)width / (GLfloat)height,
                                               0.1f, 3000.0f); //perspective(3rd change)

    gWidth = width;
    gHeight = height;

}

extern "C" JNIEXPORT void JNICALL
Java_com_example_flocking_BlueScreen_resize(
        JNIEnv *env,
        jobject /* this */,
        jint width,
        jint height)
{
    if(!width || !height)
    {
        width = WIN_WIDTH;
        height = WIN_HEIGHT;
    }
    resize((float)width, (float)height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_flocking_BlueScreen_draw(
        JNIEnv *env,
        jobject /* this */)
{
    void display();
    display();
}


void display(void) {
    //code

    double GetTime(void);
    float t = (float)GetTime();


    /*static float t = (float)119335112.0f;
    t += 50.0f;*/
    static const float black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    static const float one = 1.0f;

    glUseProgram(flock_update_program);

    static float xTrans = 0.0f;
    static float yTrans = 50.0f;


    /*vec3 goal = vec3(sinf(t * 0.34f),
                     sinf(t * 0.29f),
                     sinf(t * 0.12f) ) ;*/
    vec3 goal = vec3(sinf(t * 0.34f),
                     cosf(t * 0.29f),
                     sinf(t * 0.12f) * cosf(t * 0.5f)) * vec3(35.0f, 25.0f, 60.0f);
    /*vec3 goal = vec3(sinf(t * (float)(4) * 7.5f * 20.0f) * 50.0f,
            cosf(t * (float)(7) * 3.9f * 20.0f) * 50.0f,
            sinf(t * (float)(3) * 5.3f * 20.0f) * cosf(t * (float)(5) * 9.1f) * 100.0f);*/

   // goal = goal * ;
    glUniform3fv(gGoalUniform, 1, goal);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, flock_buffer[frame_index]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, flock_buffer[frame_index ^ 1]);

    glDispatchCompute(NUM_WORKGROUPS, 1, 1);

    glViewport(0, 0, gWidth, gHeight);

    glClearBufferfv(GL_COLOR, 0, black);
    glClearBufferfv(GL_DEPTH, 0, &one);

    glUseProgram(flock_render_program);

    ////////////////////////////////////
    glUniform1i(gLKeyPressedUniform, 1);
    glUniform3fv(gLdUniform, 1, lightDiffuse);//white
    glUniform3fv(gLaUniform, 1, lightAmbient);
    glUniform3fv(gLsUniform, 1, lightSpecular);
    glUniform4fv(gLightPositionUniform, 1, lightPosition);

    glUniform3fv(gKdUniform, 1, material_diffuse);
    glUniform3fv(gKaUniform, 1, material_ambient);
    glUniform3fv(gKsUniform, 1, material_specular);
    glUniform1f(gMaterialShininessUniform, material_shininess);



    ///////////////////////////////////
    mat4 modelMatrix = mat4::identity();
    mat4 viewMatrix = mat4::identity();
    modelMatrix = vmath::translate(100.0f, 0.0f, 0.0f);
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);

    viewMatrix = vmath::lookat(vmath::vec3(0.0f, 0.0f, -400.0f),
                               vmath::vec3(0.0f, 0.0f, 0.0f),
                               vmath::vec3(0.0f, 1.0f, 0.0f));

    vmath::mat4 proj_matrix = vmath::perspective(60.0f, (float)gWidth / (float)gHeight, 0.1f, 3000.0f);

    glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);

    glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, proj_matrix);

    glBindVertexArray(flock_render_vao[frame_index]);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 8, FLOCK_SIZE);

    modelMatrix = mat4::identity();
    modelMatrix = vmath::translate(-500.0f, 0.0f, 0.0f);
    glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 8, FLOCK_SIZE);

    frame_index ^= 1;

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
  /*  if (gVao)
    {
        glDeleteVertexArrays(1, &gVao);
        gVao = 0;
    }
    if (gVbo)
    {
        glDeleteBuffers(1, &gVbo);
        gVbo = 0;
    }*/

    glDetachShader(flock_render_program, gVertexShaderObject);
    glDetachShader(flock_render_program, gFragmentShaderObject);

    glDeleteShader(gVertexShaderObject);
    gVertexShaderObject = 0;
    glDeleteShader(gFragmentShaderObject);
    gFragmentShaderObject = 0;

    glDeleteProgram(flock_render_program);
    flock_render_program = 0;

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
struct timeval start;
struct timeval stop;
void startTimer()
{

    gettimeofday(&start, NULL);
}
uint64_t GetTimerValue(void)
{
    /* if(monotonic)
     {
         struct timespec ts;
         clock_gettime(CLOCK_MONOTONIC, &ts);
         return (int)ts.tv_sec * (int) 1000000000 + (int)ts.tv_nsec;
     } else{*/

    gettimeofday(&stop, NULL);

    //return(uint64_t)tv.tv_sec * 1000000ULL + (uint64_t) tv.tv_usec;

    return((stop.tv_sec - start.tv_sec)*1000000 + stop.tv_usec - start.tv_usec);
    //}
}

/*unsigned long long getTimeInReady() const
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long long)_timeInReady.tv_sec * 1000000ULL +
           (unsigned long long)_timeInReady.tv_usec;
}*/
float frequency = 1000000;
int GetTimerFrequency(void)
{
    return frequency;
}

double GetTime(void)
{
    //LOGINFO("t :%d\n", GetTimerValue());
    return (double)(GetTimerValue());// / GetTimerFrequency());
}

//_glfw.timer.offset = _glfwPlatformGetTimerValue()
//        (uint64_t) (time * _glfwPlatformGetTimerFrequency());


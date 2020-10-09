#include <jni.h>
#include <string>
#include <pthread.h>

#include <GLES3/gl32.h>
#include "vmath.h"
#include "Model.h"

#include <android/log.h>

#include <android/asset_manager_jni.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define LOG_TAG "MPD: Teapot"
#define LOGINFO(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGERROR(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

/*enum {
    MPD_ATTRIBUTE_VERTEX = 0,
    MPD_ATTRIBUTE_COLOR,
    MPD_ATTRIBUTE_NORMAL,
    MPD_ATTRIBUTE_TEXTURE0,
};
*/
GLuint gShaderProgramObject;
GLint gMUniform,gVUniform,gPUniform, gTexture_sampler_uniform;

vmath::mat4 gPerspectiveProjectionMatrix;
float gWidth = 0.0f, gHeight = 0.0f;
float gAngle;

Model myModel;

/*extern "C" JNIEXPORT jstring JNICALL
Java_com_example_teapot_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject ) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}*/

extern "C" JNIEXPORT void JNICALL
        Java_com_example_teapot_GLESNativeLib_init(JNIEnv* env,
                                                   jclass clazz)
{
    void initialize();

    initialize();
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_teapot_GLESNativeLib_resize(JNIEnv* env,
                                           jclass clazz,
                                             jint width,
                                             jint height)
{
    void resize(int , int);
    if(!width || !height)
    {
        width = WIN_WIDTH;
        height = WIN_HEIGHT;
    }

    gWidth = width;
    gHeight = height;
    gAngle = 0.0f;
    resize((float)width, (float)height);
}



extern "C" JNIEXPORT void JNICALL
Java_com_example_teapot_GLESNativeLib_display(
        JNIEnv* env,
        jclass clazz) {
    void display();
    display();

}

pthread_mutex_t  threadMutex;
std::string apkInternalPath;
AAssetManager* apkAssetManager;
extern "C" JNIEXPORT void JNICALL
Java_com_example_teapot_GLESNativeLib_readAssetsNative(
        JNIEnv* env,
        jclass clazz,
        jobject assetManager,
        jstring pathToInternalDir) {

    //get native instance of asset Manager
   apkAssetManager = AAssetManager_fromJava(env, assetManager);

   //Save app internal data storage path -- we will extract assets and save here
   const char* cPathToInternalDir;
   cPathToInternalDir = env->GetStringUTFChars(pathToInternalDir, NULL);
   apkInternalPath = std::string(cPathToInternalDir);
   env->ReleaseStringUTFChars(pathToInternalDir, cPathToInternalDir);


  pthread_mutex_init(&threadMutex, NULL);


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
        glBindAttribLocation(program, 0, "vPosition");
        glBindAttribLocation(program, 3, "vTexCoord");

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

/**
 * Strip out the path and return just the filename
 */
 std::string GetFileName(std::string fileName)
{
     std::string::size_type  slashIndex = fileName.find_last_of("/");

     std::string onlyName;
     if(slashIndex == std::string::npos)
     {
         onlyName = fileName.substr(0, std::string::npos);
     } else{
         onlyName = fileName.substr(slashIndex + 1, std::string::npos);
     }

     return onlyName;
}


bool ExtractAssetReturnFilename(std::string assetName, std::string& fileName, bool checkIfFileIsAvailable = false)
{
    // construct the filename in internal storage by concatenating with path to internal storage
    fileName = apkInternalPath + "/" + GetFileName(assetName);

    //check if the file was previously extracted and is available in app's internal dir
    FILE* file = fopen(fileName.c_str(), "rb");
    if(file && checkIfFileIsAvailable)
    {
        LOGINFO("Found extracted file in assets: %s", fileName.c_str());
        fclose(file);
        pthread_mutex_unlock( &threadMutex);
        return true;
    }

    // let us look for the file in assets
    bool result = false;

    pthread_mutex_lock( &threadMutex);
    // Open file
    AAsset* asset = AAssetManager_open(apkAssetManager, assetName.c_str(), AASSET_MODE_STREAMING);

    char buf[BUFSIZ];
    int nb_read = 0;
    if(asset != NULL)
    {
        FILE* out = fopen(fileName.c_str(), "w");
        while((nb_read = AAsset_read(asset, buf, BUFSIZ)) > 0)
        {
            fwrite(buf, nb_read, 1, out);
        }
        fclose(out);
        AAsset_close(asset);
        result = true;

        LOGINFO("Asset extracted: %s", fileName.c_str());
    } else{
        LOGINFO("Asset not found: %s", assetName.c_str());
    }
    pthread_mutex_unlock( &threadMutex);
    return result;
}

void initialize()
{

    //android way
    //int loadTexture(int texWidth, int texHeight, int* texPixels);

    const GLchar *vertexShaderSource =
            "#version 320 es"\
            "\n"\
            "in vec4 vPosition;"\
           /* "in vec4 vColor;"\*/
            "in vec2 vTexCoord;" \

            "uniform mat4 u_m_matrix;"\
            "uniform mat4 u_v_matrix;"\
            "uniform mat4 u_p_matrix;"\

            /*"out vec4 out_color;"\*/
            "out vec2 out_texture0_coord;" \

            "void main(void)"\
            "{"\
            "gl_Position = u_p_matrix * u_v_matrix * u_m_matrix * vPosition;"\
            /*"out_color = vColor;"\*/
            "out_texture0_coord = vTexCoord;" \
            "}";



    const GLchar *fragmentShaderSource =
            "#version 320 es"\
            "\n"\
            "precision highp float;"\
            /*"in vec4 out_color;"\*/
            "in vec2 out_texture0_coord;" \

            "uniform sampler2D u_texture0_sampler;" \

            "out vec4 FragColor;"\
            "void main(void){"\
            "vec4 tex = texture(u_texture0_sampler, out_texture0_coord);" \

            "FragColor = vec4(tex.r, tex.g, tex.b, tex.a);"\
            "}";
    gShaderProgramObject = createProgramObject(vertexShaderSource,fragmentShaderSource);

    gMUniform = glGetUniformLocation(gShaderProgramObject,"u_m_matrix");
    gVUniform = glGetUniformLocation(gShaderProgramObject,"u_v_matrix");
    gPUniform = glGetUniformLocation(gShaderProgramObject,"u_p_matrix");
    gTexture_sampler_uniform = glGetUniformLocation(gShaderProgramObject,"u_texture0_sampler");

    std::string myOBjFileName, mtlFileName, texFileName;
    bool isFilePresent = ExtractAssetReturnFilename("amenemhat/amenemhat.obj", myOBjFileName) &&
            ExtractAssetReturnFilename("amenemhat/amenemhat.mtl", mtlFileName) &&
            ExtractAssetReturnFilename("amenemhat/amenemhat.jpg", texFileName);;


    if( !isFilePresent ) {
        LOGINFO("Model %s does not exist!", myOBjFileName.c_str());
        return;
    }
    LOGINFO("LOG Asset Extracted : %s\n", myOBjFileName.c_str());

    std::string path = myModel.getPath(myOBjFileName);
    LOGINFO("PathName : %s\n", path.c_str());
    myModel.LoadMyModel(path);


    glEnable(GL_TEXTURE_2D);
    glClearDepthf(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);

    glClearColor(0.1f,0.2f,0.3f,0.0f);

    gPerspectiveProjectionMatrix = vmath::mat4::identity();
}

void resize(int width, int height)
{
    glViewport(0,0,(GLsizei)width,(GLsizei)height);

    if(height==0)
        height=1;

    gPerspectiveProjectionMatrix = vmath::perspective(45.0f,(GLfloat)width / (GLfloat)height,0.01f,1000.0f);

    gWidth = width;
    gHeight = height;
}


void display()
{
    void update();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gShaderProgramObject);

    vmath::mat4 modelMatrix = vmath::mat4::identity();
    vmath::mat4 rotationMatrix = vmath::mat4::identity();
    vmath::mat4 scaleMatrix = vmath::mat4::identity();
    vmath::mat4 viewMatrix = vmath::mat4::identity();

    vmath::mat4 translationMatrix = vmath::mat4(1.0);
    translationMatrix = vmath::translate(vmath::vec3(0.0f, 0.0f, -10.0));
    modelMatrix *= translationMatrix;

    rotationMatrix = vmath::rotate(gAngle, vmath::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix *= rotationMatrix;

    glUniformMatrix4fv(gMUniform,1,GL_FALSE,modelMatrix);
    glUniformMatrix4fv(gVUniform,1,GL_FALSE,viewMatrix);
    glUniformMatrix4fv(gPUniform,1,GL_FALSE,gPerspectiveProjectionMatrix);

   // glActiveTexture(GL_TEXTURE0);
    //glBindTexture()
    glUniform1i(gTexture_sampler_uniform, 0);
    myModel.Draw(gShaderProgramObject);

    glUseProgram(0);

    update();
}


void update()
{
    gAngle +=1.0f;
    if(gAngle >= 360.0f)
        gAngle =0.0f;
}
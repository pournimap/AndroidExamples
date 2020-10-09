package com.example.framebuffer;

import android.content.Context;
import android.opengl.GLES20;
import android.view.MotionEvent;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.GestureDetector.OnDoubleTapListener;

import android.opengl.GLSurfaceView; // The view which gives us OPenGL Surface
import android.opengl.GLES32; //OpenGL ES 3.2 Version
import javax.microedition.khronos.opengles.GL10; //
import javax.microedition.khronos.egl.EGLConfig; // Embedded System supports EGL packages

import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.nio.ByteOrder;
import java.nio.ShortBuffer;

import android.opengl.Matrix;

public class GLESView extends GLSurfaceView implements  OnGestureListener, OnDoubleTapListener, GLSurfaceView.Renderer
{
    private final Context context;
    private GestureDetector gestureDetector;

    private int vertexShaderObject;
    private int fragmentShaderObject1;
    private int fragmentShaderObject2;
    private int shaderProgramObject1;
    private int shaderProgramObject2;

    private int numElements;
    private int numVertices;

    private int[] vao_sphere = new int[1];
    private int[] vbo_sphere_normal = new int[1];
    private int[] vbo_sphere_position = new int[1];
    private int[] vbo_sphere_element = new int[1];

    private int[]vao_cube = new int[1];
    private int[]vbo_position_cube = new int[1];
    private int[]vbo_normal_cube = new int[1];
    private int[]vbo_texture = new int[1];

    private float light_ambient[] = {0.0f,0.0f,0.0f,1.0f};
    private float light_diffuse[] = {1.0f,1.0f,1.0f,1.0f};
    private float light_specular[] = {1.0f,1.0f,1.0f,1.0f};
    private float light_position[] = { 100.0f,100.0f,100.0f,1.0f };

    private float material_ambient[] = {0.0f,0.0f,0.0f,1.0f};
    private float material_diffuse[] = {1.0f,1.0f,1.0f,1.0f};
    private float material_specular[] = {1.0f,1.0f,1.0f,1.0f};
    private float material_shininess = 50.0f;

    private int modelMatrixUniform, viewMatrixUniform, projectionMatrixUniform;
    private int laUniform, ldUniform, lsUniform, lightPositionUniform;
    private int kaUniform, kdUniform, ksUniform, materialShininessUniform;
    private int samplerUniform;
    private int doubleTapUniform;

    private int modelMatrixUniform1, viewMatrixUniform1, projectionMatrixUniform1;
    private int laUniform1, ldUniform1, lsUniform1, lightPositionUniform1;
    private int kaUniform1, kdUniform1, ksUniform1, materialShininessUniform1;
    private int doubleTapUniform1;

    private float perspectiveProjectionMatrix[] = new float[16];
    private float gAngleCube = 0.0f;
    private int doubleTap;

    private int fbo_new[] = new int[1];
    private int[] texture_attach0 = new int[1];
    private int[] depth_attach0 = new int[1];

    float gWidth;
    float gHeight;
    float cube_angle = 0.0f;

    GLESView(Context drawingContext)
    {
        super(drawingContext);
        context =drawingContext;

        setEGLContextClientVersion(3);

        setRenderer(this);

        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        gestureDetector = new GestureDetector(context, this, null, false);

        gestureDetector.setOnDoubleTapListener(this);
    }

    //Handling onTouchEvent is the Most Impostant
    //Because It Triggers All Gesture And Tap Events
    @Override
    public boolean onTouchEvent(MotionEvent event)
    {
        //code
        int eventaction = event.getAction();
        if(!gestureDetector.onTouchEvent(event))
            super.onTouchEvent(event);
        return(true);
    }

    @Override
    public boolean onDown(MotionEvent e)
    {
        // Do Not Write Any code Here Because Already Written 'onSingleTapConfirmed'
        return(true);
    }

    // abstract method from OnGestureListener so must be implemented
    @Override
    public void onShowPress(MotionEvent e)
    {
    }

    // abstract method from OnGestureListener so must be implemented
    @Override
    public boolean onSingleTapUp(MotionEvent e)
    {
        System.out.println("MPD: onSingleTapUp = ");
        return(true);
    }

    // abstract method from OnGestureListener so must be implemented
    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY)
    {
        uninitialize();
        System.exit(0);
        return (true);
    }

    // abstract method from OnGestureListener so must be implemented
    @Override
    public void onLongPress(MotionEvent e)
    {
    }

    // abstract method from OnGestureListener so must be implemented
    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY)
    {
        return(true);
    }

    // abstract method from OnDoubleTapListener so must be implemented
    @Override
    public boolean onSingleTapConfirmed(MotionEvent e)
    {

        return(true);
    }

    // abstract method from OnDoubleTapListener so must be implemented
    @Override
    public boolean onDoubleTap(MotionEvent e)
    {
        System.out.println("MPD: onDoubleTap = ");
        doubleTap++;
        if(doubleTap > 1)
            doubleTap = 0;
        return (true);
    }

    // abstract method from OnDoubleTapListener so must be implemented
    @Override
    public boolean onDoubleTapEvent(MotionEvent e)
    {
        // Do Not Write Any code Here Because Already Written 'onDoubleTap'
        return(true);
    }

    //overriden method of GLSurfaceView.Renderer (init Code)
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config)
    {
        String glesVersion = gl.glGetString(GL10.GL_VERSION);
        System.out.println("MPD: OpenGL-ES Version = "+glesVersion);

        String glslVersion = gl.glGetString(GLES32.GL_SHADING_LANGUAGE_VERSION);
        System.out.println("MPD: GLSL Version = " + glslVersion);

        initialize(gl);
    }

    //overriden method GLSurfaceView.Renderer(change Size)
    @Override
    public void onSurfaceChanged(GL10 unused, int width, int height)
    {
        resize(width, height);
    }

    // overriden method of GLSurfaceView.Renderer ( Rendering Code )
    @Override
    public void onDrawFrame(GL10 unused)
    {
        display();
        update();
    }

    private void initialize(GL10 gl)
    {
        vertexShaderObject = GLES32.glCreateShader(GLES32.GL_VERTEX_SHADER);

        final String vertexShaderSourceCode = String.format
                (
                        "#version 320 es"+
                                "\n"+
                                "in vec4 vPosition;"+
                                "in vec3 vNormal;"+
                                "in vec2 vTexcoord;"+
                                "uniform mat4 u_model_matrix;"+
                                "uniform mat4 u_view_matrix;"+
                                "uniform mat4 u_projection_matrix;"+
                                "uniform mediump int u_double_tap;"+
                                "uniform vec4 u_light_position;"+
                                "out vec3 transformed_normals;"+
                                "out vec3 light_direction;"+
                                "out vec3 viewer_vector;"+
                                "out vec2 outTexcoord;"+
                                "void main(void)"+
                                "{"+
                                "if (u_double_tap == 1)"+
                                "{"+
                                "vec4 eye_coordinates=u_view_matrix * u_model_matrix * vPosition;"+
                                "transformed_normals=mat3(u_view_matrix * u_model_matrix) * vNormal;"+
                                "light_direction = vec3(u_light_position) - eye_coordinates.xyz;"+
                                "viewer_vector = -eye_coordinates.xyz;"+
                                "outTexcoord = vTexcoord;" +
                                "}"+
                                "gl_Position=u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;"+
                                "}"
                );

        GLES32.glShaderSource(vertexShaderObject, vertexShaderSourceCode);

        //compile shader & check for errors
        GLES32.glCompileShader(vertexShaderObject);
        int [] iShaderCompiledStatus = new int[1];
        int[] iInfoLogLength = new int[1];
        String szInfoLog = null;
        GLES32.glGetShaderiv(vertexShaderObject, GLES32.GL_COMPILE_STATUS,
                iShaderCompiledStatus,0);

        if(iShaderCompiledStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(vertexShaderObject,
                    GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength,0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(vertexShaderObject);
                System.out.println("MPD: Vertex Shader Compilation Log = " +szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        //fragmentShaderObject
        fragmentShaderObject1 = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);

        final String fragmentShaderSourceCode = String.format
                (
                        "#version 320 es"+
                                "\n"+
                                "precision highp float;"+
                                "in vec3 transformed_normals;"+
                                "in vec3 light_direction;"+
                                "in vec3 viewer_vector;"+
                                "in vec2 outTexcoord;"+
                                "out vec4 FragColor;"+
                                "uniform vec3 u_La;"+
                                "uniform vec3 u_Ld;"+
                                "uniform vec3 u_Ls;"+
                                "uniform vec3 u_Ka;"+
                                "uniform vec3 u_Kd;"+
                                "uniform vec3 u_Ks;"+
                                "uniform float u_material_shininess;"+
                                "uniform int u_double_tap;"+

                                "void main(void)"+
                                "{"+
                                "vec3 phong_ads_color;"+
                                "if(u_double_tap==1)"+
                                "{"+
                                "vec3 normalized_transformed_normals=normalize(transformed_normals);"+
                                "vec3 normalized_light_direction=normalize(light_direction);"+
                                "vec3 normalized_viewer_vector=normalize(viewer_vector);"+
                                "vec3 ambient = u_La * u_Ka;"+
                                "float tn_dot_ld = max(dot(normalized_transformed_normals, normalized_light_direction),0.0);"+
                                "vec3 diffuse = u_Ld * u_Kd * tn_dot_ld;"+
                                "vec3 reflection_vector = reflect(-normalized_light_direction, normalized_transformed_normals);"+
                                "vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, normalized_viewer_vector), 0.0), u_material_shininess);"+
                                "phong_ads_color=ambient + diffuse + specular;"+
                                "}"+
                                "else"+
                                "{"+
                                "phong_ads_color = vec3(1.0, 1.0, 1.0);"+
                                "}"+
                                "FragColor = vec4(phong_ads_color, 1.0) * vec4(1.0, 0.0, 0.0, 0.0);"+
                                "}"

                );

        GLES32.glShaderSource(fragmentShaderObject1, fragmentShaderSourceCode);

        //compile shader & check for errors
        GLES32.glCompileShader(fragmentShaderObject1);
        iShaderCompiledStatus[0] = 0;
        iInfoLogLength[0] = 0;
        szInfoLog = null;
        GLES32.glGetShaderiv(fragmentShaderObject1, GLES32.GL_COMPILE_STATUS,
                iShaderCompiledStatus,0);

        if(iShaderCompiledStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(fragmentShaderObject1,
                    GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength,0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(fragmentShaderObject1);
                System.out.println("MPD: Fragment Shader Compilation Log = " +szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        shaderProgramObject1 = GLES32.glCreateProgram();

        GLES32.glAttachShader(shaderProgramObject1, vertexShaderObject);
        GLES32.glAttachShader(shaderProgramObject1, fragmentShaderObject1);

        GLES32.glBindAttribLocation(shaderProgramObject1, GLESMacros.AMC_ATTRIBUTE_POSITION, "vPosition");
        GLES32.glBindAttribLocation(shaderProgramObject1, GLESMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");
        GLES32.glBindAttribLocation(shaderProgramObject1, GLESMacros.AMC_ATTRIBUTE_TEXCOORD0, "vTexcoord");
        GLES32.glLinkProgram(shaderProgramObject1);
        int[] iShaderProgramLinkStatus = new int[1];
        iInfoLogLength[0] = 0;
        szInfoLog = null;
        GLES32.glGetProgramiv(shaderProgramObject1, GLES32.GL_LINK_STATUS,
                iShaderProgramLinkStatus, 0);

        if(iShaderProgramLinkStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetProgramiv(shaderProgramObject1,
                    GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetProgramInfoLog(shaderProgramObject1);
                System.out.println("MPD: Shader Program Link Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        modelMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject1, "u_model_matrix");
        viewMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject1, "u_view_matrix");

        projectionMatrixUniform = GLES32.glGetUniformLocation(shaderProgramObject1, "u_projection_matrix");

        doubleTapUniform = GLES32.glGetUniformLocation(shaderProgramObject1, "u_double_tap");
        laUniform = GLES32.glGetUniformLocation(shaderProgramObject1, "u_La");
        ldUniform = GLES32.glGetUniformLocation(shaderProgramObject1, "u_Ld");
        lsUniform = GLES32.glGetUniformLocation(shaderProgramObject1, "u_Ls");
        kaUniform = GLES32.glGetUniformLocation(shaderProgramObject1, "u_Ka");
        kdUniform = GLES32.glGetUniformLocation(shaderProgramObject1, "u_Kd");
        ksUniform = GLES32.glGetUniformLocation(shaderProgramObject1, "u_Ks");
        lightPositionUniform = GLES32.glGetUniformLocation(shaderProgramObject1, "u_light_position");

        materialShininessUniform = GLES32.glGetUniformLocation(shaderProgramObject1, "u_material_shininess");


        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////


        //fragmentShaderObject
        fragmentShaderObject2 = GLES32.glCreateShader(GLES32.GL_FRAGMENT_SHADER);

        final String fragmentShaderSourceCode1 = String.format
                (
                        "#version 320 es"+
                                "\n"+
                                "precision highp float;"+
                                "in vec3 transformed_normals;"+
                                "in vec3 light_direction;"+
                                "in vec3 viewer_vector;"+
                                "in vec2 outTexcoord;"+
                                "out vec4 FragColor;"+
                                "uniform vec3 u_La;"+
                                "uniform vec3 u_Ld;"+
                                "uniform vec3 u_Ls;"+
                                "uniform vec3 u_Ka;"+
                                "uniform vec3 u_Kd;"+
                                "uniform vec3 u_Ks;"+
                                "uniform float u_material_shininess;"+
                                "uniform int u_double_tap;"+
                                "uniform sampler2D texSampler;" +
                                "void main(void)"+
                                "{"+
                                "vec3 phong_ads_color;"+
                                "if(u_double_tap==1)"+
                                "{"+
                                "vec3 normalized_transformed_normals=normalize(transformed_normals);"+
                                "vec3 normalized_light_direction=normalize(light_direction);"+
                                "vec3 normalized_viewer_vector=normalize(viewer_vector);"+
                                "vec3 ambient = u_La * u_Ka;"+
                                "float tn_dot_ld = max(dot(normalized_transformed_normals, normalized_light_direction),0.0);"+
                                "vec3 diffuse = u_Ld * u_Kd * tn_dot_ld;"+
                                "vec3 reflection_vector = reflect(-normalized_light_direction, normalized_transformed_normals);"+
                                "vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, normalized_viewer_vector), 0.0), u_material_shininess);"+
                                "phong_ads_color=ambient + diffuse + specular;"+
                                "}"+
                                "else"+
                                "{"+
                                "phong_ads_color = vec3(1.0, 1.0, 1.0);"+
                                "}"+
                                "FragColor = vec4(phong_ads_color, 1.0) * texture(texSampler, outTexcoord);"+
                                "}"

                );

        GLES32.glShaderSource(fragmentShaderObject2, fragmentShaderSourceCode1);

        //compile shader & check for errors
        GLES32.glCompileShader(fragmentShaderObject2);
        iShaderCompiledStatus[0] = 0;
        iInfoLogLength[0] = 0;
        szInfoLog = null;
        GLES32.glGetShaderiv(fragmentShaderObject2, GLES32.GL_COMPILE_STATUS,
                iShaderCompiledStatus,0);

        if(iShaderCompiledStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetShaderiv(fragmentShaderObject2,
                    GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength,0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetShaderInfoLog(fragmentShaderObject2);
                System.out.println("MPD: Fragment Shader 2 Compilation Log = " +szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        shaderProgramObject2 = GLES32.glCreateProgram();

        GLES32.glAttachShader(shaderProgramObject2, vertexShaderObject);
        GLES32.glAttachShader(shaderProgramObject2, fragmentShaderObject2);

        GLES32.glBindAttribLocation(shaderProgramObject2, GLESMacros.AMC_ATTRIBUTE_POSITION, "vPosition");
        GLES32.glBindAttribLocation(shaderProgramObject2, GLESMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");
        GLES32.glBindAttribLocation(shaderProgramObject2, GLESMacros.AMC_ATTRIBUTE_TEXCOORD0, "vTexcoord");
        GLES32.glLinkProgram(shaderProgramObject2);
        iShaderProgramLinkStatus = new int[1];
        iInfoLogLength[0] = 0;
        szInfoLog = null;
        GLES32.glGetProgramiv(shaderProgramObject2, GLES32.GL_LINK_STATUS,
                iShaderProgramLinkStatus, 0);

        if(iShaderProgramLinkStatus[0] == GLES32.GL_FALSE)
        {
            GLES32.glGetProgramiv(shaderProgramObject2,
                    GLES32.GL_INFO_LOG_LENGTH, iInfoLogLength, 0);
            if(iInfoLogLength[0] > 0)
            {
                szInfoLog = GLES32.glGetProgramInfoLog(shaderProgramObject2);
                System.out.println("MPD: Shader Program 2 Link Log = " + szInfoLog);
                uninitialize();
                System.exit(0);
            }
        }

        modelMatrixUniform1 = GLES32.glGetUniformLocation(shaderProgramObject2, "u_model_matrix");
        viewMatrixUniform1 = GLES32.glGetUniformLocation(shaderProgramObject2, "u_view_matrix");

        projectionMatrixUniform1 = GLES32.glGetUniformLocation(shaderProgramObject2, "u_projection_matrix");

        doubleTapUniform1 = GLES32.glGetUniformLocation(shaderProgramObject2, "u_double_tap");
        laUniform1 = GLES32.glGetUniformLocation(shaderProgramObject2, "u_La");
        ldUniform1 = GLES32.glGetUniformLocation(shaderProgramObject2, "u_Ld");
        lsUniform1 = GLES32.glGetUniformLocation(shaderProgramObject2, "u_Ls");
        kaUniform1 = GLES32.glGetUniformLocation(shaderProgramObject2, "u_Ka");
        kdUniform1 = GLES32.glGetUniformLocation(shaderProgramObject2, "u_Kd");
        ksUniform1 = GLES32.glGetUniformLocation(shaderProgramObject2, "u_Ks");
        lightPositionUniform1 = GLES32.glGetUniformLocation(shaderProgramObject2, "u_light_position");

        materialShininessUniform1 = GLES32.glGetUniformLocation(shaderProgramObject2, "u_material_shininess");
        samplerUniform = GLES32.glGetUniformLocation(shaderProgramObject2, "texSampler");

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        Sphere sphere=new Sphere();
        float sphere_vertices[]=new float[1146];
        float sphere_normals[]=new float[1146];
        float sphere_textures[]=new float[764];
        short sphere_elements[]=new short[2280];
        sphere.getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
        numVertices = sphere.getNumberOfSphereVertices();
        numElements = sphere.getNumberOfSphereElements();


        GLES32.glGenVertexArrays(1, vao_sphere, 0);
        GLES32.glBindVertexArray(vao_sphere[0]);

        GLES32.glGenBuffers(1, vbo_sphere_position, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_sphere_position[0]);

        ByteBuffer byteBuffer = ByteBuffer.allocateDirect(sphere_vertices.length  * 4);

        byteBuffer.order(ByteOrder.nativeOrder());

        FloatBuffer verticesBuffer = byteBuffer.asFloatBuffer();
        verticesBuffer.put(sphere_vertices);
        verticesBuffer.position(0);

        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
                sphere_vertices.length *4,
                verticesBuffer,
                GLES32.GL_STATIC_DRAW);

        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION,
                3,
                GLES32.GL_FLOAT,
                false, 0, 0);

        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);
        //color
        //GLES32.glVertexAttrib3f(GLESMacros.MPD_ATTRIBUTE_COLOR, 0.39f, 0.58f, 0.9294f); //cornflower Blue

        //GLES32.glEnableVertexAttribArray(GLESMacros.MPD_ATTRIBUTE_VERTEX);
        GLES32.glGenBuffers(1, vbo_sphere_normal, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_sphere_normal[0]);

        byteBuffer = ByteBuffer.allocateDirect(sphere_normals.length * 4);

        byteBuffer.order(ByteOrder.nativeOrder());

        verticesBuffer = byteBuffer.asFloatBuffer();
        verticesBuffer.put(sphere_normals);
        verticesBuffer.position(0);

        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
                sphere_normals.length *4,
                verticesBuffer,
                GLES32.GL_STATIC_DRAW);

        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_NORMAL,
                3,
                GLES32.GL_FLOAT,
                false, 0, 0);

        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_NORMAL);


        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

        // element vbo
        GLES32.glGenBuffers(1,vbo_sphere_element,0);
        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER,vbo_sphere_element[0]);

        byteBuffer=ByteBuffer.allocateDirect(sphere_elements.length * 2);
        byteBuffer.order(ByteOrder.nativeOrder());
        ShortBuffer elementsBuffer= byteBuffer.asShortBuffer();
        elementsBuffer.put(sphere_elements);
        elementsBuffer.position(0);

        GLES32.glBufferData(GLES32.GL_ELEMENT_ARRAY_BUFFER,
                sphere_elements.length * 2,
                elementsBuffer,
                GLES32.GL_STATIC_DRAW);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER,0);

        GLES32.glBindVertexArray(0);


///////////////////////////////////////////////////////////////////////////////////////////////////


        //Square
      //  float cubeVertices[] = new float[]
       float cubeVertices[] = new float[]
            {
                    // top
                    1.0f, 1.0f, -1.0f,
                    -1.0f, 1.0f, -1.0f,
                    -1.0f, 1.0f, 1.0f,
                    1.0f, 1.0f, 1.0f,

                    //bottom
                    1.0f, -1.0f, 1.0f,
                    -1.0f, -1.0f, 1.0f,
                    -1.0f, -1.0f, -1.0f,
                    1.0f, -1.0f, -1.0f,
                    //front
                    1.0f, 1.0f, 1.0f,
                    -1.0f, 1.0f, 1.0f,
                    -1.0f, -1.0f, 1.0f,
                    1.0f, -1.0f, 1.0f,
                    //back
                    1.0f, -1.0f, -1.0f,
                    -1.0f, -1.0f, -1.0f,
                    -1.0f, 1.0f, -1.0f,
                    1.0f, 1.0f, -1.0f,
                    //right
                    -1.0f, 1.0f, 1.0f,
                    -1.0f, 1.0f, -1.0f,
                    -1.0f, -1.0f, -1.0f,
                    -1.0f, -1.0f, 1.0f,
                    //left
                    1.0f, 1.0f, -1.0f,
                    1.0f, 1.0f, 1.0f,
                    1.0f, -1.0f, 1.0f,
                    1.0f, -1.0f, -1.0f

            };

	float cubeNormals[] = new float[]
            {
                    // top
                    0.0f, 1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,

                    // bottom
                    0.0f, -1.0f, 0.0f,
                    0.0f, -1.0f, 0.0f,
                    0.0f, -1.0f, 0.0f,
                    0.0f, -1.0f, 0.0f,

                    // front
                    0.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f,

                    // back
                    0.0f, 0.0f, -1.0f,
                    0.0f, 0.0f, -1.0f,
                    0.0f, 0.0f, -1.0f,
                    0.0f, 0.0f, -1.0f,

                    // right
                    -1.0f, 0.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f,

                    // left
                    1.0f, 0.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    1.0f, 0.0f, 0.0f

            };


	float cubeTexcoords[] = new float[]
            {
                    0.0f, 0.0f,
                    1.0f, 0.0f,
                    1.0f, 1.0f,
                    0.0f, 1.0f,

                    1.0f, 0.0f,
                    1.0f, 1.0f,
                    0.0f, 1.0f,
                    0.0f, 0.0f,

                    1.0f, 0.0f,
                    1.0f, 1.0f,
                    0.0f, 1.0f,
                    0.0f, 0.0f,

                    0.0f, 0.0f,
                    1.0f, 0.0f,
                    1.0f, 1.0f,
                    0.0f, 1.0f,

                    0.0f, 1.0f,
                    0.0f, 0.0f,
                    1.0f, 0.0f,
                    1.0f, 1.0f,

                    1.0f, 1.0f,
                    0.0f, 1.0f,
                    0.0f, 0.0f,
                    1.0f, 0.0f
            };


        //Create vao_cube
        GLES32.glGenVertexArrays(1, vao_cube, 0);
        GLES32.glBindVertexArray(vao_cube[0]);

        //Create and bind vbo_position_cube
        GLES32.glGenBuffers(1,vbo_position_cube,0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_position_cube[0]);
        //we need to send Bufferdata but we have array so we need to create buffer first
        //in 5 steps
        //Step 1
        //allocate the buffer directly from native memory(not from VM memory)
        byteBuffer = ByteBuffer.allocateDirect(cubeVertices.length * 4);
        //Step 2
        //Arrange the Buffer byte order in native byte order
        byteBuffer.order(ByteOrder.nativeOrder());
        //Step 3
        //Create the float type Buffer & Convert our Bytetype Buffer in to float type Buffer
        FloatBuffer positionBuffer = byteBuffer.asFloatBuffer();
        //Step 4
        //Now put your arrays in to this coocked buffer
        positionBuffer.put(cubeVertices);
        //Step 5
        //Set the array at there 0th position of Buffer
        positionBuffer.position(0);
        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cubeVertices.length * 4, positionBuffer, GLES32.GL_STATIC_DRAW);
        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_POSITION, 3, GLES32.GL_FLOAT, false, 0 , 0 );
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_POSITION);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,0);

        //normal
        GLES32.glGenBuffers(1, vbo_normal_cube, 0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_normal_cube[0]);

        byteBuffer = ByteBuffer.allocateDirect(cubeNormals.length * 4);

        byteBuffer.order(ByteOrder.nativeOrder());

        verticesBuffer = byteBuffer.asFloatBuffer();
        verticesBuffer.put(cubeNormals);
        verticesBuffer.position(0);

        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER,
                cubeNormals.length *4,
                verticesBuffer,
                GLES32.GL_STATIC_DRAW);

        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_NORMAL,
                3,
                GLES32.GL_FLOAT,
                false, 0, 0);

        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_NORMAL);

        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, 0);

        //texture
        GLES32.glGenBuffers(1,vbo_texture,0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER, vbo_texture[0]);
        //we need to send Bufferdata but we have array so we need to create buffer first
        //in 5 steps

        //Step 1
        //allocate the buffer directly from native memory(not from VM memory)
        byteBuffer = ByteBuffer.allocateDirect(cubeTexcoords.length * 4);

        //Step 2
        //Arrange the Buffer byte order in native byte order
        byteBuffer.order(ByteOrder.nativeOrder());

        //Step 3
        //Create the float type Buffer & Convert our Bytetype Buffer in to float type Buffer
        positionBuffer = byteBuffer.asFloatBuffer();

        //Step 4
        //Now put your arrays in to this coocked buffer
        positionBuffer.put(cubeTexcoords);

        //Step 5
        //Set the array at there 0th position of Buffer
        positionBuffer.position(0);

        GLES32.glBufferData(GLES32.GL_ARRAY_BUFFER, cubeTexcoords.length * 4, positionBuffer, GLES32.GL_STATIC_DRAW);

        GLES32.glVertexAttribPointer(GLESMacros.AMC_ATTRIBUTE_TEXCOORD0, 2, GLES32.GL_FLOAT, false, 0 , 0 );
        GLES32.glEnableVertexAttribArray(GLESMacros.AMC_ATTRIBUTE_TEXCOORD0);
        GLES32.glBindBuffer(GLES32.GL_ARRAY_BUFFER,0);

//////////////////////////////////////////////////////////////////////////////////

        //framebuffer creation
        GLES32.glGenFramebuffers(1, fbo_new, 0);
        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, fbo_new[0]);

        GLES32.glGenTextures(1, texture_attach0, 0);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, texture_attach0[0]);
        GLES32.glTexStorage2D(GLES32.GL_TEXTURE_2D, 1, GLES32.GL_RGBA8, 1024,1024);
        //GLES32.glTexImage2D(GLES32.GL_TEXTURE_2D, 1, GLES32.GL_RGBA8, 1024, 1024, 0, GLES32.GL_RGBA8, gl.GL_UNSIGNED_SHORT, null);

        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MIN_FILTER, GLES32.GL_LINEAR);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MAG_FILTER, GLES32.GL_LINEAR_MIPMAP_LINEAR);

        GLES32.glGenTextures(1,depth_attach0,0);
        GLES32.glBindTexture(GLES32.GL_ARRAY_BUFFER, depth_attach0[0]);
        GLES32.glTexStorage2D(GLES32.GL_TEXTURE_2D, 1, GLES32.GL_DEPTH_COMPONENT16, 1024, 1024);
        //GLES32.glTexImage2D(GLES32.GL_TEXTURE_2D, 1, GLES20.GL_DEPTH_COMPONENT16, 1024, 1024, 0,  GLES20.GL_DEPTH_COMPONENT16, gl.GL_UNSIGNED_SHORT, null);
       // GLES32.glGenRenderbuffers(1, depth_attach0, 0);
       // GLES32.glBindRenderbuffer(GLES32.GL_RENDERBUFFER, depth_attach0[0]);
       // GLES32.glRenderbufferStorage(GLES32.GL_RENDERBUFFER, GLES32.GL_DEPTH24_STENCIL8, 1024, 1024);

        GLES32.glFramebufferTexture(GLES32.GL_FRAMEBUFFER, GLES32.GL_COLOR_ATTACHMENT0, texture_attach0[0], 0);
        //GLES32.glFramebufferRenderbuffer(GLES32.GL_FRAMEBUFFER, GLES32.GL_DEPTH_ATTACHMENT, GLES32.GL_RENDERBUFFER, depth_attach0[0]);


        GLES32.glFramebufferTexture(GLES32.GL_FRAMEBUFFER, GLES32.GL_DEPTH_ATTACHMENT, depth_attach0[0], 0);
      //  GLES32.glFramebufferTexture2D(GLES32.GL_FRAMEBUFFER, GLES32.GL_COLOR_ATTACHMENT0, GLES32.GL_TEXTURE_2D, texture_attach0[0], 0);
        //GLES32.glFramebufferTexture2D(GLES32.GL_FRAMEBUFFER, GLES32.GL_DEPTH_ATTACHMENT, GLES32.GL_TEXTURE_2D, depth_attach0[0], 0);

        final int[] draw_buffer = { GLES32.GL_COLOR_ATTACHMENT0 };
        GLES32.glDrawBuffers(1, draw_buffer, 0);

        int status = GLES32.glCheckFramebufferStatus(GLES20.GL_FRAMEBUFFER);
        if(status != GLES32.GL_FRAMEBUFFER_COMPLETE)
        {
            System.out.println("MPD: Incomplete framebuffer");
        }
        else {
            System.out.println("MPD: complete framebuffer");
        }

        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, 0);
        // enable depth testing
        GLES32.glEnable(GLES32.GL_DEPTH_TEST);
        // depth test to do
        GLES32.glDepthFunc(GLES32.GL_LEQUAL);
        // We will always cull back faces for better performance
        GLES32.glEnable(GLES32.GL_CULL_FACE);

        // Set the background color
        GLES32.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);


       // doubleTap =0;
        //set projectionMatrix to identity matrix
        Matrix.setIdentityM(perspectiveProjectionMatrix, 0);
    }

    private void resize(int width, int height)
    {
        if(height == 0)
            height = 1;
        GLES32.glViewport(0, 0, width, height);

        gWidth = width;
        gHeight = height;
       /* if(width <= height)
				Matrix.orthoM(orthographicProjectionMatrix, 0, -100.0f, 100.0f,
				(100.0f *(height/width)), (100.0f *(height / width)), -100.0f, 100.0f);

		else
				Matrix.orthoM(orthographicProjectionMatrix, 0, (-100.0f *(width / height)),
				(100.0f * (width/height)), -100.0f, 100.0f, -100.0f, 100.0f);
		*/

        Matrix.perspectiveM(perspectiveProjectionMatrix, 0, 45.0f,
                (float)width/(float)height, 0.1f, 100.0f);
    }

    private void update()
    {
        cube_angle += 1.0f;
        if(cube_angle >= 360.0f)
            cube_angle = 0.0f;
    }
    public void display()
    {
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);

        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, fbo_new[0]);
        final float[] color = new float[] {0.4f, 0.4f, 0.4f, 1.0f};
        GLES32.glViewport(0, 0, 1024, 1024);
        GLES32.glClearBufferfv(GLES32.GL_COLOR, 0, color, 0);
        GLES32.glClearBufferfi(GLES32.GL_DEPTH24_STENCIL8, 0, 1, 0);

        GLES32.glUseProgram(shaderProgramObject1);
        if(doubleTap ==1)
        {
            GLES32.glUniform1i(doubleTapUniform, 1);

            GLES32.glUniform3fv(ldUniform, 1, light_ambient, 0);
            GLES32.glUniform3fv(ldUniform, 1, light_diffuse, 0);
            GLES32.glUniform3fv(lsUniform, 1, light_specular, 0);

            GLES32.glUniform4fv(lightPositionUniform, 1, light_position, 0);

            GLES32.glUniform3fv(kaUniform, 1, material_ambient, 0);
            GLES32.glUniform3fv(kdUniform, 1, material_diffuse, 0);
            GLES32.glUniform3fv(ksUniform, 1, material_specular, 0);
            GLES32.glUniform1f(materialShininessUniform, material_shininess);

        }
        else
        {
            GLES32.glUniform1i(doubleTapUniform, 0);
        }

        float modelMatrix[] = new float[16];
        float[]rotateMatrix = new float[16];
        float viewMatrix[] = new float[16];

        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix,0);
        Matrix.setIdentityM(rotateMatrix,0);

        Matrix.translateM(modelMatrix, 0, 0.0f,0.0f,-10.0f);//perspective change

        Matrix.setRotateM(rotateMatrix,0,cube_angle, 1.0f, 0.0f,0.0f);
        //Matrix.multiplyMM(rotateMatrix,0,rotateMatrix2,0,rotateMatrix,0);
        Matrix.multiplyMM(modelMatrix,0,modelMatrix,0,rotateMatrix,0);

        //multiply modelView and Projection matrix -> modelViewProjection matrix
        //Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);

        //TriangleBlock
        GLES32.glUniformMatrix4fv(modelMatrixUniform, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform, 1, false, viewMatrix, 0);

        GLES32.glUniformMatrix4fv(projectionMatrixUniform, 1, false, perspectiveProjectionMatrix, 0);

       /* GLES32.glBindVertexArray(vao_sphere[0]);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //unbind vao
        GLES32.glBindVertexArray(0);
*/
        GLES32.glBindVertexArray(vao_cube[0]);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 0, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 4, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 8, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 12, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 16, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 20, 4);
        GLES32.glBindVertexArray(0);

        GLES32.glUseProgram(0);

        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, 0);

        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
        GLES32.glViewport(0, 0, (int)gWidth, (int)gHeight);

        GLES32.glUseProgram(shaderProgramObject2);
        if(doubleTap ==1)
        {
            GLES32.glUniform1i(doubleTapUniform1, 1);

            GLES32.glUniform3fv(ldUniform1, 1, light_ambient, 0);
            GLES32.glUniform3fv(ldUniform1, 1, light_diffuse, 0);
            GLES32.glUniform3fv(lsUniform1, 1, light_specular, 0);

            GLES32.glUniform4fv(lightPositionUniform1, 1, light_position, 0);

            GLES32.glUniform3fv(kaUniform1, 1, material_ambient, 0);
            GLES32.glUniform3fv(kdUniform1, 1, material_diffuse, 0);
            GLES32.glUniform3fv(ksUniform1, 1, material_specular, 0);
            GLES32.glUniform1f(materialShininessUniform1, material_shininess);

        }
        else
        {
            GLES32.glUniform1i(doubleTapUniform1, 0);
        }


        Matrix.setIdentityM(modelMatrix, 0);
        Matrix.setIdentityM(viewMatrix,0);
        Matrix.setIdentityM(rotateMatrix,0);

        Matrix.translateM(modelMatrix, 0, 0.0f,0.0f,-5.0f);//perspective change
        Matrix.setRotateM(rotateMatrix,0,cube_angle, 0.0f, 1.0f,0.0f);
        //Matrix.multiplyMM(rotateMatrix,0,rotateMatrix2,0,rotateMatrix,0);
        Matrix.multiplyMM(modelMatrix,0,modelMatrix,0,rotateMatrix,0);


        //multiply modelView and Projection matrix -> modelViewProjection matrix
        //Matrix.multiplyMM(modelViewProjectionMatrix, 0, perspectiveProjectionMatrix, 0, modelViewMatrix, 0);

        //TriangleBlock
        GLES32.glUniformMatrix4fv(modelMatrixUniform1, 1, false, modelMatrix, 0);
        GLES32.glUniformMatrix4fv(viewMatrixUniform1, 1, false, viewMatrix, 0);

        GLES32.glUniformMatrix4fv(projectionMatrixUniform1, 1, false, perspectiveProjectionMatrix, 0);

        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D, texture_attach0[0]);
       /* GLES32.glBindVertexArray(vao_sphere[0]);

        GLES32.glBindBuffer(GLES32.GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element[0]);
        GLES32.glDrawElements(GLES32.GL_TRIANGLES, numElements, GLES32.GL_UNSIGNED_SHORT, 0);

        //unbind vao
        GLES32.glBindVertexArray(0);
*/

        GLES32.glBindVertexArray(vao_cube[0]);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 0, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 4, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 8, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 12, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 16, 4);
        GLES32.glDrawArrays(GLES32.GL_TRIANGLE_FAN, 20, 4);
        GLES32.glBindVertexArray(0);

        GLES32.glUseProgram(0);

        //flush
        requestRender();

    }

    void uninitialize()
    {
        // code
        // destroy vao


        if(vao_sphere[0] != 0)
        {
            GLES32.glDeleteVertexArrays(1, vao_sphere, 0);
            vao_sphere[0]=0;
        }



        // destroy vao
        if(vbo_sphere_position[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_sphere_position, 0);
            vbo_sphere_position[0]=0;
        }
        if(vbo_sphere_normal[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_sphere_normal, 0);
            vbo_sphere_normal[0]=0;
        }
        if(vbo_sphere_element[0] != 0)
        {
            GLES32.glDeleteBuffers(1, vbo_sphere_element, 0);
            vbo_sphere_element[0]=0;
        }
        if(shaderProgramObject1 != 0)
        {
            if(vertexShaderObject != 0)
            {
                // detach vertex shader from shader program object
                GLES32.glDetachShader(shaderProgramObject1, vertexShaderObject);
                // delete vertex shader object
                GLES32.glDeleteShader(vertexShaderObject);
                vertexShaderObject = 0;
            }

            if(fragmentShaderObject1 != 0)
            {
                // detach fragment  shader from shader program object
                GLES32.glDetachShader(shaderProgramObject1, fragmentShaderObject1);
                // delete fragment shader object
                GLES32.glDeleteShader(fragmentShaderObject1);
                fragmentShaderObject1 = 0;
            }

        }

        if(shaderProgramObject2 != 0) {
            if (fragmentShaderObject2 != 0) {
                // detach fragment  shader from shader program object
                GLES32.glDetachShader(shaderProgramObject2, fragmentShaderObject2);
                // delete fragment shader object
                GLES32.glDeleteShader(fragmentShaderObject2);
                fragmentShaderObject2 = 0;
            }
        }
        // delete shader program object
        if(shaderProgramObject1 != 0)
        {
            GLES32.glDeleteProgram(shaderProgramObject1);
            shaderProgramObject1 = 0;
        }
        if(shaderProgramObject2 != 0)
        {
            GLES32.glDeleteProgram(shaderProgramObject2);
            shaderProgramObject2 = 0;
        }
    }
}

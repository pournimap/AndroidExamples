package com.astromedicomp.GeometryShader;

import android.content.Context; //for drawing context related
import android.opengl.GLSurfaceView;//surface view
import javax.microedition.khronos.opengles.GL10;

import javax.microedition.khronos.egl.EGLConfig;

import android.opengl.GLES32; //OPpenGLES 3.2
import android.view.MotionEvent;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.GestureDetector.OnDoubleTapListener;

public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer, OnGestureListener, OnDoubleTapListener
{
	private final Context context;

	private GestureDetector gestureDetector;

	private BlueScreen blueScreen;
	
	public GLESView(Context drawingContext)
	{
		super(drawingContext);
		context = drawingContext;

		setEGLContextClientVersion(3);

		setRenderer(this);

		setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY); //OnScreen Rendering
	
		gestureDetector = new GestureDetector(context, this, null, false);

		gestureDetector.setOnDoubleTapListener(this);
	}

	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config)
	{
		String version = gl.glGetString(GL10.GL_VERSION);
		System.out.println("MALATI: " +version);

		blueScreen = new BlueScreen();
		
		blueScreen.initialize();
	}

	@Override
	public void onSurfaceChanged(GL10 unused, int width, int height)
	{
		blueScreen.resize(width, height);
	}

	@Override
	public void onDrawFrame(GL10 unused)
	{
		blueScreen.draw();
		requestRender();
	}

	@Override
	public boolean onTouchEvent(MotionEvent e)
	{
		int eventAction = e.getAction();
		if(!gestureDetector.onTouchEvent(e))
			super.onTouchEvent(e);
		return (true);
	}

	@Override
	public boolean onDoubleTap(MotionEvent e)
	{
		System.out.println("MALATI : " + " Double Tap" );
		return (true);
	}

	@Override
	public boolean onDoubleTapEvent(MotionEvent e)
	{
		return (true);
	}

	@Override
	public boolean onSingleTapConfirmed(MotionEvent e)
	{
		System.out.println("MALATI : " + " Single Tap" );
		return (true);
	}

	@Override
	public boolean onDown(MotionEvent e)
	{
		return (true);
	}
	
	@Override
	public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY)
	{
		System.out.println("MALATI : " + " Swipe" );
		return (true);
	}

	@Override
	public void onLongPress(MotionEvent e)
	{
		System.out.println("MALATI : " + " Long Press" );
	}

	@Override
	public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY)
	{
		System.out.println("MALATI : " + "Scroll" );
		System.exit(0);
		return(true);
	}

	@Override
	public void onShowPress(MotionEvent e)
	{

	}

	@Override 
	public boolean onSingleTapUp(MotionEvent e)
	{
		return (true);
	}

	/*private void initialize(GL10 gl)
	{
		GLES32.glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	}

	private void resize(int width, int height)
	{
		GLES32.glViewport(0, 0 , width, height);
	}

	public void draw()
	{
		GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_DEPTH_BUFFER_BIT);
	}*/
}



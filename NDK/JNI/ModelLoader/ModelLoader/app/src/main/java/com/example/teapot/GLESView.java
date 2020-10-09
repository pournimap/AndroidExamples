package com.example.teapot;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.GestureDetector;
import android.view.MotionEvent;

import javax.microedition.khronos.opengles.GL10;

public class GLESView extends GLSurfaceView implements GLSurfaceView.Renderer, GestureDetector.OnGestureListener , GestureDetector.OnDoubleTapListener{
    private GestureDetector gestureDetector;

    private float xCoord;
    private float yCoord;

    public GLESView(Context context){
        super(context);
        //set EGLContext to current supported version of Opengl-ES
        setEGLContextClientVersion(3);

        //set renderer for drawing on the GLSurfaceView
        //  MyRenderer renderer = new MyRenderer();
        setRenderer(this);

        //render the view only when there is change in drawing data
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        gestureDetector = new GestureDetector(context, this, null, false);
        gestureDetector.setOnDoubleTapListener(this);
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent motionEvent) {
        return false;
    }

    @Override
    public boolean onDoubleTap(MotionEvent motionEvent) {
        return false;
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent motionEvent) {
        return false;
    }

    @Override
    public boolean onDown(MotionEvent motionEvent) {
        return false;
    }

    @Override
    public void onShowPress(MotionEvent motionEvent) {

    }

    @Override
    public boolean onSingleTapUp(MotionEvent motionEvent) {
        return false;
    }

    @Override
    public boolean onScroll(MotionEvent motionEvent, MotionEvent motionEvent1, float v, float v1) {
        System.exit(0);
        return false;
    }

    @Override
    public boolean onTouchEvent(MotionEvent e){
      /*  int eventaction = e.getAction();
        if(!gestureDetector.onTouchEvent(e))
            super.onTouchEvent(e);*/

      float y = e.getY();
      float x = e.getX();
      switch(e.getAction())
      {
          case MotionEvent.ACTION_DOWN:
              xCoord = x;
              yCoord = y;
              break;
          default:
              return super.onTouchEvent(e);
      }
      return (true);
    }
    @Override
    public void onLongPress(MotionEvent motionEvent) {

    }

    @Override
    public boolean onFling(MotionEvent motionEvent, MotionEvent motionEvent1, float v, float v1) {
        return false;
    }


    // private static class MyRenderer implements GLSurfaceView.Renderer{
    @Override
    public void onDrawFrame(GL10 gl)
    {
        GLESNativeLib.display();
        requestRender();
    }

    @Override
    public void onSurfaceCreated(GL10 gl10, javax.microedition.khronos.egl.EGLConfig eglConfig) {
        GLESNativeLib.init();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height){
        GLESNativeLib.resize(width, height);
    }
}

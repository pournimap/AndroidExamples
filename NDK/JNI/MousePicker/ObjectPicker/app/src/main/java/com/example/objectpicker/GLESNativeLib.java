package com.example.objectpicker;

public class GLESNativeLib {
    static {
        System.loadLibrary("native-lib");
    }

    public static native void init();
    public static native void resize(int width, int height);
    public static native void display(float xCoord, float yCoord);
    public static native void AddingTouchInputCoordinates(float xCoord, float yCoord);
}

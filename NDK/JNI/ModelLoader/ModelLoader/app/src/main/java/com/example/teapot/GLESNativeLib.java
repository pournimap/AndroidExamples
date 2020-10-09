package com.example.teapot;

import android.content.res.AssetManager;

public class GLESNativeLib {

    public static native void init();
    public static native void resize(int width, int height);
    public static native void display();
    public static native void readAssetsNative(AssetManager assetManager, String pathToInternalDir);
}

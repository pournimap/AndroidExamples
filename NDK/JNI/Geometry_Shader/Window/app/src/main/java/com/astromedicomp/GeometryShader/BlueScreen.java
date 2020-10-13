package com.astromedicomp.GeometryShader;

public class BlueScreen {
	static {
		System.loadLibrary("native-lib");
	}
	
	public native void initialize();
	
	public native void resize(int width, int height);
	
	public native void draw();
	
	
}
package com.astromedicomp.GeometryShader;

import android.app.Activity;
import android.os.Bundle;

import android.view.Window;
import android.view.WindowManager;
import android.graphics.Color;
import android.content.pm.ActivityInfo;

public class MainActivity extends Activity {
	
	
    private GLESView glesView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

	requestWindowFeature(Window.FEATURE_NO_TITLE);
	
	getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
			WindowManager.LayoutParams.FLAG_FULLSCREEN);
	setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

	getWindow().getDecorView().setBackgroundColor(Color.rgb(0,0,0));

	glesView = new GLESView(this);


    setContentView(glesView);
    }
}

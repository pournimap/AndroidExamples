package com.astromedicomp.geometryShaderByNDK;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
//import android.widget.TextView;
import android.view.Window;
import android.view.WindowManager;
import android.graphics.Color;
import android.content.pm.ActivityInfo;

public class MainActivity extends AppCompatActivity {

	private GLESView glesView;
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
       
		supportRequestWindowFeature(Window.FEATURE_NO_TITLE);
		
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
			WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

		getWindow().getDecorView().setBackgroundColor(Color.rgb(0,0,0));

		glesView = new GLESView(this);


		setContentView(glesView);
    }

    
}

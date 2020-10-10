package com.example.clouds;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;


import android.view.Window;
import android.view.WindowManager;
import android.graphics.Color;
import android.content.pm.ActivityInfo;
import android.media.MediaPlayer;

public class MainActivity extends AppCompatActivity {

    private GLESView glesView;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        requestWindowFeature(Window.FEATURE_NO_TITLE);
       // setContentView(R.layout.activity_main);



        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

        getWindow().getDecorView().setBackgroundColor(Color.rgb(0,0,0));

        glesView = new GLESView(this);

        getSupportActionBar().hide();

        MediaPlayer mediaPlayer = MediaPlayer.create(this, R.raw.meditations);

        mediaPlayer.start();
        setContentView(glesView);
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
   // public native String stringFromJNI();
}

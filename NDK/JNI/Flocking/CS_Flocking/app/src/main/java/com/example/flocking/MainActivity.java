package com.example.flocking;

import androidx.appcompat.app.AppCompatActivity;

import android.media.MediaPlayer;
import android.net.rtp.AudioStream;
import android.os.Bundle;

//import android.widget.TextView;
import android.view.Window;
import android.view.WindowManager;
import android.graphics.Color;
import android.content.pm.ActivityInfo;

import java.util.concurrent.Semaphore;

/*import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;*/


//import java.io.File;


public class MainActivity extends AppCompatActivity {

   
    private GLESView glesView;
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }
    public MediaPlayer mediaPlayer;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        /*setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());*/
        supportRequestWindowFeature(Window.FEATURE_NO_TITLE);

        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

        getWindow().getDecorView().setBackgroundColor(Color.rgb(0,0,0));

        glesView = new GLESView(this);


        //PlayMusic
       // InputStream music;
        //MediaPlayer mediaPlayer = new MediaPlayer();
        try
        {
           /* music = new FileInputStream(new File("raw/BITS.wav"));
            AudioStream audios = new AudioStream(music);
            AudioPlayer.player.start(audios);*/
           //java.applet.AudioClip clip = Applet.newAudioClip()
            mediaPlayer = MediaPlayer.create(this, R.raw.bits);


            mediaPlayer.start();

        }
        catch (Exception exc)
        {
            exc.printStackTrace(System.out);
        }

        setContentView(glesView);
        //mediaPlayer.release();
    }

    @Override
    protected void onPause()
    {
        mediaPlayer.release();
        super.onPause();
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    //public native String stringFromJNI();
}

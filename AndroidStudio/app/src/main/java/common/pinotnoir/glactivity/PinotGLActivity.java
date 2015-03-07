package common.pinotnoir.glactivity;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;

import common.pinotnoir.Helper;

public class PinotGLActivity extends Activity {
    private GLSurfaceView view;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Helper.setContext(getApplicationContext());
        super.onCreate(savedInstanceState);

        view = new PinotGLSurfaceView(this);
        setContentView(view);
    }

    @Override
    public void onResume() {
        super.onResume();
        view.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
        view.onPause();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        switch(event.getAction()) {
            case MotionEvent.ACTION_UP:
                x = event.getX();
                y = event.getY();
                tapped = true;
                Log.v("TEST", String.format("ACTION_UP %f,%f", x, y));
                return true;
        }
        super.onTouchEvent(event);
        return false;
    }
    public static boolean tapped;
    public static float x;
    public static float y;
}

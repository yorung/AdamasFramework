package common.pinotnoir.glactivity;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.widget.Toast;

import common.pinotnoir.Helper;

public class PinotGLActivity extends Activity {
    private GLSurfaceView view;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Helper.setContext(getApplicationContext(), this);

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
            case MotionEvent.ACTION_MOVE:
                x = event.getX();
                y = event.getY();
                return true;
            case MotionEvent.ACTION_UP:
                x = event.getX();
                y = event.getY();
                pressed = false;
                return true;
            case MotionEvent.ACTION_DOWN:
                x = event.getX();
                y = event.getY();
                pressed = true;
                return true;
        }
        super.onTouchEvent(event);
        return false;
    }
    public static boolean pressed;
    public static float x;
    public static float y;
}

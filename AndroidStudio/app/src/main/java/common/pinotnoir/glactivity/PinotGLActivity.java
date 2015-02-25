package common.pinotnoir.glactivity;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;

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
}

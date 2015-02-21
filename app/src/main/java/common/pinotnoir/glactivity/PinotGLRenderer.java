package common.pinotnoir.glactivity;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class PinotGLRenderer implements GLSurfaceView.Renderer {
    private int width, height;
    private int lastWidth, lastHeight;
    public void onSurfaceCreated(GL10 unused, EGLConfig config) {
        GLES20.glClearColor(0.5f, 0.2f, 0.0f, 1.0f);
    }
    public void onDrawFrame(GL10 unused) {
        if (width <= 0 || height <= 0) {
            return;
        }
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);
        if (width != lastWidth && height != lastHeight) {
        //    CommonJNI.destroy();
        //    CommonJNI.init(width, height);
            lastWidth = width;
            lastHeight = height;
        }
        float aspect = (float)width / (float)height;
        //CommonJNI.update(aspect, 0.5f);
    }
    public void onSurfaceChanged(GL10 unused, int width_, int height_) {
        width = width_;
        height = height_;
    }
}

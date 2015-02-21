package common.pinotnoir.glactivity;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import common.pinotnoir.Native;

public class PinotGLRenderer implements GLSurfaceView.Renderer {
    private int width, height;
    private int lastWidth, lastHeight;
    public void onSurfaceCreated(GL10 unused, EGLConfig config) {
    }
    public void onDrawFrame(GL10 unused) {
        if (width <= 0 || height <= 0) {
            return;
        }
        if (width != lastWidth && height != lastHeight) {
            Native.destroy();
            Native.init(width, height);
            lastWidth = width;
            lastHeight = height;
        }
        float aspect = (float)width / (float)height;
        Native.update(aspect, 0.5f);
    }
    public void onSurfaceChanged(GL10 unused, int width_, int height_) {
        width = width_;
        height = height_;
    }
}

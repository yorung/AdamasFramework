package common.pinotnoir.glactivity;

import android.content.Context;
import android.opengl.GLSurfaceView;

public class PinotGLSurfaceView extends GLSurfaceView {
    private final PinotGLRenderer renderer;
    public PinotGLSurfaceView(Context context)
    {
        super(context);
        setEGLContextClientVersion(2);
        renderer = new PinotGLRenderer();
        setRenderer(renderer);
    }
}

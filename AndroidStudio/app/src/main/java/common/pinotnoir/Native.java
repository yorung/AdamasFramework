package common.pinotnoir;

public class Native {
    static {
        System.loadLibrary("AdamasNative");
    }
    public static native void init(int screenW, int screenH);
    public static native void destroy();
    public static native void update(int screenW, int screenH, float inputX, float inputY, boolean pressed);
}

package common.pinotnoir;

public class Native {
    static {
        System.loadLibrary("AdamasNative");
    }
    public static native void init(int screenW, int screenH);
    public static native void destroy();
    public static native void update(int screenW, int screenH, float offset);
    public static native void onTap(float x, float y);
}

package common.pinotnoir;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.media.MediaPlayer;
import android.util.Log;

import java.io.IOException;

public class Bgm {
    static final String TAG = "Bgm";
    static String fileName = null;
    static private MediaPlayer mp = new MediaPlayer();
    static private void play(Context context) {
        if (fileName == null) {
            return;
        }
        try {
            mp.reset();
            AssetFileDescriptor f = context.getAssets().openFd(fileName);
            mp.setLooping(true);
            mp.setDataSource(f.getFileDescriptor(), f.getStartOffset(), f.getLength());
            mp.prepare();
            mp.start();
        } catch (IllegalStateException e) {
            Log.v(TAG, e.getMessage());
        } catch (IllegalArgumentException e) {
            Log.v(TAG, e.getMessage());
        } catch (IOException e) {
            Log.v(TAG, e.getMessage());
        }
    }
    static public void playBgm(Context context, String fileName_) {
        fileName = fileName_;
        play(context);
    }
    static public void onResume(Context context) {
        play(context);
    }
    static public void onPause() {
        mp.reset();
    }
}

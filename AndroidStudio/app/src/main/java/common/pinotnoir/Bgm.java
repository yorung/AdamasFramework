package common.pinotnoir;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.media.MediaPlayer;
import android.util.Log;

import java.io.IOException;

public class Bgm {

    static private MediaPlayer mp = new MediaPlayer();

    static public void onResume(Context context) {
        try {
            mp.reset();
            AssetFileDescriptor f = context.getAssets().openFd("sound/background.mp3");
            mp.setLooping(true);
            mp.setDataSource(f.getFileDescriptor(), f.getStartOffset(), f.getLength());
            mp.prepare();
            mp.start();
        } catch (IllegalStateException e) {
            Log.v("sound IllegalStateException", e.getMessage());
        } catch (IllegalArgumentException e) {
            Log.v("sound IllegalArgumentException", e.getMessage());
        } catch (IOException e) {
            Log.v("sound IOException", e.getMessage());
        }
    }

    static public void onPause() {
        mp.reset();
    }
}

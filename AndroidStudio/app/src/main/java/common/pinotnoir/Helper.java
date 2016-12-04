package common.pinotnoir;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.opengl.GLES20;
import android.opengl.GLUtils;
import android.util.Log;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.lang.ref.WeakReference;
import java.nio.ByteBuffer;

import javax.microedition.khronos.opengles.GL10;

public class Helper {
    private final static String TAG = "Helper";
    private static Context context;
    private static WeakReference<Activity> activity;
    public static void setContext(Context c, Activity a) {
        context = c;
        activity = new WeakReference<Activity>(a);
    }

    public static void toast(final String text) {
        Activity a = activity.get();
        if (a == null) {
            return;
        }
        a.runOnUiThread( new Runnable() {
            public void run() {
                Toast.makeText(context, text, Toast.LENGTH_LONG).show();
            }
        } );
    }
/*
    public static void strMessageBox(String txt, String type) {
        Activity a = activity.get();
        if (a == null) {
            return;
        }
        AlertDialog.Builder d = new AlertDialog.Builder(a).create();
        d.setTitle("AlertDialog");
        d.setMessage(txt);
        a.runOnUiThread( new Runnable() {
            public void run() {
                Toast.makeText(context, text, Toast.LENGTH_LONG).show();
            }
        } );
    }
*/
    public static byte[] loadIntoBytes(String fileName) {
        AssetManager assetManager = context.getAssets();
        try {
            InputStream is = assetManager.open(fileName);
            byte buf[] = new byte[is.available()];
            is.read(buf);
            return buf;
        } catch (IOException e) {
        }
        return null;
    }

    public static byte[] bitmapToByteArray(Bitmap b)
    {
        ByteBuffer buf = ByteBuffer.allocate(b.getWidth() * b.getHeight() * 4);
        buf.position(0);
        b.copyPixelsToBuffer(buf);
        return buf.array();
    }

    public static byte[] makeFontBitmap(String font, String code, int size, int[] arrayOfPos) {
        Canvas c = new Canvas();
        Paint p = new Paint();
//        Log.v(TAG, String.format("makeFontBitmap called(Java): font=%s code=%s density=%f", font, code, density));
        p.setTextSize((float)size);
        p.setAntiAlias(true);

        Rect textBounds = new Rect();
        p.getTextBounds(code, 0, code.length(), textBounds);
//        Log.v(TAG, String.format("makeFontBitmap textBounds: %d,%d,%d,%d", textBounds.left, textBounds.top, textBounds.right, textBounds.bottom));

        Rect textBoundsAxA = new Rect();
        String axa = String.format("A%sA", code);
        p.getTextBounds(axa, 0, axa.length(), textBoundsAxA);
        Rect textBoundsAA = new Rect();
        String aa = "AA";
        p.getTextBounds(aa, 0, aa.length(), textBoundsAA);

        // cache.distDelta = Vec2(0, 0);
        arrayOfPos[0] = textBounds.left;
        arrayOfPos[1] = textBounds.top;

        // cache.srcWidth = Vec2(16, 16);
        arrayOfPos[2] = textBounds.width();
        arrayOfPos[3] = textBounds.height();

        // cache.step = 16;
//		arrayOfPos[4] = textBounds.width() + 1;
        arrayOfPos[4] = textBoundsAxA.width() - textBoundsAA.width();

        if (textBounds.width() == 0 || textBounds.height() == 0) {
            Log.v(TAG, "makeFontBitmap: empty");
            return null;
        }

        Bitmap b = Bitmap.createBitmap(textBounds.width(), textBounds.height(), Bitmap.Config.ARGB_8888);
        c.setBitmap(b);

        Rect r = new Rect(0, 0, textBounds.width(), textBounds.height());
//		p.setColor(Color.RED);
        p.setARGB(0, 0, 0, 0);
        c.drawRect(r, p);
        p.setARGB(255, 255, 255, 255);

//        Log.v(TAG, "makeFontBitmap: drawText");
        c.drawText(code, -textBounds.left, -textBounds.top, p);
//        Log.v(TAG, String.format("makeFontBitmap: w=%.2f h=%.2f", arrayOfPos[2], arrayOfPos[3]));

        return bitmapToByteArray(b);
    }

    public static byte[] loadImage(String s, int[] size)
    {
        Bitmap img;
        try {
            img = BitmapFactory.decodeStream(context.getAssets().open(s));
        } catch (IOException e) {
            return null;
        }
        size[0] = img.getWidth();
        size[1] = img.getHeight();
        return bitmapToByteArray(img);
    }

    public static int loadTexture(String s){
        Bitmap img;
        try {
            img = BitmapFactory.decodeStream(context.getAssets().open(s));
        } catch (IOException e) {
            return 0;
        }
        int tex[] = new int[1];
        GLES20.glGenTextures(1, tex, 0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, tex[0]);
        GLUtils.texImage2D(GL10.GL_TEXTURE_2D, 0, img, 0);
        GLES20.glGenerateMipmap(GLES20.GL_TEXTURE_2D);
        GLES20.glTexParameteri(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MIN_FILTER, GL10.GL_LINEAR_MIPMAP_LINEAR);
        GLES20.glTexParameteri(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MAG_FILTER, GL10.GL_LINEAR);
        GLES20.glTexParameteri(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_WRAP_S, GL10.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_WRAP_T, GL10.GL_CLAMP_TO_EDGE);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        img.recycle();
        return tex[0];
    }

    public static void playBgm(String fileName) {
        Bgm.playBgm(context, fileName);
    }
}

package com.nick.compress;

import android.graphics.Bitmap;

/**
 * Created by nick on 2017/2/23
 */

public class JpegUtils {
    static {
        System.loadLibrary("native-lib");
    }

    public static native boolean compressBitmap(Bitmap bitmap, int width, int height, String fileName, int quality);
}

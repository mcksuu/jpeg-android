#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/bitmap.h>

extern "C" {
#include "include/jpeglib.h"
}

typedef uint8_t BYTE;
#define TAG "image "
#define LOGE(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)

#define true 1
#define false 0



extern "C"
JNIEXPORT jboolean JNICALL
Java_com_nick_compress_JpegUtils_compressBitmap(JNIEnv *env, jclass type, jobject bitmap,
                                                jint width, jint height, jstring fileName,
                                                jint quality) {

    AndroidBitmapInfo infoColor;
    BYTE *pixelColor;
    BYTE *data;
    BYTE *tempData;
    const char *filename = env->GetStringUTFChars(fileName, 0);

    if ((AndroidBitmap_getInfo(env, bitmap, &infoColor)) < 0) {
        LOGE("解析错误");
        return false;
    }

    if ((AndroidBitmap_lockPixels(env, bitmap, (void **) &pixelColor)) < 0) {
        LOGE("加载失败");
        return false;
    }

    BYTE r, g, b;
    int color;
    data = (BYTE *) malloc(width * height * 3);
    tempData = data;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            color = *((int *) pixelColor);
            r = ((color & 0x00FF0000) >>
                 16);//与操作获得rgb，参考java Color定义alpha color >>> 24 red (color >> 16) & 0xFF
            g = ((color & 0x0000FF00) >> 8);
            b = color & 0X000000FF;

            *data = b;
            *(data + 1) = g;
            *(data + 2) = r;
            data += 3;
            pixelColor += 4;
        }
    }

    AndroidBitmap_unlockPixels(env, bitmap);
    int resultCode = generateJPEG(tempData, width, height, quality, filename, true);

    free(tempData);
    if (resultCode == 0) {
        return false;
    }

    return true;
}

extern "C"
//图片压缩方法
int generateJPEG(BYTE *data, int w, int h, int quality,
                 const char *outfilename, jboolean optimize) {
    int nComponent = 3;

    struct jpeg_compress_struct jcs;

    struct jpeg_error_mgr jem;

    jcs.err = jpeg_std_error(&jem);

    //为JPEG对象分配空间并初始化
    jpeg_create_compress(&jcs);
    //获取文件信息
    FILE *f = fopen(outfilename, "wb");
    if (f == NULL) {
        return 0;
    }
    //指定压缩数据源
    jpeg_stdio_dest(&jcs, f);
    jcs.image_width = w;//image_width->JDIMENSION->typedef unsigned int
    jcs.image_height = h;

    jcs.arith_code = false;
    //input_components为1代表灰度图，在等于3时代表彩色位图图像
    jcs.input_components = nComponent;
    if (nComponent == 1)
        //in_color_space为JCS_GRAYSCALE表示灰度图，在等于JCS_RGB时代表彩色位图图像
        jcs.in_color_space = JCS_GRAYSCALE;
    else
        jcs.in_color_space = JCS_RGB;

    jpeg_set_defaults(&jcs);
    //optimize_coding为TRUE，将会使得压缩图像过程中基于图像数据计算哈弗曼表，由于这个计算会显著消耗空间和时间，默认值被设置为FALSE。
    jcs.optimize_coding = optimize;
    //为压缩设定参数，包括图像大小，颜色空间
    jpeg_set_quality(&jcs, quality, true);
    //开始压缩
    jpeg_start_compress(&jcs, TRUE);

    JSAMPROW row_pointer[1];//JSAMPROW就是一个字符型指针 定义一个变量就等价于=========unsigned char *temp
    int row_stride;
    row_stride = jcs.image_width * nComponent;
    while (jcs.next_scanline < jcs.image_height) {
        row_pointer[0] = &data[jcs.next_scanline * row_stride];
        //写入数据 http://www.cnblogs.com/darkknightzh/p/4973828.html
        jpeg_write_scanlines(&jcs, row_pointer, 1);
    }

    //压缩完毕
    jpeg_finish_compress(&jcs);
    //释放资源
    jpeg_destroy_compress(&jcs);
    fclose(f);

    return 1;
}

extern "C"
jstring
Java_com_nick_compress_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

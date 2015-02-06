#include <jni.h>
#include <android/log.h>
#include <stdlib.h>
#include <android/bitmap.h>
#include "jpge.h"
#include "bigfile_stream.h"

#define DDD __android_log_print(ANDROID_LOG_INFO, "BigBitmap","Debug file=%s line%d",__FILE__,__LINE__ );

#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_INFO, "BigBitmap", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "BigBitmap", __VA_ARGS__))

using namespace jpge;

int ExtractInfoFromBitmap(JNIEnv *env, AndroidBitmapInfo *src_info, jobject src_bitmap) {
	int ret;
	/*
	 * The info->format of both bitmap should be ANDROID_BITMAP_FORMAT_RGBA_8888.
	 * Both bitmaps shall also be in the same dimension.
	 */
	if ((ret = AndroidBitmap_getInfo(env, src_bitmap, src_info)) < 0) {
		LOGE("AndroidBitmap_getInfo(src_bitmap) failed, error=%d", ret);
		return ret;
	}
	return 0;
}

void copy_pixel(void* src,void* dst, int src_format){
	switch (src_format){
		case ANDROID_BITMAP_FORMAT_RGBA_8888:
			memcpy(dst,src,3);
		break;
	}
}

int LockBitmaps(JNIEnv* env, jobject src_bitmap, AndroidBitmapInfo* src_info,
		void** src_pixels) {
	int ret;
	if ((ret = ExtractInfoFromBitmap(env, src_info, src_bitmap)) < 0) {
		return ret;
	}
	if ((ret = AndroidBitmap_lockPixels(env, src_bitmap, src_pixels)) < 0) {
		LOGE("AndroidBitmap_lockPixels(src_bitmap) failed, error=%d", ret);
		return ret;
	}
	return 0;
}
void UnlockBitmaps(JNIEnv* env, jobject src_bitmap) {
	AndroidBitmap_unlockPixels(env, src_bitmap);
}

extern "C" JNIEXPORT
jboolean JNICALL Java_im_whir_buyukce_Buyukce_initNative(JNIEnv* env,
		jclass clazz, jstring filename, jint width, jint height, int channel) {
	DDD
	const char *filename_chars = env->GetStringUTFChars(filename, 0);
	bigfile_stream dst_stream;
	DDD

	if (!dst_stream.open(filename_chars))
		return (jboolean) false;
	int buffer_size = 1024 * 10;
	char* buffer = (char*) malloc(buffer_size * sizeof(char));
	memset(buffer, 255, buffer_size);
	long long filelenght = width * height * channel;
	long long current = 0;
	DDD
	while (current < filelenght) {
		if (current + buffer_size < filelenght) {
			dst_stream.put_buf(buffer, buffer_size);
			current += buffer_size;
		} else {
			dst_stream.put_buf(buffer, filelenght - current);
			current = filelenght;
		}
	}
	DDD
	free(buffer);
	buffer = 0;
	env->ReleaseStringUTFChars(filename, filename_chars);
	DDD
	return (jboolean) dst_stream.close();
}

void printline(const char* msg, void* buf, int len){
	char* tmp = (char*)malloc(len * 8);
	memset(tmp,0,len*8);
	char *buff = (char*)buf;
	for (int i = 0; i < len; i++) {
		char bb[5];
		sprintf(bb,"%x",(buff[i] & 0xff));
		if (strlen(bb) == 1)
			sprintf(tmp + strlen(tmp)," 0%s",bb);
		else
			sprintf(tmp + strlen(tmp)," %s",bb);
	}
	LOGD("%s %s",msg,tmp);
	free(tmp);
}

extern "C" JNIEXPORT
jboolean JNICALL Java_im_whir_buyukce_Buyukce_drawBitmapNative(JNIEnv* env,
		jclass clazz, jstring exportingfilename, jint exportingwidth, jint exportingheight, jint exportingchannel,
		jobject bitmap, jint dest_x, jint dest_y, jint dest_w,jint dest_h,jint direction
		) {
	const char *exportingfilename_chars = env->GetStringUTFChars(exportingfilename, 0);
	AndroidBitmapInfo info;
	void *pixels;
	LockBitmaps(env,bitmap,&info,&pixels);
	FILE* f = fopen(exportingfilename_chars, "r+b");

	LOGD("filename %s",exportingfilename_chars);
	LOGD("exportingwidth %d",exportingwidth);
	LOGD("exportingheight %d",exportingheight);
	LOGD("exportingchannel %d",exportingchannel);

	LOGD("dest_x %d",dest_x);
	LOGD("dest_y %d",dest_y);
	LOGD("dest_w %d",dest_w);
	LOGD("dest_h %d",dest_h);

	LOGD("bitmap format=%d",info.format);
	LOGD("bitmap flag=%d",info.flags);
	LOGD("bitmap height =%d",info.height);
	LOGD("bitmap width=%d",info.width);
	LOGD("bitmap stride =%d",info.stride);
	DDD

	int bitmap_channel = 4;
	int copy_x_to_count;
	if (info.width > dest_w)
		copy_x_to_count = dest_w;
	else
		copy_x_to_count = info.width;

	int copy_y_to_count;
	if (info.height > dest_h)
		copy_y_to_count = dest_h;
	else
		copy_y_to_count = info.height;


	LOGD("COPY W=%d H=%d",copy_x_to_count,copy_y_to_count);

	char* linebuf = (char*)malloc(copy_x_to_count*exportingchannel);
	int exportingbitmapline_len = exportingwidth*exportingchannel;
	char* exportingbitmapline = (char*)malloc(exportingbitmapline_len);

	for (int i = 0; i < copy_y_to_count; i++)
	{
		int seek = (dest_y+i) * exportingwidth * exportingchannel;
		int seek_result = fseek(f,seek,SEEK_SET);
		fread(exportingbitmapline,1,exportingbitmapline_len,f);

		char *copy_from = (char*) (pixels + i * info.width * bitmap_channel);

		for (int j = 0; j < copy_x_to_count; j++) {
			int alpha = copy_from[j * bitmap_channel + 3];
			linebuf[j * exportingchannel] = (copy_from[j * bitmap_channel] * alpha) / 255 + (exportingbitmapline[dest_x*exportingchannel + j*exportingchannel ] * (255-alpha)) / 255;
			linebuf[j * exportingchannel + 1] = (copy_from[j * bitmap_channel + 1] * alpha) / 255 + (exportingbitmapline[dest_x*exportingchannel + j*exportingchannel + 1]* (255-alpha)) / 255;
			linebuf[j * exportingchannel + 2] = (copy_from[j * bitmap_channel + 2] * alpha) / 255 + (exportingbitmapline[dest_x*exportingchannel + j*exportingchannel + 2]* (255-alpha)) / 255;
		}

		seek = (dest_y+i) * exportingwidth * exportingchannel + dest_x * exportingchannel;
		seek_result = fseek(f,seek,SEEK_SET);

		fwrite(linebuf,1,copy_x_to_count*3,f);
	}
	free(linebuf);
	free(exportingbitmapline);
	fclose(f);


	UnlockBitmaps(env,bitmap);
	env->ReleaseStringUTFChars(exportingfilename, exportingfilename_chars);
	return (jboolean)true;
}
extern "C" JNIEXPORT
jboolean JNICALL Java_im_whir_buyukce_Buyukce_exportJpegNative(JNIEnv* env,
		jclass clazz, jstring fromfile, jstring filename, jint width,
		jint height) {
	const char *filename_chars = env->GetStringUTFChars(filename, 0);
	const char *fromfile_chars = env->GetStringUTFChars(fromfile, 0);

	params comp_params = params();
	comp_params.m_quality = 100;
	int num_channels = 3;
	int size = width * height;

	FILE* fromfile_fd = fopen(fromfile_chars, "r");
	int pBufSize = num_channels * width;
	uint8* pBuf = (uint8*) malloc(pBufSize);

	bigfile_stream dst_stream;
	if (!dst_stream.open(filename_chars))
		return (jboolean) false;

	jpge::jpeg_encoder dst_image;
	if (!dst_image.init(&dst_stream, width, height, num_channels, comp_params))
		return (jboolean) false;

	for (uint pass_index = 0; pass_index < dst_image.get_total_passes();
			pass_index++) {
		for (int i = 0; i < height; i++) {
			fread(pBuf, sizeof(uint8), pBufSize, fromfile_fd);
			if (!dst_image.process_scanline(pBuf))
				return (jboolean) false;
		}
		if (!dst_image.process_scanline(NULL))
			return (jboolean) false;
	}

	dst_image.deinit();
	env->ReleaseStringUTFChars(filename, filename_chars);
	env->ReleaseStringUTFChars(fromfile, fromfile_chars);
	fclose(fromfile_fd);
	return (jboolean) dst_stream.close();

}

package im.whir.buyukce;

import java.io.File;
import java.io.RandomAccessFile;
import java.nio.Buffer;
import java.text.SimpleDateFormat;
import java.util.Date;

import javax.xml.transform.Templates;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Rect;
import android.os.Build;
import android.view.View;

public class Buyukce {
	static {
		try {
			System.loadLibrary("buyukce");
			lib_did_not_initialized = false;
		} catch (Exception e){
			lib_did_not_initialized = true;
		}
	}
	private static native boolean initNative(String filename, int width, int height,int channel);
	private static native boolean exportJpegNative(String from, String to, int width, int height);
	private static native boolean drawBitmapNative(
				String exportingbitmap,int exportingbitmapwidth,int exportingbitmapheight,int exportingbitmapchannel,
				Bitmap bitmap, int dest_x,int dest_y,int dest_w,int dest_h,int direction);
	
	public static enum Format {
		JPEG
	}
	public static enum Direction {
		ANGLE0,
		ANGLE90,
		ANGLE180,
		ANGLE270,
	}
	public final int width;
	public final int height;
	public final String file;
	private int channel;
	private boolean garbage;
	private boolean not_initialized;
	private static boolean lib_did_not_initialized;
	private Buyukce(String file,int width,int height){
		this.width = width;
		this.height = height;
		this.file = file;
		channel = 3;
		garbage = false;
		not_initialized = !initNative(file, width, height, channel);
	}
	
	public Buyukce drawView(View view,int x,int y,Direction direction){
		Rect to = new Rect(x, y, x+view.getWidth(), y+view.getHeight());
		drawView(view,to,direction);
		return this;
	}
	public Buyukce drawView(View view,int x,int y){
		drawView(view, x,y,Direction.ANGLE0);
		return this;
	}
	public Buyukce drawView(View view){
		drawView(view, 0,0);
		return this;
	}
	public Buyukce drawView(View view,Rect to){
		drawView(view, to, Direction.ANGLE0);
		return this;
	}
	public Buyukce drawView(View view,Rect to,Direction direction){
		checkErrors();
		
		Bitmap bitmap = getViewBitmap(view, to.right-to.left, to.bottom-to.top);
		drawBitmapNative(file,width,height,channel,bitmap,to.left,to.top,to.right - to.left,to.bottom-to.top,directionToInt(direction));
		bitmap.recycle();
		return this;
	}
	Bitmap getViewBitmap(View view,int width,int height)
	{

	    int measuredWidth = View.MeasureSpec.makeMeasureSpec(width, View.MeasureSpec.EXACTLY);
	    int measuredHeight = View.MeasureSpec.makeMeasureSpec(height, View.MeasureSpec.EXACTLY);

	    //Cause the view to re-layout
	    view.measure(measuredWidth, measuredHeight);
	    view.layout(0, 0, view.getMeasuredWidth(), view.getMeasuredHeight());

	    //Create a bitmap backed Canvas to draw the view into
	    Bitmap b = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
	    Canvas c = new Canvas(b);

	    //Now that the view is laid out and we have a canvas, ask the view to draw itself into the canvas
	    view.draw(c);

	    return b;
	}
	
	public Buyukce drawBitmap(Bitmap bitmap,int x,int y,Direction direction){
		Rect to = new Rect(x, y, x+bitmap.getWidth(), y+bitmap.getHeight());
		drawBitmap(bitmap, to,false,direction);
		return this;
	}
	public Buyukce drawBitmap(Bitmap bitmap,int x,int y){
		drawBitmap(bitmap, x,y,Direction.ANGLE0);
		return this;
	}
	public Buyukce drawBitmap(Bitmap bitmap){
		drawBitmap(bitmap, 0,0);
		return this;
	}
	public Buyukce drawBitmap(Bitmap bitmap,Rect to){
		drawBitmap(bitmap, to, false);
		return this;
	}
	public Buyukce drawBitmap(Bitmap bitmap,Rect to,boolean scaleToFit){
		drawBitmap(bitmap, to, scaleToFit,Direction.ANGLE0);
		return this;
	}
	public Buyukce drawBitmap(Bitmap bitmap,Rect to,boolean scaleToFit,Direction direction){
		checkErrors();
		if (scaleToFit){
			int destWidth = to.right - to.left;
			int destHeight= to.bottom - to.top;
			Bitmap newBitmap = Bitmap.createScaledBitmap(bitmap, destWidth, destHeight, true);
			drawBitmapNative(file,width,height,channel,newBitmap,to.left,to.top,to.right - to.left,to.bottom-to.top,directionToInt(direction));
			newBitmap.recycle();
		} else {
			drawBitmapNative(file,width,height,channel,bitmap,to.left,to.top,to.right - to.left,to.bottom-to.top,directionToInt(direction));
		}
		return this;
	}
	private int directionToInt(Direction direction){
		if (direction == Direction.ANGLE90)
			return 1;
		if (direction == Direction.ANGLE180)
			return 2;
		if (direction == Direction.ANGLE270)
			return 3;
		else
			return 0;
	}
	public boolean export(String export_file, Format format, double quality){
		boolean result = exportJpegNative(file,export_file, width, height);
		garbage = true;
		new File(file).delete();
		return result;
	}
	private void checkErrors(){
		String msg = "";
		if (garbage){
			msg += "This object is exported. Until now, you can not draw anythin on it";
		} else if (not_initialized){
			if (msg.length() > 0)
				msg += ", ";
			msg += "This object could not be initialized";
		} else if (lib_did_not_initialized){
			if (msg.length() > 0)
				msg += ", ";
			msg += "JNI Library was not loaded";
		}
		if (msg.length() > 0)
			throw new BigBitmapException(msg);
	}
	
	public static Buyukce createBigBitmap(Context ctx, int width,int height){
		ctx.getCacheDir().mkdirs();
		return createBigBitmap(ctx,ctx.getCacheDir().getAbsolutePath(), width, height);
	}
	public static Buyukce createBigBitmap(Context ctx, String temp_folder, int width,int height){
		String file = getNewTempFile(temp_folder);
		Buyukce big = new Buyukce(file,width,height);
		return big;
	}
	
	private static String getNewTempFile(String tmp_path) {
		if (!tmp_path.endsWith("/"))
			tmp_path += "/";
		tmp_path += "BigBitmap/";
		new File(tmp_path).mkdirs();
		SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH-mm-ss.SSS");
		String filename = tmp_path + "BigBitmap-"
				+ format.format(new Date()) + ".tmp";
		return filename;
	}
	
	
	public static class BigBitmapException extends RuntimeException {
		private BigBitmapException(String msg){
			super(msg);
		}
	}

}

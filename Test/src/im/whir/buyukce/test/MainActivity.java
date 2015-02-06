package im.whir.buyukce.test;

import im.whir.buyukce.Buyukce;
import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Rect;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;

public class MainActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		Bitmap bitmap = BitmapFactory.decodeResource(getResources(), R.drawable.ic_launcher);
		
		String exportFile = "/mnt/extSdCard/aaa.jpeg";
		boolean result = Buyukce.createBigBitmap(this,"/mnt/extSdCard/", 200, 200)
		.drawBitmap(bitmap,new Rect(40, 100, 200, 200),true)
		.export(exportFile, Buyukce.Format.JPEG, 1);
		
	}

}

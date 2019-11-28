package org.mtbo.opencvfastft2

import android.graphics.Bitmap
import android.os.Bundle
import android.util.Log
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_main.*
import kotlinx.coroutines.*
import java.io.File
import java.lang.Exception


class MainActivity : AppCompatActivity()  {
	
	private val font_size = 24;
	private val distance_min = font_size * 3 / 2;
	private val distance_max = distance_min + font_size;
	
	private lateinit var font_path: String
	private var background_update_job: Job? = null

	
	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)
		
		setContentView(R.layout.activity_main)
		
		runBlocking {
			backgroundUpdate()
		}
	}
	
	suspend fun backgroundUpdate() = withContext(Dispatchers.Default) {
		
		font_path = copyFont()
		
		background_update_job = GlobalScope.launch {
			try {
				while (true) {
					update()
				}
			}
			catch (e: Exception) {
				Log.e("XXX", "backgroundUpdate exception: ${Thread.currentThread().name}", e)
			}
		}
	}
	
	suspend fun createImage(width: Int, height: Int) = withContext(Dispatchers.IO) {
		
		val bmp = bitmapFromJNI("HELLO WORLD VERY LONG STRING MUST BE WRAPPED INTO MULTIPLE LINES " +
			"HELLO WORLD VERY LONG STRING MUST BE WRAPPED INTO MULTIPLE LINES " +
			"HELLO WORLD VERY LONG STRING MUST BE WRAPPED INTO MULTIPLE LINES " +
			"HELLO WORLD VERY LONG STRING MUST BE WRAPPED INTO MULTIPLE LINES " +
			"HELLO WORLD VERY LONG STRING MUST BE WRAPPED INTO MULTIPLE LINES " +
			"HELLO WORLD VERY LONG STRING MUST BE WRAPPED INTO MULTIPLE LINES " +
			"HELLO WORLD VERY LONG STRING MUST BE WRAPPED INTO MULTIPLE LINES " +
			"HELLO WORLD VERY LONG STRING MUST BE WRAPPED INTO MULTIPLE LINES " +
			"HELLO WORLD VERY LONG STRING MUST BE WRAPPED INTO MULTIPLE LINES " +
			"HELLO WORLD VERY LONG STRING MUST BE WRAPPED INTO MULTIPLE LINES " +
			"HELLO WORLD VERY LONG STRING MUST BE WRAPPED INTO MULTIPLE LINES " +
			"HELLO WORLD VERY LONG STRING MUST BE WRAPPED INTO MULTIPLE LINES " +
			"HELLO WORLD VERY LONG STRING MUST BE WRAPPED INTO MULTIPLE LINES",
			font_path, getFontSize(), getVerticalDistance(), width, height)
		
		bmp
	}
	
	private fun getFontSize(): Int {
		return font_size
	}
	
	private fun getVerticalDistance(): Int {
		return (distance_min.toDouble() + ((distance_max.toDouble() - distance_min.toDouble()) / 2.0 * (1 + Math.sin(Math.PI * (System.currentTimeMillis()).toDouble() / 1000)))).toInt()
	}
	
	suspend fun update() = withContext(Dispatchers.Default) {
		
		val image_size = withContext(Dispatchers.Main) { image_view.getSize() }
		
		var updateJob : Job? = null
		
		if (0 < image_size.width && 0 < image_size.height) {
			val bitmap = createImage(image_size.width, image_size.height)
			
			updateJob = launch(Dispatchers.Main) {
				image_view.setImageBitmap(bitmap)
			}
		}
		
		delay(20L)
		
		updateJob?.join()
	}
	
	
	
	override fun onDestroy() {
		cancelBackgroundUpdate()
		super.onDestroy()
	}
	
	
	private fun cancelBackgroundUpdate() = runBlocking(Dispatchers.Default) {
		background_update_job?.cancelAndJoin()
		background_update_job = null
	}
	
	
	suspend private fun copyFont(): String  = withContext(Dispatchers.IO) {
		
		val font_name = "font.ttf"
		val ins = assets.open(font_name)
		val file = File(filesDir, font_name)
		
		val it = file.outputStream()
		val buffer = ByteArray(1024)
		var read = ins.read(buffer);
		
		while (read != -1) {
			it.write(buffer, 0, read)
			read = ins.read(buffer)
		}
		
		file.absolutePath
	}
	
	
	external fun bitmapFromJNI(
		text: String,
		font_path: String,
		font_size: Int,
		vertical_interval: Int,
		width: Int,
		height: Int
	): Bitmap
	
	
	companion object {
		// Used to load the 'native-lib' library on application startup.
		init {
			System.loadLibrary("native-lib")
		}
	}
	
	fun exit(view: View) {
		finish()
	}
}

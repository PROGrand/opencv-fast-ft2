package org.mtbo.opencvfastft2

import android.content.Context
import android.util.AttributeSet
import android.util.Size
import android.webkit.WebView

class MatView @JvmOverloads constructor(
	context: Context, attrs: AttributeSet? = null, defStyleAttr: Int = 0
) : androidx.appcompat.widget.AppCompatImageView(context, attrs, defStyleAttr) {
	
	private var w : Int = 0
	private var h : Int = 0
	
	override fun onLayout(changed: Boolean, left: Int, top: Int, right: Int, bottom: Int) {
		super.onLayout(changed, left, top, right, bottom)
		w = right - left
		h = bottom - top
		
		var v : WebView
	}

	fun getSize() : Size {
		return Size(w, h)
	}
}


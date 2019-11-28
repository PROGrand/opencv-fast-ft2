#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <unistd.h>
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftcache.h>
#include <hb-ft.h>

using cv::String;
using cv::InputOutputArray;
using cv::Point;
using cv::Scalar;
using cv::Size;
using cv::Mat;
using namespace std;

class FreeType2Impl {
public:
	FreeType2Impl();

	~FreeType2Impl();

	void loadFontData(String fontFileName, int id);

	void putText(InputOutputArray img, const String &text, Point org,
	             unsigned int fontHeight, Scalar color);

	Size getTextSize(const String &text, unsigned int fontHeight,
	                 int *baseLine
	);

private:
	FT_Library mLibrary;
	FTC_Manager mManager;

	FTC_ImageCache mCache;
	FT_Face mFace;

	bool mIsFaceAvailable;

	hb_font_t *mHb_font;

	void putTextBitmapBlend(
		InputOutputArray img, const String &text, Point org,
		int fontHeight, Scalar color
	);

	static FT_Error Face_Requester(FTC_FaceID face_id,
	                               FT_Library library,
	                               FT_Pointer req_data,
	                               FT_Face *aface) {
		FreeType2Impl *impl = static_cast<FreeType2Impl *>(req_data);
		*aface = impl->mFace;
		return 0;
	}

};


FreeType2Impl::FreeType2Impl() {
	FT_Init_FreeType(&(this->mLibrary));

	FTC_Manager_New(this->mLibrary,
	                0,
	                0,
	                0,
	                Face_Requester,
	                this,
	                &(this->mManager));

	FTC_ImageCache_New(this->mManager,
	                   &(this->mCache));

	mIsFaceAvailable = false;
}

FreeType2Impl::~FreeType2Impl() {
	if (mIsFaceAvailable) {
		hb_font_destroy(mHb_font);
		CV_Assert(!FT_Done_Face(mFace));
		mIsFaceAvailable = false;
	}

	FTC_Manager_Done(mManager);
	CV_Assert(!FT_Done_FreeType(mLibrary));
}

void FreeType2Impl::loadFontData(String fontFileName, int idx) {
	if (mIsFaceAvailable) {
		hb_font_destroy(mHb_font);
		CV_Assert(!FT_Done_Face(mFace));
	}
	CV_Assert(!FT_New_Face(mLibrary, fontFileName.c_str(), idx, &(mFace)));
	mHb_font = hb_ft_font_create(mFace, nullptr);
	CV_Assert(mHb_font != NULL);
	mIsFaceAvailable = true;
}

void FreeType2Impl::putText(
	InputOutputArray _img, const String &_text, Point _org,
	unsigned int _fontHeight, Scalar _color) {
	CV_Assert(mIsFaceAvailable);
	CV_Assert(!_img.empty() &&
	          _img.isMat() &&
	          (_img.depth() == CV_8U) &&
	          (_img.dims() == 2) &&
	          (_img.channels() == 4));

	CV_Assert(_fontHeight >= 0);

	if (_text.empty()) {
		return;
	}

	if (_fontHeight == 0) {
		return;
	}

	CV_Assert(!FT_Set_Pixel_Sizes(mFace, _fontHeight, _fontHeight));

	putTextBitmapBlend(_img, _text, _org, _fontHeight, _color);
}

void FreeType2Impl::putTextBitmapBlend(
	InputOutputArray _img, const String &_text, Point _org,
	int _fontHeight, Scalar _color) {

	Mat dst = _img.getMat();
	hb_buffer_t *hb_buffer = hb_buffer_create();
	CV_Assert(hb_buffer != NULL);

	unsigned int textLen;
	hb_buffer_guess_segment_properties(hb_buffer);
	hb_buffer_add_utf8(hb_buffer, _text.c_str(), -1, 0, -1);
	hb_glyph_info_t *info =
		hb_buffer_get_glyph_infos(hb_buffer, &textLen);
	CV_Assert(info != NULL);

	hb_shape(mHb_font, hb_buffer, NULL, 0);

	_org.y += _fontHeight;

	FTC_ImageTypeRec  font_type;
	font_type.face_id = (FTC_FaceID) 1;
	font_type.width   = (short) _fontHeight;
	font_type.height  = (short) _fontHeight;
	font_type.flags   = FT_LOAD_RENDER;


	for (unsigned int i = 0; i < textLen; i++) {
		FT_Glyph glyph;
		FTC_ImageCache_Lookup(mCache, &font_type, info[i].codepoint, &glyph, nullptr);

		if (glyph->format != FT_GLYPH_FORMAT_BITMAP) {
			FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 0);
		}

		FT_BitmapGlyph glyph_bitmap = (FT_BitmapGlyph)glyph;

		FT_Bitmap *bmp = &(glyph_bitmap->bitmap);

		Point gPos = _org;
		gPos.y -= glyph_bitmap->top;
		gPos.x += glyph_bitmap->left;

		for (int row = 0; row < (int) bmp->rows; row++) {
			if (gPos.y + row < 0) {
				continue;
			}
			if (gPos.y + row >= dst.rows) {
				break;
			}

			for (int col = 0; col < bmp->pitch; col++) {
				int cl = bmp->buffer[row * bmp->pitch + col];
				if (cl == 0) {
					continue;
				}
				if (gPos.x + col < 0) {
					continue;
				}
				if (gPos.x + col >= dst.cols) {
					break;
				}

				auto &ptr = dst.at<cv::Vec4b>(gPos.y + row, gPos.x + col);

				//ptr[0] = static_cast<uchar>(_color[0]);
				//ptr[1] = static_cast<uchar>(_color[1]);
				//ptr[2] = static_cast<uchar>(_color[2]);
				ptr[3] = static_cast<uchar>((double) cl);
			}
		}
		_org.x += (glyph->advance.x) >> 16;
		_org.y += (glyph->advance.y) >> 16;
	}
	hb_buffer_destroy(hb_buffer);
}


Size FreeType2Impl::getTextSize(
	const String &_text,
	unsigned int _fontHeight,
	CV_OUT int *_baseLine) {
	if (_text.empty()) {
		return Size(0, 0);
	}

	CV_Assert(_fontHeight >= 0);
	if (_fontHeight == 0) {
		return Size(0, 0);
	}

	CV_Assert(!FT_Set_Pixel_Sizes(mFace, _fontHeight, _fontHeight));

	hb_buffer_t *hb_buffer = hb_buffer_create();
	CV_Assert(hb_buffer != NULL);
	Point _org(0, 0);

	unsigned int textLen;
	hb_buffer_guess_segment_properties(hb_buffer);
	hb_buffer_add_utf8(hb_buffer, _text.c_str(), -1, 0, -1);
	hb_glyph_info_t *info =
		hb_buffer_get_glyph_infos(hb_buffer, &textLen);
	CV_Assert(info != NULL);
	hb_shape(mHb_font, hb_buffer, NULL, 0);

	FTC_ImageTypeRec  font_type;
	font_type.face_id = (FTC_FaceID) 1;
	font_type.width   = (short) _fontHeight;
	font_type.height  = (short) _fontHeight;
	font_type.flags   = FT_LOAD_RENDER;

	int yMin = INT_MAX, yMax = INT_MIN, xMin = INT_MAX, xMax = INT_MIN;

	for (unsigned int i = 0; i < textLen; i++) {
		FT_Glyph glyph;
		FTC_ImageCache_Lookup(mCache, &font_type, info[i].codepoint, &glyph, nullptr);

		if (glyph->format != FT_GLYPH_FORMAT_BITMAP) {
			FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 0);
		}

		FT_BitmapGlyph glyph_bitmap = (FT_BitmapGlyph)glyph;

		FT_Bitmap *bmp = &(glyph_bitmap->bitmap);

		Point gPos = _org;

		gPos.y -= glyph_bitmap->top;
		gPos.x += glyph_bitmap->left;
		int rows = bmp->rows;
		int cols = bmp->pitch;

		yMin = cv::min(yMin, gPos.y);
		yMax = cv::max(yMax, gPos.y + rows);
		xMin = cv::min(xMin, gPos.x);
		xMax = cv::max(xMax, gPos.x + cols);

		_org.x += (glyph->advance.x) >> 16;
		_org.y += (glyph->advance.y) >> 16;
	}

	hb_buffer_destroy(hb_buffer);

	int width = xMax - xMin;
	int height = yMax - yMin;

	width = cvRound(width + 1);
	height = cvRound(height + 1);

	if (_baseLine) {
		*_baseLine = yMin;
	}

	return Size(width, height);
}

//////////////////////////////

static std::vector<std::string> split(const char *phrase, std::string delimiter) {
	vector<string> list;
	string s = string(phrase);
	size_t pos = 0;
	string token;
	while ((pos = s.find(delimiter)) != string::npos) {
		token = s.substr(0, pos);
		list.push_back(token);
		s.erase(0, pos + delimiter.length());
	}
	list.push_back(s);
	return list;
}


cv::Ptr<FreeType2Impl> ft2;

extern "C"
JNIEXPORT jobject JNICALL
Java_org_mtbo_opencvfastft2_MainActivity_bitmapFromJNI(JNIEnv *env, jobject thiz,
                                                     jstring input_string, jstring font_path,
                                                     jint font_size, jint vertical_interval,
                                                     jint width, jint height) {

	JavaVM *vm;
	env->GetJavaVM(&vm);
	vm->AttachCurrentThread(&env, nullptr);

	//__android_log_print(ANDROID_LOG_INFO, "XXX", "make bitmap...");


	jclass bitmapConfig = env->FindClass("android/graphics/Bitmap$Config");
	jfieldID argbFieldID = env->GetStaticFieldID(bitmapConfig, "ARGB_8888",
	                                             "Landroid/graphics/Bitmap$Config;");
	jobject argbObj = env->GetStaticObjectField(bitmapConfig, argbFieldID);

	jclass bitmapClass = env->FindClass("android/graphics/Bitmap");
	jmethodID createBitmapMethodID = env->GetStaticMethodID(bitmapClass, "createBitmap",
	                                                        "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");

	cv::Size bitmap_size(width, height);

	jobject bitmapObj = env->CallStaticObjectMethod(bitmapClass, createBitmapMethodID,
	                                                bitmap_size.width, bitmap_size.height, argbObj);

	void *addr = nullptr;
	AndroidBitmap_lockPixels(env, bitmapObj, &addr);

	cv::Mat argb(bitmap_size, CV_8UC4, addr);

	argb = Scalar(255, 0, 0, 0);

	////////////////////////////////////////////////////////////

	if (!ft2) {
		//__android_log_print(ANDROID_LOG_INFO, "XXX", "create ft2...");

		ft2 = new FreeType2Impl();

		const char *str = env->GetStringUTFChars(font_path, 0);

		//__android_log_print(ANDROID_LOG_INFO, "XXX", "load font data: %s", str);
		ft2->loadFontData(str, 0);
		env->ReleaseStringUTFChars(font_path, str);
	}

	////////////////////////////////////////////////////////////////

	std::string current_string = "";

	auto dy = 0;
	bool need = false;

	//__android_log_print(ANDROID_LOG_INFO, "XXX", "put text...");

	const char *_text = env->GetStringUTFChars(input_string, 0);
	auto words = split(_text, " ");
	env->ReleaseStringUTFChars(input_string, _text);

	for (const auto &word : words) {
		int baseLine = 0;
		auto attempt = current_string.size() ? current_string + " " + word : word;

		cv::Size text_size = ft2->getTextSize(attempt, font_size, &baseLine);

		if (text_size.width > argb.cols) {
			//u8"سلام"
			ft2->putText(argb, current_string, cv::Point(0, dy),
			             font_size, cv::Scalar(0, 0, 0));

			current_string = word;
			dy += vertical_interval;
			need = false;
		} else {
			current_string = attempt;
			need = true;
		}
	}

	if (need) {
		ft2->putText(argb, current_string, cv::Point(0, dy),
		             font_size, cv::Scalar(0, 0, 0));
	}

	//////////////////////////////////////

	AndroidBitmap_unlockPixels(env, bitmapObj);

	//__android_log_print(ANDROID_LOG_INFO, "XXX", "return bitmap...");

	return bitmapObj;
}
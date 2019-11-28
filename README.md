# opencv-fast-ft2
Using Freetype2 font rendering in OpenCV on Android

Font rendering using Freetype2(+ Cache) directly, instead of (unsupported?) opencv-freetype module.
Freetype2Impl renders text into cv::Mat of type CV_8UC4 (RGBA). Alpha channel guided by font.
Example of converting of cv:Mat to Android Bitmap is provided. Kotlin Coroutines are used intensively.

## Requirements:
* opencv
* freetype2
* harfbuzz

I use CMake, NDK 21, MinGW, Android Studio 4.0 Canary 4.

## Performance

Screen rendering on Samsung Galaxy S7 - about 20 FPS. Same functionality achieved using opencv-freetype module performs is below 3 FPS.

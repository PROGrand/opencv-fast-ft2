# opencv-fast-ft2
Using Freetype2 text rendering in OpenCV on Android

Text rendering using Freetype2(+ Cache) directly, instead of (unsupported?) opencv-freetype module.
Freetype2Impl renders text into cv::Mat of type CV_8UC4 (RGBA). Alpha channel guided by font.
Example of simple text wrapping and converting of cv:Mat to Android Bitmap is provided. Kotlin Coroutines are used intensively.
For time sensitive tasks.

## Requirements:
* opencv
* freetype2
* harfbuzz

I use CMake, NDK 21, MinGW, Android Studio 4.0 Canary 4.

## Performance

Screen rendering on Samsung Galaxy S7 - about 20 FPS. Same functionality achieved using opencv-freetype module performs is below 3 FPS.

## TODO
* Move ft2 instance from static variable to java object field.
* Smarter text wrapping (caching word sizes, estimating line characters count, justifying, etc.)

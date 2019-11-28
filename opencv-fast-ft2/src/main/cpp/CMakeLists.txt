cmake_minimum_required(VERSION 3.4.1)

set(OpenCV_STATIC ON)
set(OpenCV_DIR "F:/work/android/opencv/arm-v7a/sdk/native/jni")

SET(CMAKE_CXX_FLAGS "-fopenmp ${CMAKE_CXX_FLAGS}")

find_package(OpenCV REQUIRED)

set(FREETYPE_LIBRARY "F:/work/android/freetype-2.10.0/arm-v7a/lib/libfreetype.a")
set(FREETYPE_INCLUDE_DIRS "F:/work/android/freetype-2.10.0/arm-v7a/include/freetype2")
set(HARFBUZZ_LIBRARIES "F:/work/android/harfbuzz-2.6.4/arm-v7a/lib/libharfbuzz.a")
set(HARFBUZZ_INCLUDE_DIRS "F:/work/android/harfbuzz-2.6.4/arm-v7a/include/harfbuzz")

find_package(Freetype REQUIRED)

find_package(OpenMP REQUIRED)


include_directories(${OpenCV_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS} ${HARFBUZZ_INCLUDE_DIRS})

link_directories(${OpenCV_LIB_DIR})

add_library(
        native-lib
        SHARED
        native-lib.cpp)

find_library(
        log-lib
        log)

find_library(
        android-lib
        android)

find_library(
        jnigraphics-lib
        jnigraphics)


find_library(HARFBUZZ_LIBRARIES
        NAMES harfbuzz)


# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

message("opencv libs: ${OpenCV_LIBS}")

target_link_libraries( # Specifies the target library.
        native-lib

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        ${android-lib}
        ${jnigraphics-lib}
        ${HARFBUZZ_LIBRARIES}
        ${FREETYPE_LIBRARIES}
        ${OpenCV_LIBS}
        ${OpenMP_CXX_LIBRARIES}
        )
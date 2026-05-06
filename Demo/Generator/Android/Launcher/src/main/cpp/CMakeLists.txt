cmake_minimum_required(VERSION 3.10)

project("EgoLauncherAndroidNative")

add_subdirectory(${CMAKE_SOURCE_DIR}/../../../../../../ ego)

set(GLUE_PATH ${ANDROID_NDK}/sources/android/native_app_glue)

add_library(EgoLauncherAndroidNative SHARED AndroidMain.cpp ${GLUE_PATH}/android_native_app_glue.c)

target_include_directories(EgoLauncherAndroidNative PUBLIC ${GLUE_PATH})

target_link_libraries(EgoLauncherAndroidNative
    PUBLIC
        EgoLauncherAndroid
        android
        log
)

target_link_options(EgoLauncherAndroidNative PRIVATE
    "-uANativeActivity_onCreate"
)
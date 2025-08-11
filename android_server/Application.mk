APP_ABI := armeabi-v7a arm64-v8a x86 x86_64
APP_PLATFORM := android-29
APP_STL := c++_shared
APP_CPPFLAGS := -std=c++20 -fexceptions -frtti -DANDROID -D__ANDROID__
APP_OPTIM := debug
APP_STRIP_MODE := --strip-unneeded 
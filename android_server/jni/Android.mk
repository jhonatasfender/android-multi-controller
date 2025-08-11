LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := android_server
LOCAL_SRC_FILES := \
    main.cpp \
    server.cpp \
    screen_capture/media_projection_capture.cpp \
    video_encoder/mediacodec_h264_encoder.cpp \
    network/tcp_server.cpp \
    protocol/protocol_handler.cpp

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH) \
    $(LOCAL_PATH)/common \
    $(LOCAL_PATH)/protocol \
    $(LOCAL_PATH)/video_encoder \
    $(LOCAL_PATH)/screen_capture \
    $(LOCAL_PATH)/network \
    $(LOCAL_PATH)/audio_capture

LOCAL_CPPFLAGS := -std=c++20 -Wall -Wextra -O2 -DANDROID -D__ANDROID__
LOCAL_LDLIBS := -llog -landroid -lmediandk -lEGL -lGLESv2

include $(BUILD_EXECUTABLE) 
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := buyukce
LOCAL_SRC_FILES := BuyukceBridge.cpp jpge.cpp jpgd.cpp stb_image.c
LOCAL_LDLIBS :=  -llog 
LOCAL_LDFLAGS += -ljnigraphics

include $(BUILD_SHARED_LIBRARY)

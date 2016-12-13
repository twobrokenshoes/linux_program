LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_SRC_FILES := \
	epoll_test.c
LOCAL_MODULE:= epoll_test
LOCAL_MODULE_TAGS := eng
LOCAL_SHARED_LIBRARIES := libc libcutils
include $(BUILD_EXECUTABLE)

LOCAL_PATH := $(call my-dir)
ROOT := $(LOCAL_PATH)/../
include $(ROOT)/app.mk
include $(ROOT)/common.mk

CFLAGS = -fms-extensions -Wno-microsoft-anon-tag -Wno-unused-command-line-argument
CFLAGS += -DCOMPILE_COMPAT
CFLAGS += -DAPP_NAME="\"$(APP_NAME)\"" -DAPP_VERSION="\"$(APP_VERSION)\""
CFLAGS += -DAPP_MAINTAINER="\"$(APP_MAINTAINER)\"" -DAPP_DESC="\"$(APP_DESC)\""

include $(CLEAR_VARS)
LOCAL_MODULE := libautodiag
LOCAL_C_INCLUDES := $(ROOT)/include/main/ $(ROOT)/cJSON/
LOCAL_SRC_FILES := ../cJSON/cJSON.c $(patsubst $(LOCAL_PATH)/%,%,$(call rwildcard,$(ROOT)/src/main/libautodiag/,*.c))
LOCAL_CFLAGS := $(CFLAGS)
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := elm327sim
LOCAL_C_INCLUDES := $(ROOT)/include/main/
LOCAL_CFLAGS := $(CFLAGS)
include $(BUILD_SHARED_LIBRARY)
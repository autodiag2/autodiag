LOCAL_PATH := $(call my-dir)
ROOT := $(LOCAL_PATH)/../

include $(ROOT)/app.mk
include $(CLEAR_VARS)

rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2)) $(wildcard $1$2)

LOCAL_MODULE := libautodiag
LOCAL_C_INCLUDES := $(ROOT)/include/main $(ROOT)/cJSON/
SRCDIR := $(ROOT)/src/main/libautodiag/
LOCAL_SRC_FILES := ../cJSON/cJSON.c $(patsubst $(LOCAL_PATH)/%,%,$(call rwildcard,$(SRCDIR),*.c))

LOCAL_CFLAGS += -fms-extensions -Wno-microsoft-anon-tag -Wno-unused-command-line-argument
LOCAL_CFLAGS += -DCOMPILE_COMPAT
LOCAL_CFLAGS += -DAPP_NAME="\"$(APP_NAME)\"" -DAPP_VERSION="\"$(APP_VERSION)\""
LOCAL_CFLAGS += -DAPP_MAINTAINER="\"$(APP_MAINTAINER)\"" -DAPP_DESC="\"$(APP_DESC)\""

include $(BUILD_SHARED_LIBRARY)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2)) $(wildcard $1$2)

ROOT := $(LOCAL_PATH)/../
LOCAL_MODULE := libautodiag
LOCAL_C_INCLUDES := $(ROOT)/include/main $(ROOT)/cJSON/
SRCDIR := $(ROOT)/src/main/libautodiag/
LOCAL_SRC_FILES := $(patsubst $(LOCAL_PATH)/%,%,$(call rwildcard,$(SRCDIR),*.c))

LOCAL_CFLAGS += -fms-extensions -Wno-microsoft-anon-tag -Wno-unused-command-line-argument
LOCAL_CFLAGS += -DCOMPILE_COMPAT

include $(BUILD_SHARED_LIBRARY)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2)) $(wildcard $1$2)

ROOT := $(LOCAL_PATH)/../
LOCAL_MODULE := libautodiag
LOCAL_C_INCLUDES := $(ROOT)/include/main $(ROOT)/cJSON/
SRCDIR := $(ROOT)/src/main/libautodiag/
LOCAL_SRC_FILES := $(patsubst $(LOCAL_PATH)/%,%,$(call rwildcard,$(SRCDIR),*.c))

$(info LOCAL_SRC_FILES = $(LOCAL_SRC_FILES))
$(info LOCAL_PATH = $(LOCAL_PATH))
$(info LOCAL_C_INCLUDES = $(LOCAL_C_INCLUDES))

LOCAL_CFLAGS := -DCOMPILE_COMPAT

include $(BUILD_SHARED_LIBRARY)

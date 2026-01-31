LOCAL_PATH := $(call my-dir)
ROOT := $(LOCAL_PATH)/../
include $(ROOT)/app.mk
include $(ROOT)/common.mk

CFLAGS = -fms-extensions -Wno-microsoft-anon-tag -Wno-unused-command-line-argument
CFLAGS += -DCOMPILE_COMPAT
CFLAGS += -DAPP_NAME="\"$(APP_NAME)\"" -DAPP_VERSION="\"$(APP_VERSION)\""
CFLAGS += -DAPP_MAINTAINER="\"$(APP_MAINTAINER)\"" -DAPP_DESC="\"$(APP_DESC)\""
CFLAGS += -ffile-prefix-map=$(ROOT)=. -fdebug-prefix-map=$(ROOT)=.

include $(CLEAR_VARS)
LOCAL_MODULE := libautodiag
LOCAL_C_INCLUDES := $(ROOT)/include/main/ $(ROOT)/cJSON/
LOCAL_SRC_FILES := $(sort \
  ../cJSON/cJSON.c \
  $(patsubst $(LOCAL_PATH)/%,%,$(call rwildcard,$(ROOT)/src/main/libautodiag/,*.c)) \
)
LOCAL_CFLAGS := $(CFLAGS)
LOCAL_LDLIBS += -llog
LOCAL_LDFLAGS += -Wl,--build-id=none
LOCAL_CFLAGS += -fno-profile-generate -fno-profile-use
LOCAL_CFLAGS += -fno-bolt
LOCAL_CFLAGS += -fno-mlgo
LOCAL_CFLAGS += -fno-lto
LOCAL_LDFLAGS += -fno-lto
LOCAL_CFLAGS += \
  -fno-ident \
  -fno-record-gcc-switches
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := elm327sim
LOCAL_C_INCLUDES := $(ROOT)/include/main/ $(ROOT)/cJSON/
SRC_FILES_LIBPROG := $(sort $(call filterout-multi, \
  ..//src/main/libprog/sim_ecu_generator_gui.c \
  ..//src/main/libprog/ui/% \
  ,$(patsubst $(LOCAL_PATH)/%,%,$(call rwildcard,$(ROOT)/src/main/libprog,*.c)) \
))
LOCAL_SRC_FILES := $(sort $(SRC_FILES_LIBPROG) ../src/main/prog/elm327sim.c)
LOCAL_CFLAGS := $(CFLAGS)
LOCAL_SHARED_LIBRARIES := libautodiag
LOCAL_LDLIBS += -llog
LOCAL_LDFLAGS += -Wl,--build-id=
LOCAL_CFLAGS += -fno-profile-generate -fno-profile-use
LOCAL_CFLAGS += -fno-bolt
LOCAL_CFLAGS += -fno-mlgo
LOCAL_CFLAGS += -fno-lto
LOCAL_LDFLAGS += -fno-lto
LOCAL_CFLAGS += \
  -fno-ident \
  -fno-record-gcc-switches
include $(BUILD_EXECUTABLE)
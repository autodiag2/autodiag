### Overridable options with `make -e`

ifdef INSTALL_DATA_FOLDER

else ifdef LOCALAPPDATA
	# Windows local user
	INSTALL_DATA_FOLDER = $(LOCALAPPDATA)
else ifdef APPDATA
	# Windows system wide
	INSTALL_DATA_FOLDER = $(APPDATA)
else ifdef XDG_DATA_DIRS
	# XDG specification
	INSTALL_DATA_FOLDER = $(HOME)/.local/share/
else
	# UNIX
	INSTALL_DATA_FOLDER = /usr/share/
endif
 
ifdef INSTALL_BIN_FOLDER

else ifdef LOCALAPPDATA
    INSTALL_BIN_FOLDER = .
    INSTALL_LIB_FOLDER = .
else ifdef APPDATA
    INSTALL_BIN_FOLDER = .
    INSTALL_LIB_FOLDER = .
else ifdef XDG_DATA_DIRS
    ROOT = $(HOME)/.local/
    INSTALL_BIN_FOLDER = $(ROOT)/bin/
    INSTALL_LIB_FOLDER = $(ROOT)/lib/
else
    ROOT = /usr/
    INSTALL_BIN_FOLDER = $(ROOT)/bin/
    INSTALL_LIB_FOLDER = $(ROOT)/lib/
endif

CFLAGS_DEBUG    = -ggdb
CGLAGS_GUI      = `pkg-config --cflags gtk+-3.0`
CFLAGS          = -I include/main/ -I cJSON/ -fms-extensions -Wno-microsoft-anon-tag -Wno-unused-command-line-argument -fPIC $(CFLAGS_DEBUG)
CFLAGS_LIBS     = -lpthread -lm
CFLAGS_LIBS_GUI = `pkg-config --libs gtk+-3.0`

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)
MACHINE := $(UNAME_M)

ifeq ($(OS),Windows_NT)
    SYSTEM := windows
    EXT := dll
    CFLAGS_LIB_COMPILE := -shared
    CFLAGS_LIBS += -lsetupapi
else ifneq (,$(findstring MINGW64_NT,$(UNAME_S)))
    SYSTEM := windows
    EXT := dll
    CFLAGS_LIB_COMPILE := -shared
    CFLAGS_LIBS += -lsetupapi
else ifneq (,$(findstring x86_64-w64-mingw32-,$(TOOLCHAIN)))
    SYSTEM := windows
    MACHINE := x86_64
    EXT := dll
    CFLAGS_LIB_COMPILE := -shared
    CFLAGS_LIBS += -lsetupapi
else ifneq (,$(findstring i686-w64-mingw32-,$(TOOLCHAIN)))
    SYSTEM := windows
    MACHINE := i686
    EXT := dll
    CFLAGS_LIB_COMPILE := -shared
    CFLAGS_LIBS += -lsetupapi
else ifeq ($(UNAME_S),Darwin)
    SYSTEM := darwin
    EXT := dylib
    CFLAGS_LIB_COMPILE := -dynamiclib
else ifeq ($(UNAME_S), Linux)
    SYSTEM := linux
    EXT := so
    CFLAGS_LIB_COMPILE := -shared
else
    $(error OS unsupported)
endif

BIN_LIB_NAME := libautodiag-$(SYSTEM)-$(MACHINE).$(EXT)

define rwildcard
	$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
endef

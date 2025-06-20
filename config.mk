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
else ifdef APPDATA
    INSTALL_BIN_FOLDER = .
else ifdef XDG_DATA_DIRS
    INSTALL_BIN_FOLDER = $(HOME)/.local/bin/
else
    INSTALL_BIN_FOLDER = /usr/bin/
endif

CFLAGS_DEBUG    = -ggdb
CGLAGS_GUI      = `pkg-config --cflags gtk+-3.0`
CFLAGS          = -I include/main/ -fms-extensions -Wno-microsoft-anon-tag -fPIC $(CFLAGS_DEBUG)
CFLAGS_LIBS     = -lpthread -lm
CFLAGS_LIBS_GUI = `pkg-config --libs gtk+-3.0`

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

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

MACHINE := $(UNAME_M)
BIN_LIB_NAME := libautodiag-$(SYSTEM)-$(MACHINE).$(EXT)

define rwildcard
	$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
endef

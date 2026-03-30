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
    IS_FEDORA := $(shell sh -c '. /etc/os-release 2>/dev/null; [ "$$ID" = "fedora" ] && echo 1 || echo 0')
    ROOT = /usr/
    INSTALL_BIN_FOLDER = $(ROOT)/bin/
    ifeq ($(IS_FEDORA),1)
        INSTALL_LIB_FOLDER = $(ROOT)/lib64/
    else
        INSTALL_LIB_FOLDER = $(ROOT)/lib/
    endif
endif

CFLAGS_DEBUG            = -ggdb
CGLAGS_GUI              = `pkg-config --cflags gtk+-3.0`
CFLAGS                  = -DMG_TLS=MG_TLS_BUILTIN -I dependencies/sqlite3/ -I dependencies/mongoose/ -I include/main/ -I dependencies/cJSON/ -DENABLE_LOCALES=true -fms-extensions -Wno-microsoft-anon-tag -Wno-unused-command-line-argument -fPIC $(CFLAGS_DEBUG)
CFLAGS_LIBS             = -lpthread -lm
CFLAGS_LIBS_GUI         = `pkg-config --libs gtk+-3.0`
COMPILE_NEED_OBJS       = false

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)
MACHINE := $(UNAME_M)

ifeq ($(OS),Windows_NT)
    SYSTEM := windows
    EXT := dll
    CFLAGS_LIB_COMPILE := -shared
    CFLAGS_LIBS += -lsetupapi -ldbghelp
else ifneq (,$(findstring MINGW64_NT,$(UNAME_S)))
    SYSTEM := windows
    EXT := dll
    CFLAGS_LIB_COMPILE := -shared
    CFLAGS_LIBS += -lsetupapi -ldbghelp
else ifneq (,$(findstring x86_64-w64-mingw32-,$(TOOLCHAIN)))
    SYSTEM := windows
    MACHINE := x86_64
    EXT := dll
    CFLAGS_LIB_COMPILE := -shared
    CFLAGS_LIBS += -lsetupapi -lws2_32 -ldbghelp
    COMPILE_NEED_OBJS = true
else ifneq (,$(findstring i686-w64-mingw32-,$(TOOLCHAIN)))
    SYSTEM := windows
    MACHINE := i686
    EXT := dll
    CFLAGS_LIB_COMPILE := -shared
    CFLAGS_LIBS += -lsetupapi -lws2_32 -ldbghelp
    COMPILE_NEED_OBJS = true
else ifeq ($(UNAME_S),Darwin)
    SYSTEM := darwin
    EXT := dylib
    CFLAGS_LIB_COMPILE := -dynamiclib
else ifeq ($(UNAME_S),Linux)
    SYSTEM := linux
    EXT := so
    CFLAGS_LIB_COMPILE := -shared
    CFLAGS += -rdynamic
else
    $(error OS unsupported)
endif

BIN_LIB_NAME := libautodiag-$(SYSTEM)-$(MACHINE).$(EXT)

ifeq ($(COMPILE_VERBOSE),)
    COMPILE_MSG = @echo "$@"
    PRINT_VOIDER = @
else
    COMPILE_MSG = @echo "Compiling ($^) -> $@"
    PRINT_VOIDER = 
endif

include common.mk
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

DEBUG_CFLAGS = -ggdb
CFLAGS = $(DEBUG_CFLAGS) -I include/main/ -fms-extensions `pkg-config --cflags gtk+-3.0`
LIBS = -lpthread -lm `pkg-config --libs gtk+-3.0`

define rwildcard
	$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
endef

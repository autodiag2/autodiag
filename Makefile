include config.mk
include app.mk

INSTALL_DATA_FOLDER_APP = $(INSTALL_DATA_FOLDER)/$(APP_NAME)/

# Programs
SOURCES_PROGS = $(call rwildcard,src/main/,*.c)
OBJS_PROGS = $(filter-out obj/main/libautodiag/%.o,$(filter-out obj/main/prog/%.o,$(subst src/main/,obj/main/,$(SOURCES_PROGS:.c=.o))))
BINS_PROGS = $(patsubst src/main/prog/%.c,bin/%,$(call rwildcard,src/main/prog/,*.c))

# Library shared object
OBJS_LIB = $(filter obj/main/libautodiag/%.o,$(subst src/main/,obj/main/,$(SOURCES_PROGS:.c=.o)))
BIN_LIB := $(BIN_LIB_NAME)
PYTHON_INSTALL_FOLDER_ROOT = pyautodiag/autodiag
PYTHON_INSTALL_FOLDER_LIB = $(PYTHON_INSTALL_FOLDER_ROOT)/libs/
PYTHON_INSTALL_FOLDER_DATA = $(PYTHON_INSTALL_FOLDER_ROOT)/data/

# Tests
SOURCES_TESTS = $(call rwildcard,src/test/,*.c)
OBJS_TESTS = $(subst obj/test/regression.o,,$(subst obj/test/obd_get_pid_supported.o,,$(subst src/test/,obj/test/,$(SOURCES_TESTS:.c=.o))))
BINS_TESTS = bin/regression bin/obd_get_pid_supported

SOURCES = $(SOURCES_PROGS) $(SOURCES_TESTS)
OBJS = $(OBJS_PROGS) $(OBJS_TESTS) $(OBJS_LIB)

CFLAGS_TESTS = -I src/testFixtures/
CFLAGS_COVERAGE = 
CFLAGS_LIBS_TESTS = 

ifeq ($(filter release_progs, $(MAKECMDGOALS)),)
    CFLAGS += $(DEBUG_CFLAGS)
endif

CFLAGS += -DAPP_NAME="\"$(APP_NAME)\"" -DAPP_VERSION="\"$(APP_VERSION)\""
CFLAGS += -DAPP_MAINTAINER="\"$(APP_MAINTAINER)\"" -DAPP_DESC="\"$(APP_DESC)\""
CC = $(TOOLCHAIN)gcc

default: compile_progs

release_progs: compile_progs
	@-$(TOOLCHAIN)strip $(BINS_PROGS)

compile_progs: $(BINS_PROGS)
	@-echo "Software ready at: $^"

compile_tests: $(BINS_TESTS)
	@-echo "Tests: $^"

compile_lib: $(BIN_LIB)
	@-echo "Library ready at: $^"

coverage: CFLAGS_COVERAGE += --coverage
coverage: CFLAGS_LIBS_TESTS += -lgcov
coverage: veryclean compile_tests
	echo "running coverage with : $(CFLAGS)"
	./bin/regression
	$(TOOLCHAIN)gcov -p -t $(OBJS_LIB)

bin/%: src/main/prog/%.c $(OBJS_PROGS) $(BIN_LIB)
	mkdir -p "$$(dirname '$@')"
	$(CC) $(CFLAGS) $(CGLAGS_GUI) -o '$@' $^ $(CFLAGS_LIBS) $(CFLAGS_LIBS_GUI)

$(BIN_LIB): $(OBJS_LIB)
	$(CC) $(CFLAGS) $(CFLAGS_LIB_COMPILE) -fPIC -o '$@' $^ $(CFLAGS_LIBS)
	mkdir -p bin/
	cp "$@" bin/

uninstallPython:
	-rm -f $(PYTHON_INSTALL_FOLDER_LIB)/$(BIN_LIB_NAME)
	-rm -fr $(PYTHON_INSTALL_FOLDER_DATA)

_installPython: compile_lib uninstallPython
	@-echo "Installing library in the python package"
	mkdir -p "$(PYTHON_INSTALL_FOLDER_LIB)" "$(PYTHON_INSTALL_FOLDER_DATA)"

installPython: _installPython
	cp -f $(BIN_LIB) $(PYTHON_INSTALL_FOLDER_LIB)/$(BIN_LIB_NAME)
	cp -rf data/data $(PYTHON_INSTALL_FOLDER_DATA)/

installPythonDev: _installPython
	ln -s $(PWD)/$(BIN_LIB) $(PYTHON_INSTALL_FOLDER_LIB)/$(BIN_LIB_NAME)
	ln -s $(PWD)/data/data $(PYTHON_INSTALL_FOLDER_DATA)/

bin/%: src/test/%.c $(OBJS_PROGS) $(OBJS_TESTS) $(BIN_LIB)
	mkdir -p "$$(dirname '$@')"
	$(CC) $(CFLAGS) $(CGLAGS_GUI) $(CFLAGS_TESTS) $^ -o '$@' $(CFLAGS_LIBS) $(CFLAGS_LIBS_TESTS) $(CFLAGS_LIBS_GUI)

obj/main/%.o:
	@-echo "Compiling ($^) -> $@"
	@-printf "  "
	mkdir -p "$$(dirname '$@')"
	$(CC) $(CFLAGS) $(CGLAGS_GUI) $(CFLAGS_COVERAGE) -c $(filter %.c,$(^)) -o '$@'

obj/test/%.o:
	mkdir -p "$$(dirname '$@')"
	$(CC) $(CFLAGS) $(CGLAGS_GUI) $(CFLAGS_COVERAGE) $(CFLAGS_TESTS) -c $(filter %.c,$(^)) -o '$@'

# Additionnal specific dependencies
dependencies: cmd = $(CC) $(CFLAGS) $(CGLAGS_GUI) -I src/testFixtures/ -I include/main/ -MM -MT $(subst src/,obj/,$(var:.c=.o)) $(var) | sed 's/^\([ \t]*\)\/.*\(\\\)/\1\2/g' | sed 's/^\([ \t]*\)\/.*/\1/g' | grep -v -e "^[ \t]\+\\\\" >> dependencies.mk;
dependencies: $(SOURCES)
	@echo "Generating dependencies..."
	@> dependencies.mk
	@$(foreach var, $(SOURCES), $(cmd))	

-include dependencies.mk

clean:
	rm -rf obj/
	rm -fr pyautodiag/build
	rm -fr pyautodiag/dist
	rm -fr pyautodiag/*.egg-info

veryclean: clean
	rm -rf bin/

run: default
	bin/$(APP_NAME)
runDebug: default
	GTK_DEBUG=interactive AUTODIAG_LOG_LEVEL=debug bin/$(APP_NAME)
runTest: ./bin/regression
	./bin/regression

info:
	@-echo "OBJS=$(OBJS)"
	@-echo "OBJS_TESTS=$(OBJS_TESTS)"
	@-echo "OBJS_PROGS=$(OBJS_PROGS)"	
	@-echo "OBJS_LIB=$(OBJS_LIB)"
	@-echo "SOURCES=$(SOURCES)"

tarball:
	prefix="$(APP_NAME)-$(APP_VERSION)" && \
	tmp="/tmp/$(APP_NAME)fileList" && \
	mkdir -p "$${prefix}" && \
	git ls-tree -r HEAD --name-only | grep -v '^.git' | grep -v '^\.' > "$${tmp}" && \
	cpio -pdm "$${prefix}" < "$${tmp}" && \
	tar jcf "$${prefix}".tar.bz2 "$${prefix}" && \
	rm -rf "$${prefix}"

distDebianSrc: veryclean
	dpkg-source --format="$$(cat dist/debian/source/format)" -l$$(pwd)/dist/debian/changelog -c$$(pwd)/dist/debian/control -b .

distDebianBin: release_progs
	cd dist/ && dpkg-buildpackage -b --buildinfo-option=-u../bin/ --changes-option=-u../bin/ -us -uc --hook-done='fakeroot debian/rules done'
	# Due to missing behaviour in dpkg-genchanges(1.19.7) (that is in dpkg-genbuilinfo) look -u -O, we are required to do this
	mv $(APP_NAME)_*.changes ./bin/
	dpkg -I ./bin/$(APP_NAME)*$(APP_VERSION)*.deb

distWindows: release_progs
	powershell.exe -Command "& 'C:\Program Files (x86)\Inno Setup 6\ISCC.exe' 'dist/windows/package.iss'"

distMacOS: release_progs
	./dist/macos/package.sh

distDebian: distDebianBin

newVersion:
	@-control=app.mk ; \
	version=$$(bash -c "let version=$(APP_VERSION)+1 ; echo \$$version") ; \
	tmp="/tmp/control_tmp" ; \
	sed "s/APP_VERSION = .*/APP_VERSION = $${version}/" $${control} > $${tmp} ; \
		mv $${tmp} $${control} ; \
	\
	debchange --changelog dist/debian/changelog --release-heuristic log --newversion $${version} "Version $${version}" ; \
	\
	sed "s/AppVersion=.*/AppVersion=$${version}/" dist/windows/package.iss > $${tmp} ; \
		mv $${tmp} dist/windows/package.iss ; \
	\
	git add . ; \
	git commit -m "Version $${version}" ; \
	git tag "v$${version}"

_install: uninstall
	mkdir -p "$(INSTALL_DATA_FOLDER_APP)" "$(INSTALL_BIN_FOLDER)" "$(INSTALL_LIB_FOLDER)"

# Manual installation
install: _install
	cp -fr ui "$(INSTALL_DATA_FOLDER_APP)"
	cp -fr data/data "$(INSTALL_DATA_FOLDER_APP)"
	cp -fr media "$(INSTALL_DATA_FOLDER_APP)"
	-cp ./bin/* "$(INSTALL_BIN_FOLDER)"
	cp ./bin/libautodiag* "$(INSTALL_LIB_FOLDER)"
installDev: _install
	ln -s "$${PWD}/data/data" "$(INSTALL_DATA_FOLDER_APP)"
	ln -s "$${PWD}/ui" "$(INSTALL_DATA_FOLDER_APP)"
	ln -s "$${PWD}/media" "$(INSTALL_DATA_FOLDER_APP)"
	ln -s "$${PWD}"/bin/* "$(INSTALL_BIN_FOLDER)"
	ln -s "$${PWD}"/bin/libautodiag* "$(INSTALL_LIB_FOLDER)"
uninstall:
	rm -fr "$(INSTALL_DATA_FOLDER_APP)"
	for bin in "$${PWD}"/bin/*; do \
		if [ -f "$${bin}" ]; then \
			rm -f "$(INSTALL_BIN_FOLDER)/$$(basename "$${bin}")"; \
			rm -f "$(INSTALL_LIB_FOLDER)/$$(basename "$${bin}")"; \
		fi; \
	done
help:
	@-echo "Development setup"
	@-echo " install      		- copy files"
	@-echo " installDev 		- using symlinks"
	@-echo " uninstall    		- delete previous installation"
	@-echo " run          		- run the software"
	@-echo " runDebug     		- run with debug flags"
	@-echo " runTest      		- run regression test"
	@-echo " compile_progs  	- compile progs"
	@-echo " release_progs  	- compile progs with debugging info removed"
	@-echo " compile_tests 		- compile tests"
	@-echo " compile_lib  		- compile the library"
	@-echo " installPython		- install data in the python package"
	@-echo " installPythonDev	- same but using symlinks"
	@-echo " uninstallPython	- uninstall data from the python package"
	@-echo " coverage     		- recompile project with coverage information included"
	@-echo " dependencies 		- update project files dependencies"
	@-echo "Software management"
	@-echo " distDebian   		- package for debian"
	@-echo " distWindows  		- package in an installer for windows"
	@-echo " distMacOS    		- package as DMG for macOS"
	@-echo " newVersion   		- create a new version"
	@-echo "Configuration variables"
	@-echo " TOOLCHAIN           - prefix for the toolchain (eg TOOLCHAINgcc TOOLCHAINstrip)"
	@-echo " INSTALL_DATA_FOLDER - where to install application data"
	@-echo " INSTALL_BIN_FOLDER  - where to install application binaries"


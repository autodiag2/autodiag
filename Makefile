include config.mk
include app.mk

INSTALL_DATA_FOLDER_APP = $(INSTALL_DATA_FOLDER)/$(APP_NAME)/

# Programs
SOURCES_PROGS := $(sort $(call rwildcard,src/main/,*.c))
ifneq (,$(findstring release,$(MAKECMDGOALS)))
#   keep the debug info in production for now
#	CFLAGS += -DCOMPILE_RELEASE
endif

ifneq (,$(findstring compat,$(MAKECMDGOALS)))
	OBJS_PROGS := $(sort $(call filterout-multi, \
		output/obj/main/libprog/ui/%.o output/obj/main/ui/%.o \
		output/obj/main/libprog/sim_ecu_generator_gui.o output/obj/main/libautodiag/%.o \
		output/obj/main/prog/%.o, \
		$(subst src/main/,output/obj/main/,$(SOURCES_PROGS:.c=.o)) \
	))
	CFLAGS += -DCOMPILE_COMPAT
else
	OBJS_PROGS := $(sort $(call filterout-multi, \
		output/obj/main/libautodiag/%.o \
		output/obj/main/prog/%.o, \
		$(subst src/main/,output/obj/main/,$(SOURCES_PROGS:.c=.o)) \
	))
endif
BINS_PROGS := $(sort $(patsubst src/main/prog/%.c,output/bin/%,$(call rwildcard,src/main/prog/,*.c)))
BINS_EXAMPLES := $(sort $(patsubst src/example/%.c,output/bin/example_%,$(call rwildcard,src/example/,*.c)))

# Library shared object
OBJS_LIB := $(sort output/obj/sqlite3.o output/obj/mongoose.o output/obj/cJSON.o $(filter output/obj/main/libautodiag/%.o,$(subst src/main/,output/obj/main/,$(SOURCES_PROGS:.c=.o))))
BIN_LIB := $(BIN_LIB_NAME)
PYTHON_INSTALL_FOLDER_ROOT := pyautodiag/autodiag
PYTHON_INSTALL_FOLDER_LIB := $(PYTHON_INSTALL_FOLDER_ROOT)/libs/
PYTHON_INSTALL_FOLDER_DATA := $(PYTHON_INSTALL_FOLDER_ROOT)/data/

# Tests
BINS_TESTS := $(sort output/bin/regression output/bin/fuzz)
SOURCES_TESTS := $(sort $(call rwildcard,src/test/,*.c))
OBJS_TESTS := $(sort $(patsubst src/test/%.c,output/obj/test/%.o,$(SOURCES_TESTS)))
OBJS_TESTS := $(sort $(filter-out $(patsubst output/bin/%,output/obj/test/%.o,$(BINS_TESTS)),$(OBJS_TESTS)))

SOURCES := $(sort $(SOURCES_PROGS) $(SOURCES_TESTS))
OBJS := $(sort $(OBJS_PROGS) $(OBJS_TESTS) $(OBJS_LIB))

CFLAGS_TESTS = -I src/testFixtures/
CFLAGS_COVERAGE = 
CFLAGS_LIBS_TESTS = 

CFLAGS += $(DEBUG_CFLAGS)

CFLAGS += -DAPP_NAME="\"$(APP_NAME)\"" -DAPP_VERSION="\"$(APP_VERSION)\""
CFLAGS += -DAPP_MAINTAINER="\"$(APP_MAINTAINER)\"" -DAPP_DESC="\"$(APP_DESC)\""
CC = $(TOOLCHAIN)gcc

default: compile_progs

tools_prerequistes:
	@command -v cp > /dev/null 2>&1 || { echo "cp is required"; exit 1; }
	@command -v mkdir > /dev/null 2>&1 || { echo "mkdir is required"; exit 1; }
	@command -v rm > /dev/null 2>&1 || { echo "rm is required"; exit 1; }
	@command -v printf > /dev/null 2>&1 || { echo "printf is required"; exit 1; }
	@command -v ln > /dev/null 2>&1 || { echo "ln is required"; exit 1; }
	@command -v $(CC) > /dev/null 2>&1 || { echo "$(CC) is required"; exit 1; }

# With debugging info removed
release_progs: tools_prerequistes compile_progs
release_progs_compat: tools_prerequistes compile_progs_compat
release_lib: tools_prerequistes compile_lib

compile_progs_compat: tools_prerequistes output/bin/elm327sim_compat output/bin/doipsim_compat
	@echo "Software ready at: $(filter-out tools_prerequistes,$^)"

compile_progs: tools_prerequistes $(BINS_PROGS)
	@echo "Software ready at: $(filter-out tools_prerequistes,$^)"

compile_examples: tools_prerequistes $(BINS_EXAMPLES)
	@echo "Examples ready at: $(filter-out tools_prerequistes,$^)"

compile_tests: tools_prerequistes $(BINS_TESTS)
	@echo "Tests: $(filter-out tools_prerequistes,$^)"

compile_lib: tools_prerequistes $(BIN_LIB)
	@echo "Library ready at: $(filter-out tools_prerequistes,$^)"

coverage: CFLAGS_COVERAGE += --coverage
coverage: CFLAGS_LIBS_TESTS += -lgcov
coverage: tools_prerequistes veryclean compile_tests
	@command -v gcov > /dev/null 2>&1 || { echo "gcov is required"; exit 1; }
	echo "running coverage with : $(CFLAGS)"
	./output/bin/regression
	$(TOOLCHAIN)gcov -p -t $(OBJS_LIB)

uninstallPython: tools_prerequistes
	-rm -f $(PYTHON_INSTALL_FOLDER_LIB)/$(BIN_LIB_NAME)
	-rm -fr $(PYTHON_INSTALL_FOLDER_DATA)

_installPython: tools_prerequistes compile_lib uninstallPython
	@-echo "Installing library in the python package"
	mkdir -p "$(PYTHON_INSTALL_FOLDER_LIB)" "$(PYTHON_INSTALL_FOLDER_DATA)"

installPython: tools_prerequistes _installPython
	cp -f $(BIN_LIB) $(PYTHON_INSTALL_FOLDER_LIB)/$(BIN_LIB_NAME)
	cp -rf data/data $(PYTHON_INSTALL_FOLDER_DATA)/

installPythonDev: tools_prerequistes _installPython
	ln -s $(PWD)/$(BIN_LIB) $(PYTHON_INSTALL_FOLDER_LIB)/$(BIN_LIB_NAME)
	ln -s $(PWD)/data/data $(PYTHON_INSTALL_FOLDER_DATA)/

output/obj/main/ui/%.o output/obj/main/libprog/%.o:
	$(COMPILE_MSG)
	$(PRINT_VOIDER)mkdir -p "$$(dirname '$@')"
	$(PRINT_VOIDER)$(CC) $(CFLAGS) $(CGLAGS_GUI) $(CFLAGS_COVERAGE) -c $(subst output/,,$(filter %.c,$(^))) -o '$@'

output/obj/main/%.o:
	$(COMPILE_MSG)
	$(PRINT_VOIDER)mkdir -p "$$(dirname '$@')"
	$(PRINT_VOIDER)$(CC) $(CFLAGS) $(CFLAGS_COVERAGE) -c $(subst output/,,$(filter %.c,$(^))) -o '$@'

output/obj/mongoose.o: dependencies/mongoose/mongoose.h dependencies/mongoose/mongoose.c
	$(COMPILE_MSG)
	$(PRINT_VOIDER)mkdir -p "$$(dirname '$@')"
	$(PRINT_VOIDER)$(CC) $(CFLAGS) $(CFLAGS_COVERAGE) -c $(filter %.c,$(^)) -o '$@'

output/obj/cJSON.o: dependencies/cJSON/cJSON.h dependencies/cJSON/cJSON.c
	$(COMPILE_MSG)
	$(PRINT_VOIDER)mkdir -p "$$(dirname '$@')"
	$(PRINT_VOIDER)$(CC) $(CFLAGS) $(CFLAGS_COVERAGE) -c $(filter %.c,$(^)) -o '$@'

output/obj/sqlite3.o: dependencies/sqlite3/sqlite3.h dependencies/sqlite3/sqlite3.c
	$(COMPILE_MSG)
	$(PRINT_VOIDER)mkdir -p "$$(dirname '$@')"
	$(PRINT_VOIDER)$(CC) $(CFLAGS) $(CFLAGS_COVERAGE) -c $(filter %.c,$(^)) -o '$@'

output/obj/test/%.o:
	$(PRINT_VOIDER)mkdir -p "$$(dirname '$@')"
	$(PRINT_VOIDER)$(CC) $(CFLAGS) $(CGLAGS_GUI) $(CFLAGS_COVERAGE) $(CFLAGS_TESTS) -c $(subst output/,,$(filter %.c,$(^))) -o '$@'

output/bin/%: src/test/%.c $(OBJS_PROGS) $(OBJS_TESTS) $(BIN_LIB)
	$(PRINT_VOIDER)mkdir -p "$$(dirname '$@')"
	$(PRINT_VOIDER)$(CC) $(CFLAGS) $(CGLAGS_GUI) $(CFLAGS_TESTS) $^ -o '$@' $(CFLAGS_LIBS) $(CFLAGS_LIBS_TESTS) $(CFLAGS_LIBS_GUI)

output/bin/%_compat: src/main/prog/%.c $(OBJS_PROGS) $(BIN_LIB)
	$(PRINT_VOIDER)mkdir -p "$$(dirname '$@')"
	$(PRINT_VOIDER)if $(COMPILE_NEED_OBJS) ; then \
		$(CC) $(CFLAGS) -o '$@' $(OBJS_LIB) $^ $(CFLAGS_LIBS) ; \
	else \
		$(CC) $(CFLAGS) -o '$@' $^ ; \
	fi

output/bin/example_%: src/example/%.c $(OBJS_PROGS) $(BIN_LIB)
	$(PRINT_VOIDER)mkdir -p "$$(dirname '$@')"
	$(PRINT_VOIDER)$(CC) $(CFLAGS) $(CGLAGS_GUI) -o '$@' $^ $(CFLAGS_LIBS) $(CFLAGS_LIBS_GUI)

output/bin/%: src/main/prog/%.c $(OBJS_PROGS) $(BIN_LIB)
	$(PRINT_VOIDER)mkdir -p "$$(dirname '$@')"
	$(PRINT_VOIDER)$(CC) $(CFLAGS) $(CGLAGS_GUI) -o '$@' $^ $(CFLAGS_LIBS) $(CFLAGS_LIBS_GUI)

$(BIN_LIB): $(OBJS_LIB)
	$(PRINT_VOIDER)$(CC) $(CFLAGS) $(CFLAGS_LIB_COMPILE) -fPIC -o '$@' $^ $(CFLAGS_LIBS)
	$(PRINT_VOIDER)mkdir -p output/bin/
	$(PRINT_VOIDER)cp "$@" output/bin/

# Additionnal specific dependencies
genDependencies: cmd = $(CC) $(CFLAGS) $(CGLAGS_GUI) -I src/testFixtures/ -I include/main/ -MM -MT $(subst src/,output/obj/,$(var:.c=.o)) $(var) | sed 's/^\([ \t]*\)\/.*\(\\\)/\1\2/g' | sed 's/^\([ \t]*\)\/.*/\1/g' | grep -v -e "^[ \t]\+\\\\" >> dependencies.mk;
genDependencies: $(SOURCES)
	@command -v sed > /dev/null 2>&1 || { echo "sed is required"; exit 1; }
	@command -v grep > /dev/null 2>&1 || { echo "grep is required"; exit 1; }
	@echo "Generating dependencies..."
	@> dependencies.mk
	@$(foreach var, $(SOURCES), $(cmd))	

-include dependencies.mk

clean: tools_prerequistes
	rm -rf output/obj/
	rm -fr pyautodiag/build
	rm -fr pyautodiag/dist
	rm -fr pyautodiag/*.egg-info

veryclean: tools_prerequistes clean
	rm -rf output/bin/
	rm -fr output/doc/

run: tools_prerequistes default
	output/bin/$(APP_NAME) $(args)

runDebug: tools_prerequistes default
	GTK_DEBUG=interactive AUTODIAG_LOG_LEVEL=debug output/bin/$(APP_NAME) $(args)

testRegression: tools_prerequistes ./output/bin/regression
	./output/bin/regression $(args)

testFuzz: tools_prerequistes ./output/bin/fuzz
	./output/bin/fuzz $(args)

info:
	@-echo "OBJS=$(OBJS)"
	@-echo "OBJS_TESTS=$(OBJS_TESTS)"
	@-echo "OBJS_PROGS=$(OBJS_PROGS)"	
	@-echo "OBJS_LIB=$(OBJS_LIB)"
	@-echo "SOURCES=$(SOURCES)"

tarball: tools_prerequistes
	@command -v git > /dev/null 2>&1 || { echo "git is required"; exit 1; }
	@command -v cpio > /dev/null 2>&1 || { echo "cpio is required"; exit 1; }
	@command -v tar > /dev/null 2>&1 || { echo "tar is required"; exit 1; }
	prefix="$(APP_NAME)-$(APP_VERSION)" && \
	tmp="/tmp/$(APP_NAME)fileList" && \
	rm -rf "$${prefix}" && \
	mkdir -p "$${prefix}" && \
	git ls-tree -r HEAD --name-only | grep -v '^.git' | grep -v '^\.' > "$${tmp}" && \
	cpio -pdm "$${prefix}" < "$${tmp}" && \
	cp -a dependencies/mongoose "$${prefix}/" && \
	cp -a dependencies/sqlite3 "$${prefix}/" && \
	cp -a dependencies/cJSON "$${prefix}/" && \
	mkdir -p "$${prefix}/data" && \
	cp -a data/data "$${prefix}/data/" && \
	tar jcf "$${prefix}".tar.bz2 "$${prefix}" && \
	rm -rf "$${prefix}"

distDebianSrc: tools_prerequistes veryclean
	@command -v dpkg-source > /dev/null 2>&1 || { echo "dpkg-source is required"; exit 1; }
	dpkg-source --format="$$(cat dist/debian/source/format)" -l$$(pwd)/dist/debian/changelog -c$$(pwd)/dist/debian/control -b .

distDebianBin: tools_prerequistes release_progs
	@command -v dpkg > /dev/null 2>&1 || { echo "dpkg is required"; exit 1; }
	@command -v dpkg-buildpackage > /dev/null 2>&1 || { echo "dpkg-buildpackage is required"; exit 1; }
	cd dist/ && dpkg-buildpackage -b --buildinfo-option=-u../output/bin/ --changes-option=-u../output/bin/ -us -uc --hook-done='fakeroot debian/rules done'
	# Due to missing behaviour in dpkg-genchanges(1.19.7) (that is in dpkg-genbuilinfo) look -u -O, we are required to do this
	mv $(APP_NAME)_*.changes ./output/bin/
	dpkg -I ./output/bin/$(APP_NAME)*$(APP_VERSION)*.deb

distWindows: tools_prerequistes release_progs
	"/c/Program Files (x86)/Inno Setup 6/ISCC.exe" ./dist/windows/package.iss

distMacOS: tools_prerequistes release_progs
	./dist/macos/package.sh

distDebian: tools_prerequistes distDebianBin

distFedora: tools_prerequistes tarball
	@command -v rpmbuild > /dev/null 2>&1 || { echo "rpmbuild is required"; exit 1; }
	@command -v rpmdev-setuptree > /dev/null 2>&1 || { echo "rpmdev-setuptree is required"; exit 1; }

	@rpmdev-setuptree
	@mkdir -p ./output/bin/

	cp -f "$(APP_NAME)-$(APP_VERSION).tar.bz2" "$$HOME/rpmbuild/SOURCES/"
	cp -f "dist/fedora/$(APP_NAME).spec" "$$HOME/rpmbuild/SPECS/"

	rpmbuild -bb "$$HOME/rpmbuild/SPECS/$(APP_NAME).spec" \
		--define "_topdir $$HOME/rpmbuild" \
		--define "app_name $(APP_NAME)" \
		--define "app_version $(APP_VERSION)"

	@rpms="$$(find "$$HOME/rpmbuild/RPMS" -type f -name "$(APP_NAME)-*$(APP_VERSION)*.rpm")"; \
	test -n "$$rpms" || { echo "no binary rpm produced"; exit 1; }; \
	cp -f $$rpms ./output/bin/

	rpm -qpi ./output/bin/$(APP_NAME)-*$(APP_VERSION)*.rpm

newVersion: tools_prerequistes
	@command -v git > /dev/null 2>&1 || { echo "git is required to create a new version"; exit 1; }
	@command -v debchange > /dev/null 2>&1 || { echo "debchange is required to create a new version"; exit 1; }
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
install: tools_prerequistes _install
	cp -fr ui "$(INSTALL_DATA_FOLDER_APP)"
	cp -fr data/data "$(INSTALL_DATA_FOLDER_APP)"
	cp -fr media "$(INSTALL_DATA_FOLDER_APP)"
	-cp ./output/bin/autodiag "$(INSTALL_BIN_FOLDER)"
	-cp ./output/bin/elm327sim "$(INSTALL_BIN_FOLDER)"
	-cp ./output/bin/doipsim "$(INSTALL_BIN_FOLDER)"
	cp ./output/bin/libautodiag* "$(INSTALL_LIB_FOLDER)"
installDev: tools_prerequistes _install
	ln -s "$${PWD}/data/data" "$(INSTALL_DATA_FOLDER_APP)"
	ln -s "$${PWD}/ui" "$(INSTALL_DATA_FOLDER_APP)"
	ln -s "$${PWD}/media" "$(INSTALL_DATA_FOLDER_APP)"
	ln -s "$${PWD}"/output/bin/* "$(INSTALL_BIN_FOLDER)"
	ln -s "$${PWD}"/output/bin/libautodiag* "$(INSTALL_LIB_FOLDER)"
uninstall: tools_prerequistes
	rm -fr "$(INSTALL_DATA_FOLDER_APP)"
	for bin in "$${PWD}"/output/bin/*; do \
		if [ -f "$${bin}" ]; then \
			rm -f "$(INSTALL_BIN_FOLDER)/$$(basename "$${bin}")"; \
			rm -f "$(INSTALL_LIB_FOLDER)/$$(basename "$${bin}")"; \
		fi; \
	done
doc: tools_prerequistes
	@command -v doxygen > /dev/null 2>&1 || { echo "doxygen is required"; exit 1; }
	doxygen
	@-echo "Documentation generated in output/doc/html/index.html"
help:
	@-echo "Production rules"
	@-echo " release_progs            - compile progs with debugging info removed"
	@-echo " release_progs_compat     - compile progs maximizing compatibility with debugging info removed"
	@-echo " release_lib              - compile the library with debugging info removed"
	@-echo " distDebian               - package as .deb"
	@-echo " distWindows              - package in an installer for windows"
	@-echo " distMacOS                - package as DMG for macOS"
	@-echo " distFedora               - package as .rpm"
	@-echo " doc                      - generate documentation"
	@-echo " install                  - install on this computer"
	@-echo " uninstall                - uninstall"
	@-echo " run                      - run the software"
	@-echo "Development rules"
	@-echo " compile_lib              - compile the library"
	@-echo " compile_progs_compat     - compile progs maximizing compatibility"
	@-echo " compile_progs            - compile progs"
	@-echo " compile_tests            - compile tests"
	@-echo " compile_examples         - compile examples"
	@-echo " installDev               - install using symlinks"
	@-echo " runDebug                 - run with debug flags"
	@-echo " testRegression           - run regression test"
	@-echo " testFuzz                 - run fuzz test"
	@-echo " installPython            - install data in the python package"
	@-echo " installPythonDev         - same but using symlinks"
	@-echo " uninstallPython          - uninstall data from the python package"
	@-echo " coverage                 - recompile project with coverage information included"
	@-echo " genDependencies          - update make dependencies"
	@-echo " newVersion               - create a new version of the software"
	@-echo "Environment variables"
	@-echo " Compile time"
	@-echo "  TOOLCHAIN               - prefix for the toolchain (eg TOOLCHAINgcc TOOLCHAINstrip)"
	@-echo "  COMPILE_VERBOSE         - set to any value to increase the verbosity of compiling"
	@-echo "  INSTALL_DATA_FOLDER     - where to install application data"
	@-echo "  INSTALL_BIN_FOLDER      - where to install application binaries"
	@-echo " Runtime"
	@-echo "  AUTODIAG_LOG_LEVEL         - log messages with defined log level (none,info,warning,error,debug)"
	@-echo "  AUTODIAG_LOG_TIMESTAMP     - log messages with timestamp (true, false and require log level)"
	@-echo "  AUTODIAG_LOG_CODE_LOCATION - log messages with source code location (true, false and require log level)"
	@-echo "  AUTODIAG_LOG_COLOR         - log messages with colors (true, false and require log level)"
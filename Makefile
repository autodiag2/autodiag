include config.mk
include app.mk

INSTALL_FOLDER = $(INSTALL_DATA_FOLDER)/$(APP_NAME)/

SOURCES_main = $(call rwildcard,src/main/,*.c)
# all objects needed for the main application
OBJS_main = $(filter-out obj/main/prog/%.o,$(subst src/main/,obj/main/,$(SOURCES_main:.c=.o)))
OUTPUT_APP = bin/$(APP_NAME)
OUTPUT_MAIN = $(OUTPUT_APP) bin/elm327sim

SOURCES_test = $(call rwildcard,src/test/,*.c)
#OUTPUT_TESTS = $(subst src/test/,bin/,$(SOURCES_test:.c=))
OBJS_test = $(subst obj/test/regression.o,,$(subst obj/test/obd_get_pid_supported.o,,$(subst src/test/,obj/test/,$(SOURCES_test:.c=.o))))
OUTPUT_TESTS = bin/regression bin/obd_get_pid_supported

# objects of the library
OBJS_lib = obj/main/ui/config.o $(filter-out obj/main/ui/%.o,$(OBJS_main))


SOURCES = $(SOURCES_main) $(SOURCES_test)
OBJS = $(OBJS_main) $(OBJS_test)

CFLAGS_TESTS = -I src/testFixtures/
CFLAGS_OBJECTS = 
LIBS_TESTS = 

ifeq ($(filter release, $(MAKECMDGOALS)),)
        CFLAGS += $(DEBUG_CFLAGS)
endif

CFLAGS += -DAPP_NAME="\"$(APP_NAME)\"" -DAPP_VERSION="\"$(APP_VERSION)\""
CFLAGS += -DAPP_MAINTAINER="\"$(APP_MAINTAINER)\"" -DAPP_DESC="\"$(APP_DESC)\""
CC = $(TOOLCHAIN)gcc

default: compile_main

release: compile_main
	@-$(TOOLCHAIN)strip $(OUTPUT_MAIN)

compile_main: $(OUTPUT_MAIN)
	@-echo "Software ready at: $(OUTPUT_APP)"

compile_test: $(OUTPUT_TESTS)
	@-echo "Tests: $(OUTPUT_TESTS)"

coverage: CFLAGS_OBJECTS += --coverage
coverage: LIBS_TESTS += -lgcov
coverage: veryclean compile_test
	echo "running coverage with : $(CFLAGS)"
	./bin/regression
	$(TOOLCHAIN)gcov -p -t $(OBJS_lib)

bin/$(APP_NAME): $(OBJS_main) src/main/prog/autodiag.c
	mkdir -p "$$(dirname '$@')"
	$(CC) $(CFLAGS) $^ -o '$@' $(LIBS)

bin/elm327sim: $(OBJS_main) src/main/prog/elm327sim.c
	mkdir -p "$$(dirname '$@')"
	$(CC) $(CFLAGS) $^ -o '$@' $(LIBS)

bin/%: src/test/%.c $(OBJS_main) $(OBJS_test) 
	mkdir -p "$$(dirname '$@')"
	$(CC) $(CFLAGS) $(CFLAGS_TESTS) $^ -o '$@' $(LIBS) $(LIBS_TESTS)

obj/main/%.o:
	@-echo "Compiling ($^) -> $@"
	@-printf "  "
	mkdir -p "$$(dirname '$@')"
	$(CC) $(CFLAGS) $(CFLAGS_OBJECTS) -c $(filter %.c,$(^)) -o '$@'

obj/test/%.o:
	mkdir -p "$$(dirname '$@')"
	$(CC) $(CFLAGS) $(CFLAGS_OBJECTS) $(CFLAGS_TESTS) -c $(filter %.c,$(^)) -o '$@'

# Additionnal specific dependencies
dependencies: cmd = $(CC) $(CFLAGS) -I src/testFixtures/ -I include/main/ -MM -MT $(subst src/,obj/,$(var:.c=.o)) $(var) | sed 's/^\([ \t]*\)\/.*\(\\\)/\1\2/g' | sed 's/^\([ \t]*\)\/.*/\1/g' | grep -v -e "^[ \t]\+\\\\" >> dependencies.mk;
dependencies: $(SOURCES)
	@echo "Generating dependencies..."
	@> dependencies.mk
	@$(foreach var, $(SOURCES), $(cmd))	

-include dependencies.mk

clean:
	rm -rf obj/
veryclean: clean
	rm -rf bin/

run: default
	$(OUTPUT_APP)
runDebug: default
	GTK_DEBUG=interactive AUTODIAG_LOG_LEVEL=debug $(OUTPUT_APP)
runTest: ./bin/regression
	./bin/regression

info:
	@-echo "OBJS=$(OBJS)"
	@-echo "OBJS_test=$(OBJS_test)"
	@-echo "OBJS_main=$(OBJS_main)"	
	@-echo "OBJS_lib=$(OBJS_lib)"
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

distDebianBin: release
	cd dist/ && dpkg-buildpackage -b --buildinfo-option=-u../bin/ --changes-option=-u../bin/ -us -uc --hook-done='fakeroot debian/rules done'
	# Due to missing behaviour in dpkg-genchanges(1.19.7) (that is in dpkg-genbuilinfo) look -u -O, we are required to do this
	mv $(APP_NAME)_*.changes ./bin/
	dpkg -I ./bin/$(APP_NAME)*$(APP_VERSION)*.deb

distWindows: release
	echo '& "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" dist/windows/package.iss' | powershell.exe

distMacOS: release
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

# Manual installation
install: uninstall
	mkdir -p "$(INSTALL_FOLDER)" "$(INSTALL_BIN_FOLDER)"
	cp -fr ui "$(INSTALL_FOLDER)"
	cp -fr data/data "$(INSTALL_FOLDER)"
	cp -fr media "$(INSTALL_FOLDER)"
	cp ./bin/$(APP_NAME) "$(INSTALL_BIN_FOLDER)"
installDebug: uninstall
	mkdir -p "$(INSTALL_FOLDER)" "$(INSTALL_BIN_FOLDER)"
	ln -s "$${PWD}/data/data" "$(INSTALL_FOLDER)"
	ln -s "$${PWD}/ui" "$(INSTALL_FOLDER)"
	ln -s "$${PWD}/media" "$(INSTALL_FOLDER)"
	ln -s "$${PWD}/bin/$(APP_NAME)" "$(INSTALL_BIN_FOLDER)"
uninstall:
	rm -fr "$(INSTALL_FOLDER)"
	rm -f "$(INSTALL_BIN_FOLDER)/$(APP_NAME)"
help:
	@-echo "Development setup"
	@-echo " install      - copy files"
	@-echo " installDebug - using symlinks"
	@-echo " uninstall    - delete previous installation"
	@-echo " run          - run the software"
	@-echo " runDebug     - run with debug flags"
	@-echo " runTest      - run regression test"
	@-echo " compile_main - compile the main"
	@-echo " release      - compile the main with debugging info removed"
	@-echo " compile_test - compile tests"
	@-echo " coverage     - recompile project with coverage information included"
	@-echo " dependencies - update project files dependencies"
	@-echo "Software management"
	@-echo " distDebian   - package for debian"
	@-echo " distWindows  - package in an installer for windows"
	@-echo " distMacOS    - package as DMG for macOS"
	@-echo " newVersion   - create a new version"
	@-echo "Configuration variables"
	@-echo " TOOLCHAIN           - prefix for the toolchain (eg TOOLCHAINgcc TOOLCHAINstrip)"
	@-echo " INSTALL_DATA_FOLDER - where to install application data"
	@-echo " INSTALL_BIN_FOLDER  - where to install application binaries"


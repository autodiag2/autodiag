
### Overridable options with `make -e`

# possible values: ~/.local/ /usr/
INSTALL_ROOT = ~/.local/

### End overridable options ###

include Makefile.global
INSTALL_FOLDER = $(INSTALL_ROOT)/share/autodiag/
INSTALL_BIN_FOLDER = $(INSTALL_ROOT)/bin/
SOURCES_main = $(call rwildcard,src/main/,*.c)
SOURCES_test = $(call rwildcard,src/test/,*.c)
SOURCES = $(SOURCES_main) $(SOURCES_test)
OBJS_main = $(subst src/main/,obj/main/,$(patsubst %.c,%.o,$(SOURCES_main)))
OBJS = $(OBJS_main)
OUTPUT = bin/autodiag
default: compile_main
compile_main: compile_prerequisite $(OBJS_main)
	@-echo "Final assembly ..."
	@-printf "    "
	$(CC) $(CFLAGS) $(OBJS_main) -o $(OUTPUT) $(LIBS)
	@-echo
	@-echo "Software ready:"
	@-echo "    $(OUTPUT)"
	@-echo
compile_test: src/test/*.c
	@-echo
	@-echo "Compile tests:"
	@-echo
	@-objects="$(subst obj/main/ui/options.o,,$(subst obj/main/ui/main.o,,$(OBJS_main)))" ; \
	for file in $^ ; do \
		output="bin/$$(basename -s .c $$file)" ; \
		$(CC) $(CFLAGS) $${objects} $${file} -o $$output $(LIBS) & \
		echo "	- $$output" ; \
	done ; \
	wait $$(jobs -p) \

	@-echo
	@-echo "Tests compiled ..."
	@-echo
compile_prerequisite:
	@-echo "Compilation prerequisites ..."
	@-printf "    "
	@-mkdir -p bin $(dir $(OBJS))
	@-echo
obj/%.o: src/%.c include/%.h
	@-echo "Compiling ($^) -> $@"
	@-printf "    "
	$(CC) $(CFLAGS) -c "$(filter %.c,$(^))" -o "$@"
	@-echo
# Additionnal specific dependencies
obj/main/globals.o: obj/main/log.o
obj/main/main.o: obj/main/log.o obj/main/globals.o obj/main/trouble_code_reader.o obj/main/com/serial/serial.o obj/main/options.o
obj/main/options.o: obj/main/log.o obj/main/com/serial/serial.o
obj/main/com/serial/serial.o: obj/main/log.o obj/main/globals.o obj/main/main.o
obj/main/trouble_code_reader.o: obj/main/log.o obj/main/main.o
obj/main/com/obd/obd.o: obj/main/log.o obj/main/globals.o obj/main/com/serial/serial.o
obj/main/com/serial/uds/obd/saej1979.o: obj/main/com/serial/uds/uds.o

clean:
	rm -rf obj/ debian/autodiag/
veryclean: clean
	rm -rf bin/ debian/autodiag*

run: default
	$(OUTPUT)
run_debug: default
	GTK_DEBUG=interactive AUTODIAG_LOG_LEVEL=debug $(OUTPUT)

info:
	@-echo "OBJS=$(OBJS)"
	@-echo "SOURCES=$(SOURCES)"

buildDebian:
	dpkg-buildpackage -b -us -uc --hook-done='fakeroot debian/rules done'

buildDistro: buildDebian

newVersion:
	@-control=properties.mk ; \
	search="APP_VERSION = " ; \
	if ! [ -e $${control} ] ; then \
		version=0 ; \
	else \
		version=$$(grep "$${search}" $${control} |sed "s/$${search}//" ); \
	fi ; \
	version=$$(bash -c "let version=$$version+1 ; echo \$$version") ; \
	tmp="/tmp/control_tmp" ; \
	sed "s/$${search}.*/$${search}$${version}/g" $${control} > $${tmp} ; \
	mv $${tmp} $${control} ; \
	\
	debchange --release-heuristic log --newversion $${version} "Version $${version}" ; \
	\
	git add . ; \
	git commit -m "Version $$version" ; \
	git tag "v$$version"

# Manual installation
install: uninstall
	mkdir -p $(INSTALL_FOLDER) $(INSTALL_BIN_FOLDER)
	cp -fr {ui,data/data,media}/ $(INSTALL_FOLDER)
	cp ./bin/autodiag $(INSTALL_BIN_FOLDER)
installDebug: uninstall
	mkdir -p $(INSTALL_FOLDER) $(INSTALL_BIN_FOLDER)
	ln -s $${PWD}/{data/data,ui,media}/ $(INSTALL_FOLDER)
	ln -s $${PWD}/bin/autodiag $(INSTALL_BIN_FOLDER)
uninstall:
	rm -fr $(INSTALL_FOLDER)
	rm -f $(INSTALL_BIN_FOLDER)/autodiag


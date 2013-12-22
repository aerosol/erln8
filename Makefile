ERLBINS=Install escript run_erl beam dyn_erl etop runcgi.sh beam.smp edoc_generate format_man_pages snmpc bench.sh emem getop start cdv epmd heart start_webtool child_setup erl inet_gethost to_erl codeline_preprocessing.escript erl.src makewhatis typer ct_run erl_call memsup xml_from_edoc.escript dialyzer erlc odbcserver diameterc erlexec printenv.sh

PREFIX=$(DESTDIR)/usr/local/bin
CC=gcc
CFLAGS=-O2 -Wall
PROG=erln8

$(PROG): *.c
	$(CC) -o erln8 erln8.c `pkg-config --cflags --libs glib-2.0 gio-2.0` -DGLIB_DISABLE_DEPRECATION_WARNINGS $(CFLAGS)

format:
	astyle --style=attach --indent=spaces=2 --indent-cases --delete-empty-lines --align-pointer=type --align-reference=type --mode=c ./erln8.c

clean:
	rm -f erln8

install:
	echo "Installing"
	cp ./erln8 $(PREFIX)/erln8
	$(foreach b,$(ERLBINS),ln -s $(PREFIX)/erln8 $(PREFIX)/$(b);)

uninstall:
	echo "Uninstalling"
	rm -f $(PREFIX)/$(PROG)
	$(foreach b,$(ERLBINS),rm -f $(PREFIX)/$(b);)

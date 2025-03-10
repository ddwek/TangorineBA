all: ui.gresource

ui.gresource: ui.xml
	glib-compile-resources --target $@ --generate $<

clean-local:
	rm -f ui.gresource

distclean-local:
	rm -f ui.gresource

resdir = $(datadir)/TangorineBA/data
res_DATA = ui.gresource

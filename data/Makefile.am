all: ui.gresource

ui.gresource: ui.xml
	glib-compile-resources --target ../src/resources.cc --generate $<

clean-local:
	rm -f ../src/resources.cc
	rm -f ui.gresource

distclean-local:
	rm -f ../src/resources.cc
	rm -f ui.gresource

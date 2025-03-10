TANGORINEBA_DATADIR = ${prefix}/share/TangorineBA/

AM_CXXFLAGS = `pkg-config --cflags gtk+-3.0` -std=c++11 -O2
AM_CXXFLAGS += -DTANGORINEBA_DATADIR=\"${TANGORINEBA_DATADIR}\"
AM_CXXFLAGS += -DTANGORINEBA=\"${datarootdir}\"

AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = TangorineBA
TangorineBA_SOURCES = \
		Board.cc \
		Digraph.cc \
		Callback.cc \
		../test/Test.cc \
		main.cc

LIBS = `pkg-config --libs gtk+-3.0`

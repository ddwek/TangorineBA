TangorineBA_DATADIR = ${prefix}/share/TangorineBA/

AM_CXXFLAGS = `pkg-config --cflags gtk+-3.0` -std=c++11 -O2
AM_CXXFLAGS += -DTangorineBA_DATADIR=\"${TangorineBA_DATADIR}\"
AM_CXXFLAGS += -DTangorineBA=\"${datarootdir}\"

AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = TangorineBA
TangorineBA_SOURCES = \
		Board.cc \
		Digraph.cc \
		Callback.cc \
		resources.cc \
		../test/Test.cc \
		main.cc

LIBS = `pkg-config --libs gtk+-3.0`

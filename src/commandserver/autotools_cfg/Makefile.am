## Place generated object files (.o) into the same directory as their source
## files, in order to avoid collisions when non-recursive make is used.
AUTOMAKE_OPTIONS = foreign subdir-objects

## Additional flags to pass to aclocal when it is invoked automatically at
## make time. The ${ACLOCAL_FLAGS} variable is picked up from the environment
## to provide a way for the user to supply additional arguments.
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS} 

CFLAGS = -I/usr/include/boost
LIBS = -lboost_program_options \
       -lboost_thread

AM_CPPFLAGS = -std=c++0x $(DEPS_CFLAGS)

## Define an executable target "hello", which will be installed into the
## directory named by the predefined variable $(bindir).
bin_PROGRAMS = commandserver 

## Define the list of source files for the "hello" target. The file extension
## .cc is recognized by Automake, and causes it to produce rules which invoke
## the C++ compiler to produce an object file (.o) from each source file. The
## header files (.h) do not result in object files by themselves, but will be
## included in distribution archives of the project.
commandserver_SOURCES = src/commandserver.cpp src/log.hpp

commandserver_LDADD = $(DEPS_LIBS)


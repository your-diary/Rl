.RECIPEPREFIX := $(.RECIPEPREFIX) 
SHELL = /bin/bash
.DELETE_ON_ERROR :

binary_name := rl.out
source_name := rl.cpp
header_name := header_pty/pty.h header/readline_helper.h header/misc.h header/parse_string.h header/debug.h

CXXFLAGS := -pthread
LDLIBS := -lreadline

.PHONY: all debug install uninstall clean

all : CXXFLAGS += -O3 -D NDEBUG
all : $(binary_name)

debug : CXXFLAGS += -Og -g
debug : $(binary_name)

$(binary_name) : $(source_name) $(header_name)
    g++ -o $@ $(CXXFLAGS) $< $(LDLIBS)

install : $(binary_name)
    $(if $(prefix),\
    mv $^ $(prefix),\
    $(error Please define `prefix` variable.))

clean :
    rm $(binary_name)

uninstall :
    $(if $(prefix),\
    rm $(prefix)/$(binary_name),\
    $(error Please define `prefix` variable.))


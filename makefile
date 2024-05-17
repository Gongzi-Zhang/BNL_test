SHELL       := /bin/bash
CXXFLAGS    := -std=c++11 -Iinclude
VPATH	    := include

root_libs    = `root-config --libs --glibs --cflags`
sqlite3_libs = -l sqlite3

# include file
cali 	:= cali.h
db	:= db.C

# libraries
libdb	:= lib/libdb.so


$(libdb): $(db) $(cali)
	g++ $(CXXFLAGS) -fPIC --shared -o $@ $< $(sqlite3_libs)


test: test.C $(libdb) 
	g++ $(CXXFLAGS) -o $@ $^ $(sqlite3_libs)
	mv $@ bin/

QA: QA.C $(libdb) 
	g++ $(CXXFLAGS) -o $@ $^ $(sqlite3_libs) $(root_libs)
	mv $@ bin/

all: $(libdb)
# vim: set shiftwidth=4 softtabstop=4 tabstop=8: #

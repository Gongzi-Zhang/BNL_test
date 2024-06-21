SHELL       := /bin/bash
CXXFLAGS    := -std=c++11 -Iinclude
VPATH	    := include

root_libs    = `root-config --libs --glibs --cflags`
sqlite3_libs = -l sqlite3
cali_libs  = -Llib/ -ldb

# include file
cali 	:= cali.h
db	:= db.C

# libraries
libdb	:= libdb.so


$(libdb): $(db) $(cali)
	g++ $(CXXFLAGS) -fPIC --shared -o $@ $< $(sqlite3_libs)
	mv $@ lib/


test: test.C $(libdb) 
	g++ $(CXXFLAGS) -o $@ $^ $(sqlite3_libs)
	mv $@ bin/

QA: QA.C QA.h
	g++ $(CXXFLAGS) -o $@ $^ $(cali_libs) $(sqlite3_libs) $(root_libs)
	mv $@ bin/

makeTree: makeTree.C makeTree.h 
	g++ $(CXXFLAGS) -o $@ $^ $(cali_libs) $(sqlite3_libs) $(root_libs)
	mv $@ bin/

makeCosmic: makeCosmic.C makeCosmic.h buildEvent.h
	g++ $(CXXFLAGS) -o $@ $^ $(cali_libs) $(sqlite3_libs) $(root_libs)
	mv $@ bin/

makeTree1: makeTree1.C makeTree.h buildEvent.h
	g++ $(CXXFLAGS) -o $@ $^ $(cali_libs) $(sqlite3_libs) $(root_libs)

all: $(libdb)
# vim: set shiftwidth=4 softtabstop=4 tabstop=8: #

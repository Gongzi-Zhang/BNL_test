SHELL       := /bin/bash
CXXFLAGS    := -std=c++11 -Iinclude
VPATH	    := include

root_libs    = `root-config --libs --glibs --cflags`
sqlite3_libs = -l sqlite3
cali_libs  = -Llib/ -lmakeTree

# include file
calo 	:= calo.h
cali 	:= cali.h
utilities   := utilities.h
db	    := db.C
makeTree  := makeTree.C

# libraries
libdb	:= libdb.so
libmt	:= libmakeTree.so


$(libdb): $(db) $(cali)
	g++ $(CXXFLAGS) -fPIC --shared -o $@ $< $(sqlite3_libs)
	mv $@ lib/
$(libmt): $(makeTree)
	g++ $(CXXFLAGS) -fPIC --shared -o $@ $< $(root_libs)
	mv $@ lib/


test: test.C $(libdb) 
	g++ $(CXXFLAGS) -o $@ $^ $(sqlite3_libs)
	mv $@ bin/

QA: QA.C
	g++ $(CXXFLAGS) -o $@ $^ $(sqlite3_libs) $(root_libs)
	mv $@ bin/
MIP: MIP.C
	g++ $(CXXFLAGS) -o $@ $^ $(sqlite3_libs) $(root_libs)
	mv $@ bin/

convert: convert.C
	g++ $(CXXFLAGS) -o $@ $^ $(cali_libs) $(sqlite3_libs) $(root_libs)
	mv $@ bin/
calibrate: calibrate.C
	g++ $(CXXFLAGS) -o $@ $^ $(cali_libs) $(sqlite3_libs) $(root_libs)
	mv $@ bin/

convert1: convert1.C
	g++ $(CXXFLAGS) -o $@ $^ $(cali_libs) $(sqlite3_libs) $(root_libs)
	mv $@ bin/

makeCosmic: makeCosmic.C makeCosmic.h 
	g++ $(CXXFLAGS) -o $@ $^ $(cali_libs) $(sqlite3_libs) $(root_libs)
	mv $@ bin/

all: $(libdb)
# vim: set shiftwidth=4 softtabstop=4 tabstop=8: #

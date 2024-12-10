SHELL       := /bin/bash
CXXFLAGS    := -std=c++11
VPATH	    := include

root_libs    = `root-config --libs --glibs --cflags`
sqlite3_libs = -l sqlite3
cali_libs  = -Llib/ -lmakeTree

# include file
calo	    := calo.h
cali	    := cali.h
caliType    := caliType.h
utilities   := utilities.h
db	    := db.h
convert	    := convert.h
calibrate   := calibrate.h
QA	    := QA.h
MIP	    := MIP.h

cali_include = calo.h cali.h caliType.h utilities.h db.h convert.h calibrate.h QA.h MIP.h makeRec.h clustering.h

# dictionary
Dict	:= caliTypeDict.cxx



# $(libdb): $(db) $(cali)
# 	g++ $(CXXFLAGS) -fPIC --shared -o $@ $< $(sqlite3_libs)
# 	mv $@ lib/
# $(libmt): $(makeTree)
# 	g++ $(CXXFLAGS) -fPIC --shared -o $@ $< $(root_libs)
# 	mv $@ lib/

cali: cali.C $(Dict) $(cali_include)
	g++ $(CXXFLAGS) -o $@ $^ $(sqlite3_libs) $(root_libs)
	mv $@ bin/

test: test.C $(libdb) 
	g++ $(CXXFLAGS) -o $@ $^ $(sqlite3_libs)
	mv $@ bin/

QA: QA.C QA.h
	g++ $(CXXFLAGS) -o $@ $^ $(sqlite3_libs) $(root_libs)
	mv $@ bin/
MIP: MIP.C
	g++ $(CXXFLAGS) -o $@ $^ $(sqlite3_libs) $(root_libs)
	mv $@ bin/
LGMIP: LGMIP.C
	g++ $(CXXFLAGS) -o $@ $^ $(root_libs)
	mv $@ bin/

convert: convert.C
	g++ $(CXXFLAGS) -o $@ $^ $(cali_libs) $(sqlite3_libs) $(root_libs)
	mv $@ bin/
calibrate: calibrate.C
	g++ $(CXXFLAGS) -o $@ $^ $(cali_libs) $(sqlite3_libs) $(root_libs)
	mv $@ bin/
makeEdm4eic: makeEdm4eic.C
	# check eic-shell environment
	@if [ -z "$(DETECTOR)" ] || [ "epic" != "$(DETECTOR)" ]; then
	    echo -e "ERROR\t eic-shell not set.\n makeEdm4eic must be compiled within eic-shell"
	    exit 1
	fi
	# g++ -o edm4hep edm4hep.cpp -I../EDM4hep/install/include/ -I../podio/install/include/ -I/home/weibin/local/root/include -L../DEM4hep/install/lib/ -ledm4hep -L../podio/install/lib -lpodio -lpodioRootIO
	mv $@ bin/

# $(Dict): $(caliType) LinkDef.h
# 	rootcling -f $@ -c $^

makeRecTree: makeRecTree.C $(Dict)
	g++ $(CXXFLAGS) -o $@ $^ $(cali_libs) $(sqlite3_libs) $(root_libs)
	mv $@ bin/
clustering: clustering.C $(Dict)
	g++ $(CXXFLAGS) -o $@ $^ $(root_libs)
	mv $@ bin/

convert1: convert1.C
	g++ $(CXXFLAGS) -o $@ $^ $(cali_libs) $(sqlite3_libs) $(root_libs)
	mv $@ bin/

makeCosmic: makeCosmic.C makeCosmic.h 
	g++ $(CXXFLAGS) -o $@ $^ $(cali_libs) $(sqlite3_libs) $(root_libs)
	mv $@ bin/

all: $(libdb)
# vim: set shiftwidth=4 softtabstop=4 tabstop=8: #

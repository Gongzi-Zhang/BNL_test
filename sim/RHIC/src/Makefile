ROOTLIBS = $(shell root-config --libs)
CXX      =  g++
# CXX      =  gcc
CXXFLAGS =  -m64 -fno-inline -O  -W -Wall -pthread -std=c++11  -Wno-deprecated-declarations 
CPPFLAGS = -I${EICDIRECTORY}/include -I$(ROOTSYS)/include
LDFLAGS  = -L${EICDIRECTORY}/lib -Wl,-rpath,${EICDIRECTORY}/lib -lpythia8 -ldl $(ROOTLIBS) -L${EICDIRECTORY}/lib64 -lHepMC3

RHIC_pp: RHIC_pp.cc 
	$(CXX) $(CXXFLAGS)  $(PROGRAM).cc $(CPPFLAGS) $(LDFLAGS) -o $(PROGRAM) 

RHIC_pp_filter:	RHIC_pp_filter.cc 
	$(CXX) $(CXXFLAGS)  $^ $(CPPFLAGS) $(LDFLAGS) -o $@

RHIC_pp_filt_T1: RHIC_pp_filt_T1.cc 
	$(CXX) $(CXXFLAGS)  $^ $(CPPFLAGS) $(LDFLAGS) -o $@

RHIC_pp_pi0_filter: RHIC_pp_pi0_filter.cc 
	$(CXX) $(CXXFLAGS)  $^ $(CPPFLAGS) $(LDFLAGS) -o $@



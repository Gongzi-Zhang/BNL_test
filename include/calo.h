#ifndef __CALO__
#define __CALO__

#include <iostream>
#include <vector>
#include "utilities.h"

// units
const double s = 1;
const double ms = 1e-3*s;
const double us = 1e-6*s;

const double m = 1;
const double cm = 1e-2*m;
const double mm = 1e-3*m;

namespace calo {
    const char* gains[] = {"LG", "HG"};

    int nCAENs = 0;
    int nChannels = 0;
    std::vector<int> nCAENChannels;	// number of channels in each CAEN unit
    std::vector<int> preChannels;	// number of channels before this CAEN unit

    void setnCAENChannels(std::vector<int> nCh)
    {
	nCAENs = nCh.size();
	if (!nCAENs)
	{
	    std::cerr << WARNING << "empty value for number of channels in each CAEN unit" << std::endl;
	    return;
	}
	nCAENChannels.clear();
	preChannels.clear();
	nCAENChannels = nCh;
	preChannels = std::vector<int> (nCAENs, 0);
	nChannels = nCh[0];

	for (int i=1; i<nCAENs; i++)
	{
	    nChannels += nCh[i];
	    preChannels[i] = preChannels[i-1] + nCh[i-1];
	}
    }
}

#endif

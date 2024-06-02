#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <nlohmann/json.hpp>
#include "cali.h"

using namespace std;

struct chPed {
    double mean;
    double rms;
};

typedef map<string, map<int, chPed>> pedestal;

bool getPedestal(const int pedRun, pedestal &res)
{
    char pedFileName[1024];
    sprintf(pedFileName, "%s/data/Run%d_ped.json", cali::CALIROOT, pedRun);
    if (!fileExists(pedFileName))
    {
	cerr << FATAL << "no ped result for ped run: " << pedRun << ", please check it" << endl;
	return false;
    }

    ifstream pedFile(pedFileName);
    auto ped = nlohmann::json::parse(pedFile);

    for (auto gain : cali::gains)
    {
	for (int ch=0; ch<cali::nChannels; ch++)
	{
	    const char* chName = to_string(ch).c_str();
	    double mean = ped[gain][chName][0];
	    double rms = ped[gain][chName][1];
	    res[gain][ch] = {mean, rms};
	}
    }

    return true;
}

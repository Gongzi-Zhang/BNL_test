#ifndef _ANALYSIS_
#define _ANALYSIS_

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <nlohmann/json.hpp>
#include "calo.h"
#include "cali.h"

using namespace std;

struct chPed {
    double mean;
    double rms;
};

typedef map<int, map<string, chPed>> ped_t;
typedef map<int, map<string, double>> mip_t;

bool getPedestal(const char* pedFileName, ped_t &res)
{
    if (!fileExists(pedFileName))
    {
	cerr << FATAL << "ped file doesn't exist: " << pedFileName << endl;
	return false;
    }

    ifstream pedFile(pedFileName);
    auto ped = nlohmann::json::parse(pedFile);
    pedFile.close();

    for (auto gain : calo::gains)
    {
	for (int ch=0; ch<calo::nChannels; ch++)
	{
	    string chName = to_string(ch);
	    double mean = ped[gain][chName][0];
	    double rms = ped[gain][chName][1];
	    res[ch][gain] = {mean, rms};
	}
    }

    return true;
}

bool getPedestal(const int pedRun, ped_t &res)
{
    char pedFileName[1024];
    sprintf(pedFileName, "%s/data/Run%d_ped.json", cali::CALIROOT, pedRun);
    return getPedestal(pedFileName, res);
}

bool getMIP(const char* mipFileName, mip_t &res)
{
    if (!fileExists(mipFileName))
    {
	cerr << FATAL << "MIP file doesn't exist: " << mipFileName << endl;
	return false;
    }

    ifstream mipFile(mipFileName);
    auto mip = nlohmann::json::parse(mipFile);
    mipFile.close();

    for (auto gain : calo::gains)
    {
	for (int ch=0; ch<calo::nChannels; ch++)
	{
	    res[ch][gain] = mip[gain][to_string(ch)];
	}
    }

    return true;
}

bool getMIP(const int mipRun, mip_t &res)
{
    char mipFileName[1024];
    sprintf(mipFileName, "%s/data/Run%d_mip.json", cali::CALIROOT, mipRun);
    return getMIP(mipFileName, res);
}
#endif

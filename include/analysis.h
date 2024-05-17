#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>
#include "cali.h"

using namespace std;

struct Ped0 {
    double mean;
    double rms;
};

struct Ped1 {
    struct Ped0 LG;
    struct Ped0 HG;
};

typedef vector<struct Ped1> Ped;

Ped getPedestal(const int pedRun)
{
    char pedFileName[1024];
    sprintf(pedFileName, "%s/data/Run%d_ped.json", cali::CALIROOT, pedRun);
    if (!fileExists(pedFileName))
    {
	cerr << FATAL << "no ped result for ped run: " << pedRun << ", please check it" << endl;
	return {};
    }

    ifstream pedFile(pedFileName);
    stringstream buf;
    buf << pedFile.rdbuf();
    auto ped = nlohmann::json::parse(buf.str());

    Ped res;
    struct Ped1 r1;
    for (int ch=0; ch<cali::nChannels; ch++)
    {
	const char* chName = to_string(ch).c_str();
	r1.LG.mean = ped["LG"][chName][0];
	r1.LG.rms = ped["LG"][chName][1];
	r1.HG.mean = ped["HG"][chName][0];
	r1.HG.rms = ped["HG"][chName][1];
	res.push_back(r1);
    }

    return res;
}

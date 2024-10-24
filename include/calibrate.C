#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include "cali.h"
#include "utilities.h"
#include "db.h"
#include "calibrate.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
	cerr << FATAL << "no run number provided" << endl;
	exit(4);
    }

    const int run = atoi(argv[1]);
    cali::setRun(run);
    caliDB db;

    string runType = db.getRunType(run);
    if (runType != "data" && runType != "cmdata")
    {
	cerr << WARNING << "not a data/cmdata run: " << run << endl;
	exit(1);
    }

    string runFlag = db.getRunFlag(run);
    if (runFlag != "good")
    {
	cerr << WARNING << "not a good run: " << run << endl;
	exit(1);
    }

    const int pedRun = db.getPedRun(run);
    ped_t ped;
    if (!getPedestal(pedRun, ped))
    {
	cerr << FATAL << "unable to read pedestal" << endl;
	exit(2);
    }

    const int mipRun = db.getMIPRun(run);
    mip_t mip;
    if (!getMIP(mipRun, mip))
    {
	cerr << FATAL << "unable to read mip" << endl;
	exit(2);
    }

    string rootFile = cali::getRootFile(run);

    char fdir[1024];
    sprintf(fdir, "%s/figures/%d", cali::CALIROOT, run);
    if (!dirExists(fdir))
	mkdir(fdir, 0755);

    calibrate *cab = new calibrate();
    cab->setRootFile(rootFile.c_str());
    cab->setOutDir(fdir);
    cab->setPed(ped);
    cab->setMIP(mip);
    cab->init();
    cab->fillCorADC();
    cab->fillCorMIP();
    cab->write();
    delete cab;
}

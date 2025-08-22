#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include "cali.h"
#include "utilities.h"
#include "db.h"
#include "QA.h"

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

    char rootFile[1024];
    sprintf(rootFile, "%s/data/Run%d.root", cali::CALIROOT, run);
    if (!fileExists(rootFile))
    {
	sprintf(rootFile, "%s/data/Run%d.root", cali::CALIBACKUP, run);
	if (!fileExists(rootFile))
	{
	    cerr << FATAL << "can't find the root file" << endl;
	    exit(4);
	}
    }

    char fdir[1024];
    sprintf(fdir, "%s/figures/%d/", cali::CALIROOT, run);
    if (!dirExists(fdir))
	mkdir(fdir, 0755);

    QA *qa = new QA();
    qa->setRunType(runType.c_str());
    qa->setRootFile(rootFile);
    qa->setOutDir(fdir);
    qa->setDeltaT(-5*3600); // switch to NY time zone
    qa->init();
    qa->fill();
    qa->plot();
    delete qa;
}

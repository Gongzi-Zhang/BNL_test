#include <string>
#include "utilities.h"
#include "cali.h"
#include "db.h"
#include "MIP.h"

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
    if (runType != "mip")
    {
	cerr << ERROR << "Not a MIP run: " << run << endl;
	exit(4);
    }

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

    char mipOut[1024];
    sprintf(mipOut, "%s/data/Run%d_MIP.json", cali::CALIROOT, run);

    MIPfinder *finder = new MIPfinder();
    finder->setRootFile(rootFile);
    finder->setOutDir(fdir);
    finder->setOutFile(mipOut);
    finder->init();
    finder->readADC();
    // finder->findMIP();
    finder->plot();
    finder->write();

    delete finder;
}

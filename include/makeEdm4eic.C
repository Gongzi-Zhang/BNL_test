#include "makeEdm4eic.h"

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

    char rootFile[1024];
    sprintf(rootFile, "%s/data/Run%d.root", cali::CALIROOT, run);
    if (!fileExists(rootFile))
    {
	sprintf(rootFile, "%s/data/Run%d.root", cali::backupDir, run);
	if (!fileExists(rootFile))
	{
	    cerr << FATAL << "can't find the root file" << endl;
	    exit(4);
	}
    }

    makeEdm4eic *maker = new makeEdm4eic(rootFile, Form("Run%d.edm4eic.root", run));
    maker->make();
    delete maker;
}

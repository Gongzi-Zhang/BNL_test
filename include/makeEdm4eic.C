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

    string rootFile = cali::getRootFile(run);
    const char* outDir = "data";

    makeEdm4eic *maker = new makeEdm4eic(rootFile.c_str(), Form("%s/Run%d.edm4eic.root", outDir, run));
    maker->make();
    delete maker;
}

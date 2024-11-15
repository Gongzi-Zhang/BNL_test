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

    char buf[1024];
    sprintf(buf, "Run%d.root", run);
    string rootFile = cali::getFile(buf);

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%s/data/Run%d.edm4eic.root", cali::CALIROOT, run);

    makeEdm4eic *maker = new makeEdm4eic(rootFile.c_str(), buf);
    maker->make();
    delete maker;
}

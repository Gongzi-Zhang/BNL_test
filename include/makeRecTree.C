#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include "cali.h"
#include "utilities.h"
#include "db.h"
#include "makeRecTree.h"

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

    string rootFile = cali::getRootFile(run);

    makeRecTree *maker = new makeRecTree();
    maker->setInFile(rootFile);
    maker->init();
    maker->make();

    delete maker;
}

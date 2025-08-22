#include <iostream>
#include "utilities.h"
#include "makeMyrec.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
	cerr << FATAL << "no root file provided" << endl;
	exit(4);
    }

    makeRecTree *maker = new makeRecTree();
    maker->setInFile(argv[1]);
    maker->init();
    maker->make();

    delete maker;
}

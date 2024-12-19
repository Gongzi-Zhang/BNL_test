#include <iostream>
#include "utilities.h"
#include "calo.h"
#include "clustering.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
	cerr << FATAL << "1 parameter expected" << endl;
	cerr << INFO << argv[0] << " input" << endl;
	exit(4);
    }

    clustering *cs = new clustering();
    cs->setInput(argv[1]);
    cs->setNeighborX(6*cm);
    cs->setNeighborY(5*cm);
    cs->setNeighborZ(1.3);
    cs->setMinClusterNhits(5);
    cs->setMinClusterCenterE(5);
    cs->setMinClusterHitE(1);
    cs->setMinClusterE(20);
    cs->init();
    cs->process();
    delete cs;
}

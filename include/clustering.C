#include <iostream>
#include "utilities.h"
#include "calo.h"
#include "clustering.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
	cerr << FATAL << "2 parameters expected" << endl;
	cerr << INFO << argv[0] << " input output" << endl;
	exit(4);
    }

    clustering *cs = new clustering();
    cs->setInput(argv[1]);
    cs->setOutput(argv[2]);
    cs->setNeighborX(5*cm);
    cs->setNeighborY(5*cm);
    cs->setNeighborZ(1.3);
    cs->setMinClusterNhits(3);
    cs->setMinClusterCenterE(2);
    cs->setMinClusterHitE(0.5);
    cs->setMinClusterE(5);
    cs->init();
    cs->process();
    delete cs;
}

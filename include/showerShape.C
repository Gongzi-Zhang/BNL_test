#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include "cali.h"
#include "utilities.h"
#include "db.h"
#include "showerShape.h"

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
    if (runType != "data")
    {
	cerr << WARNING << "not a data run: " << run << endl;
	exit(1);
    }

    string runFlag = db.getRunFlag(run);
    if (runFlag != "good")
    {
	cerr << WARNING << "not a good run: " << run << endl;
	exit(1);
    }

    char buf[1024];
    sprintf(buf, "Run%d.rec.root", run);
    string recFile = cali::getFile(buf);

    showerShape *ss = new showerShape();
    ss->setInput(recFile);
    // ss->setOutput(fdir);
    ss->setNeighborX(5*cm);
    ss->setNeighborY(5*cm);
    ss->setNeighborZ(1.3);
    ss->setMinClusterNhits(3);
    ss->setMinClusterCenterE(2);
    ss->setMinClusterHitE(0.5);
    ss->setMinClusterE(5);
    ss->init();
    ss->process();
    delete ss;
}

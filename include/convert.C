#include <iostream>
#include <string>
#include "calo.h"
#include "cali.h"
#include "utilities.h"
#include "db.h"
#include "analysis.h"
#include "makeTree.h"

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
    if (runType != "cosmic" && runType != "data" && runType != "cmdata")
    {
	cout << WARNING << "not a data/cosmic run: " << run << endl;
	exit(1);
    }

    string runFlag = db.getRunFlag(run);
    if (runFlag != "good")
    {
	cerr << WARNING << "not a good run: " << run << endl;
	exit(1);
    }
    char outName[1024];
    sprintf(outName, "%s/data/Run%d.root", cali::CALIROOT, run);

    cout << INFO << "processing run: " << run << endl;

    char listFile[1024];
    sprintf(listFile, "%s/data/Run%d_list.txt", cali::CALIROOT, run);
    if (!fileExists(listFile))
    {
	cerr << FATAL << "no list file for run " << run << endl;
	exit(4);
    }
    listReader* reader = new listReader(listFile);

    if (runType == "data" || runType == "cmdata")
    {
	eventBuilder *builder = new eventBuilder(reader);
	treeMaker *maker = new treeMaker(builder);
	maker->setStartTime(reader->getStartTime());
	maker->setOfName(outName);
	maker->init();
	maker->fill();
	maker->write();

	delete builder;
	delete maker;
    }
    else
    {
	cosmicTreeMaker *maker = new cosmicTreeMaker(reader);
	maker->setStartTime(reader->getStartTime());
	maker->setOfName(outName);
	maker->init();
	maker->fill();
	maker->write();
	delete maker;
    }

    delete reader;
}

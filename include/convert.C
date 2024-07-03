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
    string outName = cali::CALIROOT;
    outName += "/data/Run" + to_string(run) + ".root";

    const int pedRun = db.getPedRun(run);
    pedestal ped;
    if (!getPedestal(pedRun, ped))
    {
	cerr << FATAL << "unable to read pedestal file" << endl;
	exit(2);
    }

    cout << INFO << "processing run: " << run << endl;

    string listFile = Form("%s/data/Run%d_list.txt", cali::CALIROOT, run);
    if (!fileExists(listFile.c_str()))
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
	maker->setPed(ped);
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
	maker->setPed(ped);
	maker->setOfName(outName);
	maker->init();
	maker->fill();
	maker->write();
	delete maker;
    }

    delete reader;
}

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <ctime>
#include "TFile.h"
#include "TTree.h"
#include "calo.h"
#include "utilities.h"
#include "analysis.h"
#include "makeTree.h"

using namespace std;

void usage()
{
    cout << INFO << "usage" << endl;
    cout << "\t./makeHist run_number" << endl;
    cout << "\t./makeHist -i input.root" << endl;
}

int main(int argc, char *argv[])
{
    string nCh_s;
    string listFile;
    string pedFile;
    string outName;
    string mode;
    char opt;
    while ((opt = getopt(argc, argv, "c:i:p:o:m:")) != -1)
	switch(opt)
	{
	    case 'c':
		nCh_s = optarg;
		break;
	    case 'i':
		listFile = optarg;
		break;
	    case 'p':
		pedFile = optarg;
		break;
	    case 'o':
		outName = optarg;
		outName += ".root";
		break;
	    case 'm':
		mode = optarg;
		break;
	    default:
		usage();
		exit(0);
	}

    string ch;
    vector<int> nCh;
    for(stringstream sst(nCh_s); getline(sst, ch, ','); )
	nCh.push_back(stoi(ch));
    calo::setnCAENChannels(nCh);

    pedestal ped;
    if (!getPedestal(pedFile.c_str(), ped))
    {
	cerr << FATAL << "unable to read pedestal file" << endl;
	exit(2);
    }

    if (!fileExists(listFile.c_str()))
    {
	cerr << FATAL << "list file doesn't exist: " << listFile << endl;
	exit(4);
    }
    listReader* reader = new listReader(listFile);

    if (mode == "event")
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

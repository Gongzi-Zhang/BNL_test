#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <map>
#include <vector>
#include "cali.h"
#include "utilities.h"
#include "db.h"
#include "analysis.h"
#include "QA.h"

using namespace std;

void usage()
{
    cout << INFO << "usage" << endl;
    cout << "\t./makeHist run_number" << endl;
    cout << "\t./makeHist -i input.root" << endl;
}

int main(int argc, char *argv[])
{
    bool runMode = true;
    int run = 0;
    string inputFile;

    if (argc == 2)
    {
	run = stoi(argv[1]);
	cout << INFO << "processing run: " << run << endl;
    }
    else
    {
	runMode = false;
	char opt;
	while ((opt = getopt(argc, argv, "i:r:")) != -1)
	    switch (opt)
	    {
		case 'i':
		    inputFile = optarg;
		    break;
		case 'r':
		    run = stoi(optarg);
		    break;
		default:
		    usage();
		    exit(4);
	    }
	if (!run)
	{
	    cerr << ERROR << "no reference run specified." << endl;
	    exit(3);
	}
    }

    cali::setRun(run);
    caliDB db;
    string runType = db.getRunType(run);

    if (runMode)
    {
	bool found = false;
	vector<const char*> dirs = {cali::CALIROOT, cali::backupDir};
	for (const char* dir : dirs)
	{
	    char runFile[1024];
	    sprintf(runFile, "%s/data/Run%d.root", dir, run);
	    if (fileExists(runFile))
	    {
		found = true;
		inputFile = runFile;
		break;
	    }
	}
	if (!found)
	{
	    cerr << ERROR << "no root file found for run " << run << " in the following dirs: " << endl;
	    for (auto dir : dirs)
		cout << "\t" << dir << endl;
	    exit(4);
	}
    }
    
    char fdir[1024];
    if (runMode)
	sprintf(fdir, "%s/figures/%d/", cali::CALIROOT, run);
    else
	sprintf(fdir, "%s/work/tmp/", cali::CALIROOT);
    if (!dirExists(fdir))
	mkdir(fdir, 0755);

    QA *qa = new QA();
    qa->setRunType(runType.c_str());
    qa->setRootFile(inputFile);
    qa->setOutDir(fdir);
    qa->init();
    qa->fill();
    qa->plot();
}

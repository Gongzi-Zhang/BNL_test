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
    vector<string> inputFiles;

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
		    inputFiles.push_back(optarg);
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
	for (const char* dir : {cali::CALIROOT, cali::backupDir})
	{
	    bool found = false;
	    char runFile[1024];
	    sprintf(runFile, "%s/data/Run%d.root", dir, run);
	    if (fileExists(runFile))
	    {
		found = true;
		inputFiles.push_back(runFile);
	    }
	    else
	    {
		int i=1;
		sprintf(runFile, "%s/data/Run%d_%d.root", dir, run, i);
		while (fileExists(runFile))
		{
		    found = true;
		    inputFiles.push_back(runFile);
		    i++;
		    sprintf(runFile, "%s/data/Run%d_%d.root", dir, run, i);
		}
		cout << INFO << "find " << i-1 << " root files for run " << run << endl ;
	    }
	    if (found)
		break;
	    else
		cout << WARNING << "no root file found for run " << run << " in dir " << dir << endl;
	}
    }
    if (!inputFiles.size())
    {
	cerr << FATAL << "no root file found" << endl;
	exit(4);
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
    qa->setRootFiles(inputFiles);
    qa->setOutDir(fdir);
    qa->init();
    qa->fill();
    qa->plot();
}

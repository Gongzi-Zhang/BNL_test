#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include "cali.h"
#include "utilities.h"
#include "QA.h"

using namespace std;

void QA_plots(const char* rootFile, const int run)
{
    gROOT->SetBatch(1);
    cali::setRun(run);

    if (!fileExists(rootFile))
    {
	cerr << FATAL << "can't find the root file" << endl;
	exit(4);
    }

    char fdir[1024];
    sprintf(fdir, "%s/figures/misc/", cali::CALIROOT);
    if (!dirExists(fdir))
	mkdir(fdir, 0755);

    QA *qa = new QA();
    qa->setRootFile(rootFile);
    qa->setOutDir(fdir);
    qa->setDeltaT(-5*3600); // switch to NY time zone
    qa->init();
    qa->fill();
    qa->plot();
    delete qa;
}

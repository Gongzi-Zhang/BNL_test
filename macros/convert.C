#include "makeTree.h"

using namespace std;

void convert(const char* listFile)
{
    gSystem->Load("libmakeTree.so");
    calo::setnCAENChannels({14});
    string outName = "out.root";

    const char* pedFile = "ped.json";
    pedestal ped;
    if (!getPedestal(pedFile, ped))
    {
	cerr << FATAL << "unable to read pedestal file" << endl;
	exit(2);
    }

    if (!fileExists(listFile))
    {
	cerr << FATAL << "list file not found: " << listFile << endl;
	exit(4);
    }
    listReader* reader = new listReader(listFile);

    {
        eventBuilder *builder = new eventBuilder(reader);
        treeMaker *maker = new treeMaker(builder);
        maker->setStartTime(reader->getStartTime());
        maker->setPed(ped);
        maker->setOfName(outName);
        maker->fill();
        maker->write();

        delete builder;
        delete maker;
    }
    // {
    //     cosmicTreeMaker *maker = new cosmicTreeMaker(reader);
    //     maker->setStartTime(reader->getStartTime());
    //     maker->setPed(ped);
    //     maker->setOfName(outName);
    //     cout << DEBUG << "filling events" << endl;
    //     maker->fill();
    //     maker->write();
    //     delete maker;
    // }

    delete reader;
}

#ifndef _MAKERECTREE_
#define _MAKERECTREE_

#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TLeaf.h"
#include "TClonesArray.h"

#include "calo.h"
#include "cali.h"
#include "caliType.h"

using namespace std;

class makeRecTree {
  public:
    makeRecTree() {}
    ~makeRecTree() {}
    void setInFile(string fin) { inFile = fin; }
    void setOutFile(string fout) { outFile = fout; }
    void init();
    void make();

  private:
    string inFile;
    string outFile;

    cali::sipmXY pos[cali::channelMax];
    int layerNumber[cali::channelMax];
};

void makeRecTree::init()
{
    if (outFile.empty())
    {
	outFile = inFile;
	size_t pos = outFile.find(".root");
	outFile.replace(pos, 5, ".rec.root");
    }

    // channel map
    for (int ch=0; ch<cali::nChannels; ch++)
    {
	layerNumber[ch] = cali::getSipm(ch).layer;
	pos[ch] = cali::getSipmXY(ch);
    }
}

void makeRecTree::make()
{
    // input
    TFile *fin = new TFile(inFile.c_str(), "read");
    TTree *tin = (TTree*) fin->Get("mip");
    if (!tin->GetEntries())
    {
	cerr << FATAL << "no entry in the roofile: " << inFile << endl;
	return;
    }

    map<int, float> chMIP;
    for (int ch=0; ch<cali::nChannels; ch++)
    {
	TBranch *b = (TBranch*) tin->GetBranch(Form("ch_%d", ch));
	// read only LG
	chMIP[ch] = 0;
	b->GetLeaf("LG")->SetAddress(&chMIP[ch]);
    }

    // output
    TFile *fout = new TFile(outFile.c_str(), "recreate");
    TTree *tout = new TTree("hit", "CALIHits in MIPs");
    TClonesArray*  hits = new TClonesArray("caliHit");
    tout->Branch("CALIHits", &hits);

    float e;

    for (int ei=0; ei<tin->GetEntries(); ei++)
    {
	tin->GetEntry(ei);
	hits->Clear();
	int nh = 0;
	for (int ch=0; ch<cali::nChannels; ch++)
	{
	    e = chMIP[ch];
	    if (e > 0)
	    {
		new((*hits)[nh]) caliHit(pos[ch].x, pos[ch].y, (float)layerNumber[ch], e);
		nh++;
	    }
	}
	tout->Fill();
    }
    delete tin;
    fin->Close();
    fout->cd();
    tout->Write();
    fout->Close();
}


#endif

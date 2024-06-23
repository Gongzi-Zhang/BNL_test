#ifndef _MAKE_TREE_
#define _MAKE_TREE_

#include <string>
#include <map>
#include "TFile.h"
#include "TTree.h"
#include "cali.h"
#include "analysis.h"
#include "buildEvent.h"

using namespace std;

class makeTree {
  public:
    makeTree() {}
    ~makeTree() {}
    void setPed(const pedestal p) { ped = p; }
    void setBuilder(buildEvent* b) { eb = b; }
    void setStartTime(time_t t) { st = t; }
    void setOfName(string n) { ofName = n; }
    void init();
    void fill(const int n);
    void write();

  private:
    pedestal ped;
    buildEvent *eb = NULL;

    time_t st;
    double TS;
    float rate;

    map<int, pair<int, int>> rawADC;
    map<int, pair<float, float>> corADC;
    pair<int, int> mul, mul1, mul2;

    TFile *fout;
    TTree *traw = NULL;
    TTree *tcor = NULL;
    string ofName;
};

void makeTree::init()
{
    TFile *fout = new TFile(ofName.c_str(), "recreate");
    traw = new TTree("raw", "raw ADC values");
    tcor = new TTree("cor", "corrected ADC values");

    traw->Branch("TS", &TS);
    for (int ch=0; ch<cali::nChannels; ch++)
    {
	rawADC[ch] = {0, 0};
	corADC[ch] = {0, 0};
	traw->Branch(Form("ch_%d", ch), &rawADC[ch], "LG/I:HG/I");
	tcor->Branch(Form("ch_%d", ch), &corADC[ch], "LG/F:HG/F");
    }

    tcor->Branch("rate",  &rate);
    tcor->Branch("mul",   &mul,  "LG/I:HG/I");
    tcor->Branch("mul1",  &mul1, "LG/I:HG/I");
    tcor->Branch("mul2",  &mul2, "LG/I:HG/I");
}

void makeTree::fill(const int n = -1)
{
    if (!eb)
    {
	cerr << ERROR << "No event builder specified." << endl;
	return;
    }
    init();

    int count = 0;
    double preTS = 1e32;
    for (auto &evt : eb->getEvents(n))
    {
	count++;
	mul = {0, 0};
	mul1 = {0, 0};
	mul2 = {0, 0};

	TS = evt->TS + st;
	rate = 1/(TS - preTS);
	preTS = TS;
	for (int ch=0; ch<cali::nChannels; ch++)
	{
	    rawADC[ch].first = evt->LG[ch];
	    rawADC[ch].second = evt->HG[ch];

	    corADC[ch].first = rawADC[ch].first - ped["LG"][ch].mean;
	    corADC[ch].second = rawADC[ch].second - ped["HG"][ch].mean;
	    
	    // LG
	    if (corADC[ch].first < ped["LG"][ch].rms)
		corADC[ch].first = 0;
	    else
	    {
		mul.first++;
		if (corADC[ch].first > 3*ped["LG"][ch].rms)
		    mul1.first++;
		if (corADC[ch].first > 5*ped["LG"][ch].rms)
		    mul2.first++;

	    }
	    // HG
	    if (corADC[ch].second < ped["HG"][ch].rms)
		corADC[ch].second = 0;
	    else
	    {
		mul.second++;
		if (corADC[ch].second > 3*ped["HG"][ch].rms)
		    mul1.second++;
		if (corADC[ch].second > 5*ped["HG"][ch].rms)
		    mul2.second++;
	    }
	}
	traw->Fill();
	tcor->Fill();
	delete evt;
    }
    cout << INFO << count << " events filled" << endl;
}

void makeTree::write()
{
    traw->Write();
    tcor->Write();
    fout->Close();

    delete traw;
    delete tcor;
    delete fout;
}
#endif

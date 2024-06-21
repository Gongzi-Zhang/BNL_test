#ifndef _MAKE_COSMIC_
#define _MAKE_COSMIC_

#include <string>
#include <map>
#include "TFile.h"
#include "TTree.h"
#include "cali.h"
#include "analysis.h"

using namespace std;

class boardReadout {
  public:
    boardReadout(const int bid, const double ts) { id = bid; TS =ts; }
    ~boardReadout() {}
    int getId() const { return id; }
    int getnChannels() const { return nChannels; }
    double getTS() const { return TS; }
    map<int, int> getLG() { return LG; }
    map<int, int> getHG() { return HG; }
    void addChannel(int ch, int vLG, int vHG)
    {
	LG[ch] = vLG;
	HG[ch] = vHG;
	nChannels++;
    }
    void clear()
    {
	LG.clear();
	HG.clear();
	nChannels = 0;
    }

  private:
    int id = -1;
    double TS = -1;
    int nChannels = 0;
    map<int, int> LG;
    map<int, int> HG;
};

class makeCosmic {
  public:
    makeCosmic() {}
    ~makeCosmic() {}
    void setPed(const pedestal p) { ped = p; }
    void setStartTime(time_t t) { st = t; }
    void setOfName(string n) { ofName = n; }
    void addBoard(boardReadout*);
    void init();
    void fill();
    void fill(const int ci, int n = -1);
    void write();

  private:
    pedestal ped;
    string ofName;
    TFile *fout;

    time_t st;
    double TS;
    vector<double> preTS;
    float rate;
    vector<int> count;

    map<int, vector<boardReadout*>> boards;
    map<int, pair<int, int>> rawADC;
    map<int, pair<float, float>> corADC;
    pair<int, int> mul, mul1, mul2;

    vector<TTree *> traw;
    vector<TTree *> tcor;
};

void makeCosmic::init()
{
    fout = new TFile(ofName.c_str(), "recreate");
    for (int ci=0; ci<cali::nCAENs; ci++)
    {
	preTS.push_back(1e32);
	count.push_back(0);

	fout->cd();
	traw.push_back(new TTree(Form("raw_CAEN%d", ci), Form("CAEN%d: raw ADC values", ci)));
	tcor.push_back(new TTree(Form("cor_CAEN%d", ci), Form("CAEN%d: corrected ADC values", ci)));

	traw[ci]->Branch("TS", &TS);
	for (int ch=ci*cali::nCAENChannels; ch<(ci+1)*cali::nCAENChannels; ch++)
	{
	    rawADC[ch] = {0, 0};
	    corADC[ch] = {0, 0};
	    traw[ci]->Branch(Form("ch_%d", ch), &rawADC[ch], "LG/I:HG/I");
	    tcor[ci]->Branch(Form("ch_%d", ch), &corADC[ch], "LG/F:HG/F");
	}

	tcor[ci]->Branch("rate",  &rate);
	tcor[ci]->Branch("mul",   &mul,  "LG/I:HG/I");
	tcor[ci]->Branch("mul1",  &mul1, "LG/I:HG/I");
	tcor[ci]->Branch("mul2",  &mul2, "LG/I:HG/I");
    }
}

void makeCosmic::addBoard(boardReadout *b)
{
    int bid = b->getId();
    if (b->getnChannels() != cali::nCAENChannels && b->getnChannels() != cali::nChannels)
    {
	cerr << WARNING << "bad board record in board " << bid << ", "
	     << b->getnChannels() << "/" << cali::nCAENChannels << "recorded; timestamps: " << b->getTS() << endl;
	return;
    }
    boards[bid].push_back(b);

    if (boards[bid].size() >= 100000)
	fill(bid, 100000);
}

void makeCosmic::fill()
{
    for (int ci=0; ci<cali::nCAENs; ci++)
	fill(ci);
}

void makeCosmic::fill(const int ci, int n)
{
    if (-1 == n || n > boards[ci].size())
	n = boards[ci].size();

    boardReadout* b;
    map<int, int> LG, HG;
    for (int ei=0; ei<n; ei++)
    {
	b = boards[ci][ei];
	LG = b->getLG();
	HG = b->getHG();

	mul = {0, 0};
	mul1 = {0, 0};
	mul2 = {0, 0};

	TS = b->getTS() + st;
	rate = 1/(TS - preTS[ci]);
	preTS[ci] = TS;
	for (int ch=ci*cali::nCAENChannels; ch<(ci+1)*cali::nCAENChannels; ch++)
	{
	    rawADC[ch].first = LG[ch];
	    rawADC[ch].second = HG[ch];

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
	    if (corADC[ch].second > ped["HG"][ch].rms)
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
	traw[ci]->Fill();
	tcor[ci]->Fill();

	delete b;
	b = NULL;
    }

    boards[ci].erase(boards[ci].begin(), boards[ci].begin()+n);
    count[ci] += n;

    cout << INFO << count[ci] << " events filled" << endl;
}

void makeCosmic::write()
{
    for (int ci=0; ci<cali::nCAENs; ci++)
    {
	traw[ci]->Write();
	tcor[ci]->Write();
	delete traw[ci];
	delete tcor[ci];
    }
    fout->Close();
    delete fout;
}
#endif

#ifndef _CALIBRATE_
#define _CALIBRATE_
/* calibrate a run:
 * 1. ped subtraction
 * 2. MIP conversion
 */

#include <map>
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TLeaf.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TF1.h"
#include "TCanvas.h"
#include "calo.h"
#include "cali.h"
#include "analysis.h"

using namespace std;

class calibrate {
  public:
    calibrate() {}
    ~calibrate() {}
    void setRootFile(string fin) { rootFile = fin; }
    void setPed(ped_t p) { ped = p; }
    void setMIP(mip_t m);
    void setOutDir(string f) { fdir = f; }
    void getChPos();
    void getLGMIP();
    void init();
    void fillCorADC();
    void fillCorMIP();
    void fillEvent(map<int, map<string, float>> &val, TTree *t);
    void write();

  private:
    string rootFile;
    string fdir;
    ped_t ped;
    mip_t mip;

    // histograms
    TFile *fio;
    TTree *traw, *tcor, *tmip;
    map<int, TH2F*> h2;
    map<int, map<string, float>> corADC;
    map<int, map<string, float>> corMIP;
    map<string, map<string, float>> values;
    cali::sipmXY pos[cali::channelMax];
    int layerNumber[cali::channelMax];
};

void calibrate::setMIP(mip_t m)
{
    for (int ch = 0; ch<calo::nChannels; ch++)
    {
	if (m.find(ch) == m.end())
	{
	    cerr << WARNING << "channel " << ch << " has no MIP value" << endl;
	    mip[ch]["LG"] = 1e20;
	    mip[ch]["HG"] = 1e20;
	    continue;
	}
	for (const char* gain : calo::gains)
	{
	    if (m[ch].find(gain) == m[ch].end())
	    {
		cerr << WARNING << "channel " << ch << " has no " << gain << " MIP value" << endl;
		mip[ch][gain] = 1e20;
		continue;
	    }
	    mip[ch][gain] = m[ch][gain];
	    if (0 == m[ch][gain])   // set zero MIP value to infinity
		mip[ch][gain] = 1e20;
	}
    }
}

void calibrate::init()
{
    fio = new TFile(rootFile.c_str(), "update");
    traw = (TTree*) fio->Get("raw");
    tcor = new TTree("cor", "corrected ADC values");
    tmip = new TTree("mip", "corrected MIP values");
    for (int ch=0; ch<calo::nChannels; ch++)
    {
	corADC[ch] = {{"LG", 0}, {"HG", 0}};
	corMIP[ch] = {{"LG", 0}, {"HG", 0}};
	tcor->Branch(Form("ch_%d", ch), 0, "LG/F:HG/F");
	tmip->Branch(Form("ch_%d", ch), 0, "LG/F:HG/F");
	for (const char* gain : calo::gains)
	{
	    tcor->GetBranch(Form("ch_%d", ch))->GetLeaf(gain)->SetAddress(&corADC[ch][gain]);
	    tmip->GetBranch(Form("ch_%d", ch))->GetLeaf(gain)->SetAddress(&corMIP[ch][gain]);
	}
	h2[ch] = new TH2F(Form("ch_%d", ch), Form("ch %d", ch), 100, 0, 10, 100, 0, 10);
    }
    for (const char* var : {"mul", "event_e", "event_x", "event_y", "event_z"})
    {
	values[var] = {{"LG", 0}, {"HG", 0}};
	tcor->Branch(var, 0, "LG/F:HG/F");
	tmip->Branch(var, 0, "LG/F:HG/F");
	for (const char* gain : calo::gains)
	{
	    tcor->GetBranch(var)->GetLeaf(gain)->SetAddress(&values[var][gain]);
	    tmip->GetBranch(var)->GetLeaf(gain)->SetAddress(&values[var][gain]);
	}
    }
}

void calibrate::getChPos()
{
    for (int ch=0; ch<cali::nChannels; ch++)
    {
	layerNumber[ch] = cali::getSipm(ch).layer;
	pos[ch] = cali::getSipmXY(ch);
    }
}

void calibrate::fillEvent(map<int, map<string, float>> &val, TTree* t)
{
    for (const char* gain : calo::gains)
    {
	values["event_e"][gain] = 0;
	values["event_x"][gain] = 0;
	values["event_y"][gain] = 0;
	values["event_z"][gain] = 0;
	// for (int l=0; l<cali::nLayers; l++)
	// {
	//     layerMul[l] = 0;
	//     layerADC[l] = 0;
	//     layerX[l] = 0;
	//     layerY[l] = 0;
	// }
	for(int ch=0; ch<calo::nChannels; ch++)
	{
	    int layer = layerNumber[ch];
	    float x = pos[ch].x;
	    float y = pos[ch].y;

	    float e = val[ch][gain];
	    if (e > 0)
	    {
		values["event_e"][gain] += e;
		values["event_x"][gain] += e*x;
		values["event_y"][gain] += e*y;
		values["event_z"][gain] += e*layer;
	    }
	}

	if (values["event_e"][gain])
	{
	    values["event_x"][gain] /= values["event_e"][gain];
	    values["event_y"][gain] /= values["event_e"][gain];
	    values["event_z"][gain] /= values["event_e"][gain];
	}
    }
    t->Fill();
}

void calibrate::fillCorADC()
{
    if (!traw->GetEntries())
    {
	cerr << FATAL << "no entry in the roofile: " << rootFile << endl;
	return;
    }

    map<int, map<string, int>> rawADC;
    for (int ch=0; ch<calo::nChannels; ch++)
    {
	TBranch *b = (TBranch*) traw->GetBranch(Form("ch_%d", ch));
	for (const char*gain : calo::gains)
	{
	    rawADC[ch][gain] = 0;
	    b->GetLeaf(gain)->SetAddress(&rawADC[ch][gain]);
	}
    }

    for (int ei=0; ei<traw->GetEntries(); ei++)
    {
	traw->GetEntry(ei);
	for (int ch=0; ch<calo::nChannels; ch++)
	{
	    for (const char *gain : calo::gains)
	    {
		corADC[ch][gain] = rawADC[ch][gain] - ped[ch][gain].mean;
		if (corADC[ch][gain] < 5*ped[ch][gain].rms)
		    corADC[ch][gain] = 0;
	    }
	    h2[ch]->Fill(corADC[ch]["LG"], corADC[ch]["HG"]);
	}
	fillEvent(corADC, tcor);
    }
}

void calibrate::getLGMIP()
{
    int ncol = 8;
    int nrow = 1 + (calo::nChannels - 1)/ncol;
    TCanvas *c = new TCanvas("c", "c", ncol*600, nrow*600);
    c->Divide(ncol, nrow, 0, 0);
    for (int ch=0; ch<calo::nChannels; ch++)
    {
	c->cd(ch+1);
	h2[ch]->Draw("COLZ");
	TProfile * proX = h2[ch]->ProfileX(Form("ch%d_profileX", ch));
	TF1 *fit = new TF1(Form("ch%d_fit", ch), "[0] + [1]*x", 0, 1000);
	fit->SetParameters(0, 30);
	proX->Fit(fit, "q");
	mip[ch]["LG"] = mip[ch]["HG"] / fit->GetParameter(1);
    }
    c->SaveAs(Form("%s/LG_vs_HG.png", fdir.c_str()));
}

void calibrate::fillCorMIP()
{
    getLGMIP();
    for (int ei=0; ei<tcor->GetEntries(); ei++)
    {
	tcor->GetEntry(ei);
	for (int ch=0; ch<calo::nChannels; ch++)
	{
	    for (const char *gain : calo::gains)
	    {
		corMIP[ch][gain] = corADC[ch][gain] / mip[ch][gain];
		if (corMIP[ch][gain] < 0.3*mip[ch][gain])
		    corMIP[ch][gain] = 0;
	    }
	}
	fillEvent(corMIP, tmip);
    }
}

void calibrate::write()
{
    fio->cd();
    tcor->Write();
    tmip->Write();
    fio->Close();
}
#endif

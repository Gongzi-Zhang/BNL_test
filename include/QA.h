#ifndef _QA_
#define _QA_

#include "TError.h"
#include "TFile.h"
#include "TChain.h"
#include "TLeaf.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TGaxis.h"
#include "TLatex.h"
#include "cali.h"

using namespace std;

void reverseXAxis(TH1 *h)
{
   // Remove the current axis
   h->GetXaxis()->SetLabelOffset(999);
   h->GetXaxis()->SetTickLength(0);

   // Redraw the new axis
   gPad->Update();
   TGaxis *newaxis = new TGaxis(gPad->GetUxmax(),
                                gPad->GetUymin(),
                                gPad->GetUxmin(),
                                gPad->GetUymin(),
                                h->GetXaxis()->GetXmin(),
                                h->GetXaxis()->GetXmax(),
                                510,"-");
   newaxis->SetLabelOffset(-0.03);
   newaxis->Draw();
}

class QA {
  public:
    QA() {}
    ~QA() {}
    void setRunType(string rt) { runType = rt; }
    void setRootFiles(vector<string> fin) { rootFiles = fin; }
    void setOutDir(string d) { fdir = d; }
    void init();
    void fill();
    void plot();

  private:
    string runType = "data";
    vector<string> rootFiles;
    string fdir;

    // histograms
    map<string, TH1F*[2]> h1;
    map<string, TH2F*[2]> h2;
    map<string, map<int, TH1F*[2]>> h1Layer;
    map<string, map<int, TH1F*[2]>> h1Channel;
};

void QA::init()
{
    map<string, double[2]> xmax;
    xmax["hitADC"][0]	= 8500; xmax["hitADC"][1]   = 8500;
    xmax["eventADC"][0]	= 1e5;	xmax["eventADC"][1] = 6e5;
    xmax["eventMul"][0]	= 200;	xmax["eventMul"][1] = 200;
    xmax["eventMul1"][0] = 200; xmax["eventMul1"][1] = 200;
    xmax["eventMul2"][0] = 200; xmax["eventMul2"][1] = 200;
    xmax["chRawADC"][0]	= 2000; xmax["chRawADC"][1]  = 8500;
    xmax["chCorADC"][0]	= 2000; xmax["chCorADC"][1]  = 8500;

    if (runType == "cosmic")
    {
	xmax["hitADC"][0] = 1000;	xmax["hitADC"][1] = 1000;
	xmax["eventADC"][0] = 1000;	xmax["eventADC"][1] = 1000;
	xmax["eventMul"][0] = 100;	xmax["eventMul"][1] = 100;
	xmax["eventMul1"][0] = 20;	xmax["eventMul1"][1] = 20;
	xmax["eventMul2"][0] = 20;	xmax["eventMul2"][1] = 20;
	xmax["chRawADC"][0] = 400;	xmax["chRawADC"][1] = 400;
	xmax["chCorADC"][0] = 200;	xmax["chCorADC"][1] = 200;
    }
    for (int i=0; i<2; i++) // gain
    {
	const char* gain = (0 == i) ? "LG" : "HG";

	h1["hitADC"][i] = new TH1F(Form("hitADC_%s", gain), Form("%s: hit energy;ADC", gain), 100, 0, xmax["hitADC"][i]);

	h1["eventRate"][i]  = new TH1F(Form("eventRate_%s", gain),  Form("%s: rate;Time", gain), 100, 0, 10);	// needs update later
	h1["eventMul"][i]   = new TH1F(Form("eventMul_%s",  gain),  Form("%s: mul", gain), xmax["eventMul"][i], 0, xmax["eventMul"][i]);
	h1["eventMul1"][i]  = new TH1F(Form("eventMul1_%s", gain),  Form("%s: mul (3#sigma)", gain), xmax["eventMul1"][i], 0, xmax["eventMul1"][i]);
	h1["eventMul2"][i]  = new TH1F(Form("eventMul2_%s", gain),  Form("%s: mul (5#sigma)", gain), xmax["eventMul2"][i], 0, xmax["eventMul2"][i]);
	h1["eventADC"][i] = new TH1F(Form("eventADC_%s", gain),  Form("%s: event energy;ADC", gain), 100, 0, xmax["eventADC"][i]);
	h1["eventX"][i] = new TH1F(Form("eventX_%s", gain),  Form("%s: COG X;cm", gain), 100, -10, 10);
	h1["eventY"][i] = new TH1F(Form("eventY_%s", gain),  Form("%s: COG Y;cm", gain), 100, -10, 10);
	h1["eventZ"][i] = new TH1F(Form("eventZ_%s", gain),  Form("%s: COG Z;layer", gain), 100, 0, cali::nLayers);

	for (int l=0; l<cali::nLayers; l++)
	{
	    h1Layer["mul"][l][i] = new TH1F(Form("layerMul_%d_%s", l, gain), Form("%s: layer %d", gain, l), 100, 0, 100);
	    h1Layer["ADC"][l][i] = new TH1F(Form("layerADC_%d_%s", l, gain),   Form("%s: layer %d", gain, l), 100, 0, 1000);
	    h1Layer["X"][l][i] = new TH1F(Form("layerX_%d_%s", l, gain),   Form("%s: layer %d", gain, l), 100, -10, 10);
	    h1Layer["Y"][l][i] = new TH1F(Form("layerY_%d_%s", l, gain),   Form("%s: layer %d", gain, l), 100, -10, 10);
	}

	for (int ch=0; ch<cali::nChannels; ch++)
	{
	    h1Channel["raw"][ch][i] = new TH1F(Form("chRaw_%d_%s", ch, gain), Form("%s: Ch %d", gain, ch), 100, 0, xmax["chRawADC"][i]);
	    h1Channel["cor"][ch][i] = new TH1F(Form("chCor_%d_%s", ch, gain), Form("%s: Ch %d", gain, ch), 100, -100, xmax["chCorADC"][i]);
	}

	h2["hitXY"][i] = new TH2F(Form("hitXY_%s", gain), Form("%s: hit xy;x(cm);y(cm)", gain), 100, -10, 10, 100, -10, 10);

	// formats
	h1["eventRate"][i]->GetXaxis()->SetTimeDisplay(1);
	// h1["eventRate"][i]->GetXaxis()->SetTimeOffset(0, "gmt");
	h1["eventRate"][i]->GetXaxis()->SetTimeFormat("%H:%M");
	h1["eventRate"][i]->SetStats(false);

	h2["hitXY"][i]->SetStats(false);
    }
}

void QA::fill()
{
    cerr << INFO << "filling histograms" << endl;
    TChain *t = new TChain("cor");
    for (string rf : rootFiles)
	t->AddFile(rf.c_str());
    if (!t->GetEntries())
    {
	cerr << FATAL << "no entry in the roofile: " << rootFiles[0] << endl;
	return;
    }
    t->AddFriend("raw");

    double TS;
    float rate;
    map<string, int[2]> iVal;	// int value
    map<int, int[2]> chRawADC;
    map<int, float[2]> chCorADC;

    t->SetBranchAddress("raw.TS", &TS);
    t->SetBranchAddress("rate", &rate);
    for (const char* var : {"mul", "mul1", "mul2"})
    {
	TBranch *b = (TBranch*) t->GetBranch(var);
	b->GetLeaf("LG")->SetAddress(iVal[var]);
	b->GetLeaf("HG")->SetAddress(iVal[var] + 1);
    }
    for (int ch=0; ch<cali::nChannels; ch++)
    {
	TBranch *braw = (TBranch*) t->GetBranch(Form("raw.ch_%d", ch));
	braw->GetLeaf("LG")->SetAddress(chRawADC[ch]);
	braw->GetLeaf("HG")->SetAddress(chRawADC[ch] + 1);
	TBranch *bcor = (TBranch*) t->GetBranch(Form("ch_%d", ch));
	bcor->GetLeaf("LG")->SetAddress(chCorADC[ch]);
	bcor->GetLeaf("HG")->SetAddress(chCorADC[ch] + 1);
    }

    t->GetEntry(0);
    double startTS = TS - 10;
    t->GetEntry(t->GetEntries() - 1);
    double endTS = TS + 10;
    for (int i=0; i<2; i++)
	h1["eventRate"][i]->GetXaxis()->SetRangeUser(startTS, endTS);

    cali::sipmXY pos[cali::channelMax];
    int layerNumber[cali::channelMax];
    for (int ch=0; ch<cali::nChannels; ch++)
    {
	layerNumber[ch] = cali::getSipm(ch).layer;
	pos[ch] = cali::getSipmXY(ch);
    }

    int layer;
    float x, y;

    float eventADC[2], eventX[2], eventY[2], eventZ[2];
    map<int, int[2]> layerMul;
    map<int, float[2]> layerADC;
    map<int, float[2]> layerX;
    map<int, float[2]> layerY;

    for (int ei=0; ei<t->GetEntries(); ei++)
    {
	t->GetEntry(ei);

	for(int i=0; i<2; i++)
	{
	    eventADC[i] = 0;
	    eventX[i] = 0;
	    eventY[i] = 0;
	    eventZ[i] = 0;

	    for (int l=0; l<cali::nLayers; l++)
	    {
		layerMul[l][i] = 0;
		layerADC[l][i] = 0;
		layerX[l][i] = 0;
		layerY[l][i] = 0;
	    }

	    h1["eventRate"][i]->Fill(TS, rate);
	    h1["eventMul"][i]->Fill(iVal["mul"][i]);
	    h1["eventMul1"][i]->Fill(iVal["mul1"][i]);
	    h1["eventMul2"][i]->Fill(iVal["mul2"][i]);

	    for (int ch=0; ch<cali::nChannels; ch++)
	    {
		layer = layerNumber[ch];
		x = pos[ch].x/cm;
		y = pos[ch].y/cm;

		h1Channel["raw"][ch][i]->Fill(chRawADC[ch][i]);

		float adc = chCorADC[ch][i];
		if (adc > 0)
		{
		    h1["hitADC"][i]->Fill(adc);
		    h2["hitXY"][i]->Fill(x, y);
		    h1Channel["cor"][ch][i]->Fill(adc);
		    layerMul[layer][i]++;
		    layerADC[layer][i] += adc;
		    layerX[layer][i] += adc*x;
		    layerY[layer][i] += adc*y;

		    eventADC[i] += adc;
		    eventX[i] += adc*x;
		    eventY[i] += adc*y;
		    eventZ[i] += adc*layer;
		}
	    }

	    if (eventADC[i])
	    {
		eventX[i] /= eventADC[i];
		eventY[i] /= eventADC[i];
		eventZ[i] /= eventADC[i];
		h1["eventADC"][i]->Fill(eventADC[i]);
		h1["eventX"][i]->Fill(eventX[i]);
		h1["eventY"][i]->Fill(eventY[i]);
		h1["eventZ"][i]->Fill(eventZ[i]);
	    }
	    for (int l=0; l<cali::nLayers; l++)
	    {
		h1Layer["mul"][l][i]->Fill(layerMul[l][i]);
		h1Layer["ADC"][l][i]->Fill(layerADC[l][i]);
		if (layerADC[l][i])
		{
		    layerX[l][i] /= layerADC[l][i];
		    layerY[l][i] /= layerADC[l][i];
		    h1Layer["X"][l][i]->Fill(layerX[l][i]);
		    h1Layer["Y"][l][i]->Fill(layerY[l][i]);
		}
	    }
	}
    }
    delete t;
}

void QA::plot()
{
    cout << INFO << "producing plots" << endl;
    gErrorIgnoreLevel = kWarning;
    for (int i=0; i<2; i++)
    {
	TCanvas* c = new TCanvas("c", "c", 1000, 600);
	const char* gain = (0 == i) ? "LG" : "HG";
	for (auto const & x : h1)
	{
	    if (x.first == "eventADC" || x.first == "hitADC")
		c->SetLogy(1);
	    x.second[i]->Draw("HIST");
	    c->SaveAs(Form("%s/%s_%s.png", fdir.c_str(), gain, x.first.c_str()));
	    c->SetLogy(0);
	    delete x.second[i];
	}
	for (auto const & x : h2)
	{
	    x.second[i]->Draw("text");
	    reverseXAxis(x.second[i]);
	    c->SaveAs(Form("%s/%s_%s.png", fdir.c_str(), gain, x.first.c_str()));
	    delete x.second[i];
	}
	delete c;

	// layer plot
	TCanvas* c1 = new TCanvas("c1", "c1", 800, 600);
	for (auto const & x : h1Layer)
	{
	    if (x.first == "ADC")
		c1->SetLogy();
	    
	    for (int l=0; l<cali::nLayers; l++)
	    {
		TH1F *h = h1Layer[x.first][l][i];
		h->Draw();
		c1->SaveAs(Form("%s/%s_layer%d_%s.png", fdir.c_str(), gain, l, x.first.c_str()));
		delete h;
	    }
	    c1->SetLogy(0);
	}
	delete c1;

	// channel plot
	TCanvas* c2 = new TCanvas("c2", "c2", 800, 600);
	for (auto const & x : h1Channel)
	{
	    for (int cn=0; cn<cali::nCAENs; cn++)
	    {
		c2->Clear();
		c2->SetTopMargin(0.08);
		c2->SetBottomMargin(0.08);
		c2->SetLeftMargin(0.06);
		c2->SetRightMargin(0.05);
		for (int ch=0; ch<cali::nCAENChannels; ch++)
		{
		    int gCh = ch + cn*cali::nCAENChannels;
		    if (gCh >= cali::nChannels)
			continue;
		    c2->SetLogy(1);
		    TH1F *h = h1Channel[x.first][gCh][i];
		    h->SetStats(false);
		    h->Draw();
		    c2->SaveAs(Form("%s/%s_ch%d_%s.png", fdir.c_str(), gain, gCh, x.first.c_str()));
		    delete h;
		}
	    }
	}
	delete c2;
    }
}
#endif

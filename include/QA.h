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
#include "calo.h"
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
    void setRootFile(string fin) { rootFile = fin; }
    void setOutDir(string d) { fdir = d; }
    void setDeltaT(time_t d) { deltaT = d; }
    void init();
    void fill();
    void fillData();
    void fillCosmic();
    void plot();

  private:
    string runType = "data";
    string rootFile;
    string fdir;
    time_t deltaT = 5*3600;

    // histograms
    map<string, map<string, TH1F*>> h1;
    map<string, map<string, TH2F*>> h2;
    map<int, map<string, map<string, TH1F*>>> h1Layer;
    map<int, map<string, map<string, TH1F*>>> h1Channel;
};

void QA::init()
{
    map<string, map<string, double>> xmax;
    xmax["hit_ADC"]["LG"]    = 8500;   xmax["hit_ADC"]["HG"]   = 8500;
    xmax["event_ADC"]["LG"]  = 1e5;    xmax["event_ADC"]["HG"] = 6e5;
    xmax["event_mul"]["LG"]  = 200;    xmax["event_mul"]["HG"] = 200;
    xmax["event_mul1"]["LG"] = 200;    xmax["event_mul1"]["HG"] = 200;
    xmax["event_mul2"]["LG"] = 200;    xmax["event_mul2"]["HG"] = 200;
    xmax["ch_raw_ADC"]["LG"] = 2000;   xmax["ch_raw_ADC"]["HG"]  = 8500;
    xmax["ch_cor_ADC"]["LG"] = 2000;   xmax["ch_cor_ADC"]["HG"]  = 8500;

    if (runType == "cosmic")
    {
	xmax["hit_ADC"]["LG"] = 1000;	xmax["hit_ADC"]["HG"] = 1000;
	xmax["event_ADC"]["LG"] = 1000;	xmax["event_ADC"]["HG"] = 1000;
	xmax["event_mul"]["LG"] = 100;	xmax["event_mul"]["HG"] = 100;
	xmax["event_mul1"]["LG"] = 20;	xmax["event_mul1"]["HG"] = 20;
	xmax["event_mul2"]["LG"] = 20;	xmax["event_mul2"]["HG"] = 20;
	xmax["ch_raw_ADC"]["LG"] = 400;	xmax["ch_raw_ADC"]["HG"] = 400;
	xmax["ch_cor_ADC"]["LG"] = 200;	xmax["ch_cor_ADC"]["HG"] = 200;
    }
    for (auto const &gain : calo::gains) // gain
    {
	h1["hit_ADC"][gain] = new TH1F(Form("hit_ADC_%s", gain), Form("%s: hit energy;ADC", gain), 100, 0, xmax["hit_ADC"][gain]);

	h1["event_rate"][gain]  = new TH1F(Form("event_rate_%s", gain),  Form("%s: rate;Time", gain), 100, 1e8, 1e10);	// needs update later
	h1["event_mul"][gain]   = new TH1F(Form("event_mul_%s",  gain),  Form("%s: mul", gain), xmax["event_mul"][gain], 0, xmax["event_mul"][gain]);
	h1["event_mul1"][gain]  = new TH1F(Form("event_mul1_%s", gain),  Form("%s: mul (3#sigma)", gain), xmax["event_mul1"][gain], 0, xmax["event_mul1"][gain]);
	h1["event_mul2"][gain]  = new TH1F(Form("event_mul2_%s", gain),  Form("%s: mul (5#sigma)", gain), xmax["event_mul2"][gain], 0, xmax["event_mul2"][gain]);
	h1["event_ADC"][gain] = new TH1F(Form("event_ADC_%s", gain),  Form("%s: event energy;ADC", gain), 100, 0, xmax["event_ADC"][gain]);
	h1["event_x"][gain] = new TH1F(Form("event_x_%s", gain),  Form("%s: COG X;cm", gain), 100, -10, 10);
	h1["event_y"][gain] = new TH1F(Form("event_y_%s", gain),  Form("%s: COG Y;cm", gain), 100, -10, 10);
	h1["event_z"][gain] = new TH1F(Form("event_z_%s", gain),  Form("%s: COG Z;layer", gain), 100, 0, cali::nLayers);

	for (int l=0; l<cali::nLayers; l++)
	{
	    h1Layer[l]["mul"][gain] = new TH1F(Form("layer%d_mul_%s", l, gain), Form("%s: layer %d", gain, l), 100, 0, 100);
	    h1Layer[l]["ADC"][gain] = new TH1F(Form("layer%d_ADC_%s", l, gain), Form("%s: layer %d;ADC", gain, l), 100, 0, 1000);
	    h1Layer[l]["x"][gain] = new TH1F(Form("layer%d_x_%s", l, gain),   Form("%s: layer %d;cm", gain, l), 100, -10, 10);
	    h1Layer[l]["y"][gain] = new TH1F(Form("layer%d_y_%s", l, gain),   Form("%s: layer %d;cm", gain, l), 100, -10, 10);
	}

	for (int ch=0; ch<calo::nChannels; ch++)
	{
	    h1Channel[ch]["raw"][gain] = new TH1F(Form("ch%d_raw_%s", ch, gain), Form("%s: Ch %d;ADC", gain, ch), 100, 0, xmax["ch_raw_ADC"][gain]);
	    h1Channel[ch]["cor"][gain] = new TH1F(Form("ch%d_cor_%s", ch, gain), Form("%s: Ch %d;ADC", gain, ch), 100, -100, xmax["ch_cor_ADC"][gain]);
	}

	h2["hit_xy"][gain] = new TH2F(Form("hit_xy_%s", gain), Form("%s: hit xy;x(cm);y(cm)", gain), 100, -10, 10, 100, -10, 10);

	// formats
	h1["event_rate"][gain]->GetXaxis()->SetTimeDisplay(1);
	// h1["event_rate"][gain]->GetXaxis()->SetTimeOffset(0, "gmt");
	h1["event_rate"][gain]->GetXaxis()->SetTimeFormat("%H:%M");
	h1["event_rate"][gain]->SetStats(false);

	h2["hit_xy"][gain]->SetStats(false);
    }
}

void QA::fill()
{
    cout << INFO << "filling histograms" << endl;
    if (runType == "data" || runType == "cmdata")
	fillData();
    else if (runType == "cosmic")
	fillCosmic();
}

void QA::fillData()
{
    TFile *fin = new TFile(rootFile.c_str(), "read");
    TTree *t = (TTree*) fin->Get("cor");
    t->AddFriend("raw");
    if (!t->GetEntries())
    {
	cerr << FATAL << "no entry in the roofile: " << rootFile << endl;
	return;
    }

    double TS;
    float rate;
    map<string, map<string, int>> iVal;	// int value
    map<int, map<string, int>> chRawADC;
    map<int, map<string, float>> chCorADC;

    t->SetBranchAddress("raw.TS", &TS);
    t->SetBranchAddress("rate", &rate);
    for (const char* var : {"mul", "mul1", "mul2"})
    {
	iVal[var] = {{"LG", 0}, {"HG", 0}};
	TBranch *b = (TBranch*) t->GetBranch(var);
	b->GetLeaf("LG")->SetAddress(&iVal[var]["LG"]);
	b->GetLeaf("HG")->SetAddress(&iVal[var]["HG"]);
    }
    for (int ch=0; ch<cali::nChannels; ch++)
    {
	chRawADC[ch] = {{"LG", 0}, {"HG", 0}};
	TBranch *braw = (TBranch*) t->GetBranch(Form("raw.ch_%d", ch));
	braw->GetLeaf("LG")->SetAddress(&chRawADC[ch]["LG"]);
	braw->GetLeaf("HG")->SetAddress(&chRawADC[ch]["HG"]);
	chCorADC[ch] = {{"LG", 0.}, {"HG", 0.}};
	TBranch *bcor = (TBranch*) t->GetBranch(Form("ch_%d", ch));
	bcor->GetLeaf("LG")->SetAddress(&chCorADC[ch]["LG"]);
	bcor->GetLeaf("HG")->SetAddress(&chCorADC[ch]["HG"]);
    }

    t->GetEntry(0);
    double startTS = TS - 10;
    t->GetEntry(t->GetEntries() - 1);
    double endTS = TS + 10;
    for (auto const& gain : calo::gains)
        h1["event_rate"][gain]->GetXaxis()->SetLimits(startTS, endTS);

    cali::sipmXY pos[cali::channelMax];
    int layerNumber[cali::channelMax];
    for (int ch=0; ch<cali::nChannels; ch++)
    {
	layerNumber[ch] = cali::getSipm(ch).layer;
	pos[ch] = cali::getSipmXY(ch);
    }

    int layer;
    float x, y;

    float eventADC, eventX, eventY, eventZ;
    map<int, int> layerMul;
    map<int, float> layerADC;
    map<int, float> layerX;
    map<int, float> layerY;

    for (int ei=0; ei<t->GetEntries(); ei++)
    {
	t->GetEntry(ei);

	for(auto const & gain : calo::gains)
	{
	    eventADC = 0;
	    eventX = 0;
	    eventY = 0;
	    eventZ = 0;

	    for (int l=0; l<cali::nLayers; l++)
	    {
		layerMul[l] = 0;
		layerADC[l] = 0;
		layerX[l] = 0;
		layerY[l] = 0;
	    }

	    h1["event_rate"][gain]->Fill(TS, rate);
	    h1["event_mul"][gain]->Fill(iVal["mul"][gain]);
	    h1["event_mul1"][gain]->Fill(iVal["mul1"][gain]);
	    h1["event_mul2"][gain]->Fill(iVal["mul2"][gain]);

	    for (int ch=0; ch<cali::nChannels; ch++)
	    {
		layer = layerNumber[ch];
		x = pos[ch].x;
		y = pos[ch].y;

		h1Channel[ch]["raw"][gain]->Fill(chRawADC[ch][gain]);

		float adc = chCorADC[ch][gain];
		if (adc > 0)
		{
		    h1["hit_ADC"][gain]->Fill(adc);
		    h2["hit_xy"][gain]->Fill(x/cm, y/cm);
		    h1Channel[ch]["cor"][gain]->Fill(adc);
		    layerMul[layer]++;
		    layerADC[layer] += adc;
		    layerX[layer] += adc*x;
		    layerY[layer] += adc*y;

		    eventADC += adc;
		    eventX += adc*x;
		    eventY += adc*y;
		    eventZ += adc*layer;
		}
	    }

	    if (eventADC)
	    {
		eventX /= eventADC;
		eventY /= eventADC;
		eventZ /= eventADC;
		h1["event_ADC"][gain]->Fill(eventADC);
		h1["event_x"][gain]->Fill(eventX/cm);
		h1["event_y"][gain]->Fill(eventY/cm);
		h1["event_z"][gain]->Fill(eventZ);
	    }
	    for (int l=0; l<cali::nLayers; l++)
	    {
		h1Layer[l]["mul"][gain]->Fill(layerMul[l]);
		h1Layer[l]["ADC"][gain]->Fill(layerADC[l]);
		if (layerADC[l])
		{
		    layerX[l] /= layerADC[l];
		    layerY[l] /= layerADC[l];
		    h1Layer[l]["x"][gain]->Fill(layerX[l]/cm);
		    h1Layer[l]["y"][gain]->Fill(layerY[l]/cm);
		}
	    }
	}
    }
    delete t;
}

void QA::fillCosmic()
{
    TFile *fin = new TFile(rootFile.c_str(), "read");

    double TS;
    float rate;
    map<string, map<string, int>> iVal;	// int value
    map<int, map<string, int>> chRawADC;
    map<int, map<string, float>> chCorADC;

    for (int ci=0; ci<calo::nCAENs; ci++)
    {
	TTree *t = (TTree*) fin->Get(Form("cor_CAEN%d", ci));
	t->AddFriend(Form("raw_CAEN%d", ci));
	if (!t->GetEntries())
	{
	    cerr << FATAL << "no entry for CAEN" << ci << " in the roofile: " << rootFile << endl;
	    continue;
	}

	t->SetBranchAddress(Form("raw_CAEN%d.TS", ci), &TS);
	t->SetBranchAddress("rate", &rate);
	for (const char* var : {"mul", "mul1", "mul2"})
	{
	    iVal[var] = {{"LG", 0}, {"HG", 0}};
	    TBranch *b = (TBranch*) t->GetBranch(var);
	    b->GetLeaf("LG")->SetAddress(&iVal[var]["LG"]);
	    b->GetLeaf("HG")->SetAddress(&iVal[var]["HG"]);
	}
	for (int ch=0; ch<calo::nCAENChannels[ci]; ch++)    
	{
	    chRawADC[ch] = {{"LG", 0}, {"HG", 0}};
	    TBranch *braw = (TBranch*) t->GetBranch(Form("raw_CAEN%d.ch_%d", ci, ch));
	    braw->GetLeaf("LG")->SetAddress(&chRawADC[ch]["LG"]);
	    braw->GetLeaf("HG")->SetAddress(&chRawADC[ch]["HG"]);
	    chCorADC[ch] = {{"LG", 0.}, {"HG", 0.}};
	    TBranch *bcor = (TBranch*) t->GetBranch(Form("ch_%d", ch));
	    bcor->GetLeaf("LG")->SetAddress(&chCorADC[ch]["LG"]);
	    bcor->GetLeaf("HG")->SetAddress(&chCorADC[ch]["HG"]);
	}

	t->GetEntry(0);
	double startTS = TS - 10;
	t->GetEntry(t->GetEntries() - 1);
	double endTS = TS + 10;
	for (auto const &gain : calo::gains)
	    h1["event_rate"][gain]->GetXaxis()->SetLimits(startTS, endTS);

	cali::sipmXY pos[cali::channelMax];
	int layerNumber[cali::channelMax];
	for (int ch=0; ch<cali::nChannels; ch++)
	{
	    layerNumber[ch] = cali::getSipm(ch).layer;
	    pos[ch] = cali::getSipmXY(ch);
	}

	int layer;
	float x, y;

	float eventADC, eventX, eventY, eventZ;
	map<int, int> layerMul;
	map<int, float> layerADC;
	map<int, float> layerX;
	map<int, float> layerY;

	for (int ei=0; ei<t->GetEntries(); ei++)
	{
	    t->GetEntry(ei);

	    for (auto const &gain : calo::gains)
	    {
		eventADC = 0;
		eventX = 0;
		eventY = 0;
		eventZ = 0;

		for (int l=0; l<cali::nLayers; l++)
		{
		    layerMul[l] = 0;
		    layerADC[l] = 0;
		    layerX[l] = 0;
		    layerY[l] = 0;
		}

		h1["event_rate"][gain]->Fill(TS, rate);
		h1["event_mul"][gain]->Fill(iVal["mul"][gain]);
		h1["event_mul1"][gain]->Fill(iVal["mul1"][gain]);
		h1["event_mul2"][gain]->Fill(iVal["mul2"][gain]);

		for (int ch=0; ch<cali::nChannels; ch++)
		{
		    layer = layerNumber[ch];
		    x = pos[ch].x/cm;
		    y = pos[ch].y/cm;

		    h1Channel[ch]["raw"][gain]->Fill(chRawADC[ch][gain]);

		    float adc = chCorADC[ch][gain];
		    if (adc > 0)
		    {
			h1["hit_ADC"][gain]->Fill(adc);
			h2["hit_xy"][gain]->Fill(x, y);
			h1Channel[ch]["cor"][gain]->Fill(adc);
			layerMul[layer]++;
			layerADC[layer] += adc;
			layerX[layer] += adc*x;
			layerY[layer] += adc*y;

			eventADC += adc;
			eventX += adc*x;
			eventY += adc*y;
			eventZ += adc*layer;
		    }
		}

		if (eventADC)
		{
		    eventX /= eventADC;
		    eventY /= eventADC;
		    eventZ /= eventADC;
		    h1["event_ADC"][gain]->Fill(eventADC);
		    h1["event_x"][gain]->Fill(eventX);
		    h1["event_y"][gain]->Fill(eventY);
		    h1["event_z"][gain]->Fill(eventZ);
		}
		for (int l=0; l<cali::nLayers; l++)
		{
		    h1Layer[l]["mul"][gain]->Fill(layerMul[l]);
		    h1Layer[l]["ADC"][gain]->Fill(layerADC[l]);
		    if (layerADC[l])
		    {
			layerX[l] /= layerADC[l];
			layerY[l] /= layerADC[l];
			h1Layer[l]["x"][gain]->Fill(layerX[l]);
			h1Layer[l]["y"][gain]->Fill(layerY[l]);
		    }
		}
	    }
	}
	delete t;
    }
}

void QA::plot()
{
    cout << INFO << "making plots" << endl;
    gErrorIgnoreLevel = kWarning;
    for (auto const & gain : calo::gains)
    {
	TCanvas* c = new TCanvas("c", "c", 1000, 600);
	for (auto const & x : h1)
	{
	    c->Clear();
	    string var = x.first;
	    if (var == "event_ADC" || var == "hit_ADC")
		c->SetLogy(1);
	    h1[var][gain]->Draw("HIST");
	    c->SaveAs(Form("%s/%s_%s.png", fdir.c_str(), gain, var.c_str()));
	    delete h1[var][gain];
	}
	for (auto const & x : h2)
	{
	    string var = x.first;
	    h2[var][gain]->Draw("text");
	    reverseXAxis(h2[var][gain]);
	    c->SaveAs(Form("%s/%s_%s.png", fdir.c_str(), gain, var.c_str()));
	}
	delete c;

	// layer plot
	TCanvas* c1 = new TCanvas("c1", "c1", 800, 600);
	for (int l=0; l<cali::nLayers; l++)
	{
	    for (auto const & x : h1Layer[l])
	    {
		string var = x.first;
		if (var == "ADC")
		    c1->SetLogy();
	    
		h1Layer[l][var][gain]->Draw();
		c1->SaveAs(Form("%s/%s_layer%d_%s.png", fdir.c_str(), gain, l, var.c_str()));
	    }
	    c1->SetLogy(0);
	}
	delete c1;

	// channel plot
	TCanvas* c2 = new TCanvas("c2", "c2", 800, 600);
	c2->SetTopMargin(0.08);
	c2->SetBottomMargin(0.08);
	c2->SetLeftMargin(0.06);
	c2->SetRightMargin(0.05);
	for (int ch=0; ch<calo::nChannels; ch++)
	{
	    for (auto const & x : h1Channel[ch])
	    {
		string var = x.first;
		h1Channel[ch][var][gain]->SetStats(false);
		h1Channel[ch][var][gain]->Draw();
		c2->SaveAs(Form("%s/%s_ch%d_%s.png", fdir.c_str(), gain, ch, var.c_str()));
	    }
	}
	delete c2;
    }
}
#endif

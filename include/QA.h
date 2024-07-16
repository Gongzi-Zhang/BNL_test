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
    time_t deltaT = 0;

    // histograms
    map<string, map<string, TH1F*>> h1;
    map<string, map<string, TH2F*>> h2;
    // map<int, map<string, map<string, TH1F*>>> h1Layer;
    map<int, map<string, TH1F*>> h1Channel;

    cali::sipmXY pos[cali::channelMax];
    int layerNumber[cali::channelMax];
};

void QA::init()
{
    // channel map
    for (int ch=0; ch<cali::nChannels; ch++)
    {
	layerNumber[ch] = cali::getSipm(ch).layer;
	pos[ch] = cali::getSipmXY(ch);
    }

    map<string, map<string, double>> xmax;
    xmax["hit_MIP"]["LG"] = 10;	    xmax["hit_MIP"]["HG"]   = 10;
    xmax["hit_mul"]["LG"] = 200;    xmax["hit_mul"]["HG"] = 200;
    xmax["event_MIP"]["LG"] = 500;  xmax["event_MIP"]["HG"] = 200;
    xmax["ch_MIP"]["LG"] = 10;	    xmax["ch_MIP"]["HG"] = 5;

    if (runType == "cosmic")
    {
	xmax["event_MIP"]["LG"] = 1000;	xmax["event_MIP"]["HG"] = 1000;
	xmax["event_mul"]["LG"] = 100;	xmax["event_mul"]["HG"] = 100;
	xmax["ch_MIP"]["LG"] = 200;	xmax["ch_MIP"]["HG"] = 200;
    }
    for (auto const &gain : calo::gains) // gain
    {
	h1["hit_MIP"][gain] = new TH1F(Form("hit_MIP_%s", gain), Form("%s: hit energy;MIP", gain), 100, 0, xmax["hit_MIP"][gain]);
	h1["hit_mul"][gain]   = new TH1F(Form("hit_mul_%s",  gain),  Form("%s: mul (5#sigma)", gain), xmax["hit_mul"][gain], 0, xmax["hit_mul"][gain]);

	h1["event_rate"][gain]  = new TH1F(Form("event_rate_%s", gain),  Form("%s: rate;Time", gain), 100, 1e8, 1e10);	// needs update later
	h1["event_MIP"][gain] = new TH1F(Form("event_MIP_%s", gain),  Form("%s: event energy;MIP", gain), 100, 0, xmax["event_MIP"][gain]);
	h1["event_x"][gain] = new TH1F(Form("event_x_%s", gain),  Form("%s: COG X;cm", gain), 100, -10, 10);
	h1["event_y"][gain] = new TH1F(Form("event_y_%s", gain),  Form("%s: COG Y;cm", gain), 100, -10, 10);
	h1["event_z"][gain] = new TH1F(Form("event_z_%s", gain),  Form("%s: COG Z;layer", gain), 100, 0, cali::nLayers);

	// for (int l=0; l<cali::nLayers; l++)
	// {
	//     h1Layer[l]["mul"][gain] = new TH1F(Form("layer%d_mul_%s", l, gain), Form("%s: layer %d", gain, l), 100, 0, 100);
	//     h1Layer[l]["MIP"][gain] = new TH1F(Form("layer%d_MIP_%s", l, gain), Form("%s: layer %d;MIP", gain, l), 100, 0, 1000);
	//     h1Layer[l]["x"][gain] = new TH1F(Form("layer%d_x_%s", l, gain),   Form("%s: layer %d;cm", gain, l), 100, -10, 10);
	//     h1Layer[l]["y"][gain] = new TH1F(Form("layer%d_y_%s", l, gain),   Form("%s: layer %d;cm", gain, l), 100, -10, 10);
	// }

	for (int ch=0; ch<calo::nChannels; ch++)
	{
	    h1Channel[ch][gain] = new TH1F(Form("ch%d_raw_%s", ch, gain), Form("ch %d", ch), 100, 0, xmax["ch_MIP"][gain]);
	}

	h2["hit_xy"][gain] = new TH2F(Form("hit_xy_%s", gain), Form("%s: hit xy;x(cm);y(cm)", gain), 100, -10, 10, 100, -10, 10);
	h2["event_MIP_vs_hit_mul"][gain] = new TH2F(Form("event_MIP_vs_mul_%s", gain), Form("%s: Event Sum (MIPs) vs Hit Multiplicity;Hit Multiplicity;Event Sum (MIPs)", gain), 100, 0, xmax["hit_mul"][gain], 100, 0, xmax["event_MIP"][gain]);

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
    TTree *t = (TTree*) fin->Get("mip");
    t->AddFriend("raw");
    if (!t->GetEntries())
    {
	cerr << FATAL << "no entry in the roofile: " << rootFile << endl;
	return;
    }

    double TS;
    float rate;
    map<string, map<string, float>> values;
    map<int, map<string, float>> chMIP;

    t->SetBranchAddress("raw.TS", &TS);
    t->SetBranchAddress("raw.rate", &rate);
    for (const char* var : {"hit_mul", "event_e", "event_x", "event_y", "event_z"})
    {
	TBranch *b = (TBranch*) t->GetBranch(var);
	for (const char* gain : calo::gains)
	{
	    values[var][gain] = 0;
	    b->GetLeaf(gain)->SetAddress(&values[var][gain]);
	}
    }
    for (int ch=0; ch<cali::nChannels; ch++)
    {
	TBranch *b = (TBranch*) t->GetBranch(Form("ch_%d", ch));
	for (const char* gain : calo::gains)
	{
	    chMIP[ch][gain] = 0;
	    b->GetLeaf(gain)->SetAddress(&chMIP[ch][gain]);
	}
    }

    t->GetEntry(0);
    double startTS = TS + deltaT - 10;
    t->GetEntry(t->GetEntries() - 1);
    double endTS = TS + deltaT + 10;
    for (auto const& gain : calo::gains)
        h1["event_rate"][gain]->GetXaxis()->SetLimits(startTS, endTS);

    for (int ei=0; ei<t->GetEntries(); ei++)
    {
	t->GetEntry(ei);

	for(auto const & gain : calo::gains)
	{
	    // for (int l=0; l<cali::nLayers; l++)
	    // {
	    //     layerMul[l] = 0;
	    //     layerADC[l] = 0;
	    //     layerX[l] = 0;
	    //     layerY[l] = 0;
	    // }

	    h1["hit_mul"][gain]->Fill(values["hit_mul"][gain]);
	    h1["event_rate"][gain]->Fill(TS + deltaT, rate);
	    h1["event_MIP"][gain]->Fill(values["event_e"][gain]);
	    h1["event_x"][gain]->Fill(values["event_x"][gain]/cm);
	    h1["event_y"][gain]->Fill(values["event_y"][gain]/cm);
	    h1["event_z"][gain]->Fill(values["event_z"][gain]);
	    h2["event_MIP_vs_hit_mul"][gain]->Fill(values["hit_mul"][gain], values["event_e"][gain]);

	    for (int ch=0; ch<cali::nChannels; ch++)
	    {
		float e = chMIP[ch][gain];
		if (e > 0)
		{
		    h1["hit_MIP"][gain]->Fill(e);
		    h2["hit_xy"][gain]->Fill(pos[ch].x/cm, pos[ch].y/cm);
		    h1Channel[ch][gain]->Fill(e);
		}
	    }
	}
    }
    delete t;
}

void QA::fillCosmic()
{
    /*
    TFile *fin = new TFile(rootFile.c_str(), "read");

    double TS;
    float rate;
    map<string, map<string, int>> values;
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
	{
	    string var = "mul";
	    iVal[var] = {{"LG", 0}, {"HG", 0}};
	    TBranch *b = (TBranch*) t->GetBranch(var.c_str());
	    b->GetLeaf("LG")->SetAddress(&iVal[var]["LG"]);
	    b->GetLeaf("HG")->SetAddress(&iVal[var]["HG"]);
	}
	for (int ch=0; ch<calo::nCAENChannels[ci]; ch++)    
	{
	    chCorADC[ch] = {{"LG", 0.}, {"HG", 0.}};
	    TBranch *bcor = (TBranch*) t->GetBranch(Form("ch_%d", ch));
	    bcor->GetLeaf("LG")->SetAddress(&chCorADC[ch]["LG"]);
	    bcor->GetLeaf("HG")->SetAddress(&chCorADC[ch]["HG"]);
	}

	t->GetEntry(0);
	double startTS = TS + deltaT - 10;
	t->GetEntry(t->GetEntries() - 1);
	double endTS = TS + deltaT + 10;
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

	for (int ei=0; ei<t->GetEntries(); ei++)
	{
	    t->GetEntry(ei);

	    for (auto const &gain : calo::gains)
	    {
		eventADC = 0;
		eventX = 0;
		eventY = 0;
		eventZ = 0;


		h1["event_rate"][gain]->Fill(TS + deltaT, rate);
		h1["event_mul"][gain]->Fill(iVal["mul"][gain]);

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
     */
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
	    string var = x.first;
	    if (var == "event_MIP" || var == "hit_MIP")
		c->SetLogy(1);
	    else
		c->SetLogy(0);

	    h1[var][gain]->Draw("HIST");
	    c->SaveAs(Form("%s/%s_%s.png", fdir.c_str(), gain, var.c_str()));
	}
	c->SetLogy(0);
	for (auto const & x : h2)
	{
	    string var = x.first;
	    if (var == "hit_xy")
	    {
		reverseXAxis(h2[var][gain]);
		h2[var][gain]->Draw("text");
	    }
	    else
	    {
		h2[var][gain]->Draw("colz");
	    }
	    c->SaveAs(Form("%s/%s_%s.png", fdir.c_str(), gain, var.c_str()));
	}
	delete c;

	// layer plot
	// TCanvas* c1 = new TCanvas("c1", "c1", 800, 600);
	// for (int l=0; l<cali::nLayers; l++)
	// {
	//     for (auto const & x : h1Layer[l])
	//     {
	// 	string var = x.first;
	// 	if (var == "ADC")
	// 	    c1->SetLogy(1);
	// 	else
	// 	    c1->SetLogy(0);
	//     
	// 	h1Layer[l][var][gain]->Draw();
	// 	c1->SaveAs(Form("%s/%s_layer%d_%s.png", fdir.c_str(), gain, l, var.c_str()));
	//     }
	// }
	// delete c1;

	// channel plot
	TCanvas* c2 = new TCanvas("c2", "c2", 600*8, 600*(1 + (calo::nChannels-1)/8));
	c2->SetTopMargin(0.08);
	c2->SetBottomMargin(0.08);
	c2->SetLeftMargin(0.06);
	c2->SetRightMargin(0.05);
	c2->Divide(8, 1 + (calo::nChannels-1)/8, 0, 0);
	for (int ch=0; ch<calo::nChannels; ch++)
	{
	    TVirtualPad *p = c2->cd(ch+1);
	    p->SetLogy(1);
	    h1Channel[ch][gain]->SetStats(false);
	    h1Channel[ch][gain]->Draw();
	}
	c2->SaveAs(Form("%s/%s_ch_e.png", fdir.c_str(), gain));
	delete c2;
    }
}
#endif

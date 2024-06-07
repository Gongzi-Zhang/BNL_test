#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "TStyle.h"
#include "TError.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TVirtualPad.h"
#include "TGaxis.h"
#include "TLatex.h"
#include "cali.h"
#include "utilities.h"
#include "db.h"
#include "analysis.h"

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

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
	cerr << FATAL << "no run number provided" << endl;
	exit(4);
    }

    gErrorIgnoreLevel = kWarning;
    gStyle->SetOptStat(1110);
    const int run = atoi(argv[1]);
    cali::setRun(run);
    caliDB db;
    string runType = db.getRunType(run);
    if (runType != "cosmic" && runType != "data")
    {
	cout << WARNING << "not a data/cosmic run: " << run << endl;
	exit(1);
    }

    const int pedRun = db.getPedRun(run);
    pedestal ped;
    if (!getPedestal(pedRun, ped))
    {
	cerr << FATAL << "unable to read pedestal file" << endl;
	exit(2);
    }

    TChain *tin = new TChain("raw");
    char runFile[1024];
    sprintf(runFile, "%s/data/Run%d.root", cali::CALIROOT, run);
    if (fileExists(runFile))
    {
	tin->Add(runFile);
    }
    else
    {
	int i=1;
	sprintf(runFile, "%s/data/Run%d_%d.root", cali::CALIROOT, run, i);
	while (fileExists(runFile))
	{
	    tin->Add(runFile);
	    i++;
	    sprintf(runFile, "%s/data/Run%d_%d.root", cali::CALIROOT, run, i);
	}
	cout << INFO << "find " << i-1 << " root files for run " << run << endl ;
    }
    
    if (!tin->GetEntries())
    {
	cerr << FATAL << "no entry in the root file: " << runFile << endl;
	exit(4);
    }

    map<string, map<int, long long>> rawValue;
    map<string, map<int, long long>> corValue;
    double TS, preTS = -1e9;
    // TTree *tout = new TTree("cor", "calibevent_rated values");
    for (int ch=0; ch<cali::nChannels; ch++)
    {
	for (auto gain : cali::gains)
	{
	    rawValue[gain][ch] = 0;
	    corValue[gain][ch] = 0;
	    tin->SetBranchAddress(Form("Ch_%d_%s", ch, gain), &rawValue[gain][ch]); // Long64_t ==> long long
	}
	// tout->Branch(Form("Ch_%d", ch), &corValue[ch], "LG/F:HG/F");
    }
    tin->SetBranchAddress("TS", &TS);

    tin->GetEntry(0);
    double startTS = TS - 10;
    tin->GetEntry(tin->GetEntries() - 1);
    double endTS = TS + 10;

    cali::sipmXY pos[cali::channelMax];
    int layerNumber[cali::channelMax];
    for (int ch=0; ch<cali::nChannels; ch++)
    {
	layerNumber[ch] = cali::getSipm(ch).layer;
	pos[ch] = cali::getSipmXY(ch);
    }

    char fdir[256];
    sprintf(fdir, "%s/figures/%d/", cali::CALIROOT, run);
    if (!dirExists(fdir))
	mkdir(fdir, 0755);

    // QA plots
    map<string, map<string, TH1F*>> h1;
    map<string, map<string, TH2F*>> h2;
    map<string, map<string, vector<TH1F*>>> h1Layer;
    map<string, map<string, vector<TH1F*>>> h1Channel;
    map<string, map<string, double>> xmax = {
	{"hit_e",	{{"LG", 8500}, {"HG", 8500}}},
	{"event_e",	{{"LG", 1e5},  {"HG", 6e5}}},
	{"event_mul",	{{"LG", 200},  {"HG", 200}}},
	{"event_mul1",	{{"LG", 200},  {"HG", 200}}},
	{"event_mul2",	{{"LG", 200},  {"HG", 200}}},
	{"ch_raw_e",    {{"LG", 2000}, {"HG", 8500}}},
	{"ch_cor_e",    {{"LG", 2000}, {"HG", 8500}}},
    };
    if (runType == "cosmic")
    {
	xmax["hit_e"]["LG"] = 1000;
	xmax["event_e"]["LG"] = 1000;
	xmax["event_e"]["HG"] = 25000;
	xmax["event_mul"]["LG"] = 100;
	xmax["event_mul"]["HG"] = 100;
	xmax["event_mul1"]["LG"] = 20;
	xmax["event_mul1"]["HG"] = 20;
	xmax["event_mul2"]["LG"] = 20;
	xmax["event_mul2"]["HG"] = 20;
	xmax["ch_raw_e"]["LG"] = 300;
	xmax["ch_raw_e"]["HG"] = 800;
	xmax["ch_cor_e"]["LG"] = 100;
	xmax["ch_cor_e"]["HG"] = 800;
    }
    for (auto gain : cali::gains)
    {
	h1[gain]["hit_e"] = new TH1F(Form("%s_hit_e", gain), Form("%s: hit energy;ADC", gain), 100, 0, xmax["hit_e"][gain]);

	h1[gain]["event_rate"]  = new TH1F(Form("%s_event_rate", gain),  Form("%s: rate;Time", gain), 100, startTS, endTS);
	h1[gain]["event_mul"]   = new TH1F(Form("%s_event_mul",  gain),  Form("%s: mul", gain), xmax["event_mul"][gain], 0, xmax["event_mul"][gain]);
	h1[gain]["event_mul1"]  = new TH1F(Form("%s_event_mul1", gain),  Form("%s: mul (3#sigma)", gain), xmax["event_mul1"][gain], 0, xmax["event_mul1"][gain]);
	h1[gain]["event_mul2"]  = new TH1F(Form("%s_event_mul2", gain),  Form("%s: mul (5#sigma)", gain), xmax["event_mul2"][gain], 0, xmax["event_mul2"][gain]);
	h1[gain]["event_e"] = new TH1F(Form("%s_event_e", gain),  Form("%s: event energy;ADC", gain), 100, 0, xmax["event_e"][gain]);
	h1[gain]["event_x"] = new TH1F(Form("%s_event_x", gain),  Form("%s: COG X;cm", gain), 100, -10, 10);
	h1[gain]["event_y"] = new TH1F(Form("%s_event_y", gain),  Form("%s: COG Y;cm", gain), 100, -10, 10);
	h1[gain]["event_z"] = new TH1F(Form("%s_event_z", gain),  Form("%s: COG Z;layer", gain), 100, 0, cali::nLayers);

	for (int l=0; l<cali::nLayers; l++)
	{
	    h1Layer[gain]["mul"].push_back( new TH1F(Form("%s_mul_l%d", gain, l), Form("%s: layer %d", gain, l), 100, 0, 100));
	    h1Layer[gain]["e"].push_back(   new TH1F(Form("%s_e_l%d", gain, l),   Form("%s: layer %d", gain, l), 100, 0, 1000));
	    h1Layer[gain]["x"].push_back(   new TH1F(Form("%s_x_l%d", gain, l),   Form("%s: layer %d", gain, l), 100, -10, 10));
	    h1Layer[gain]["y"].push_back(   new TH1F(Form("%s_y_l%d", gain, l),   Form("%s: layer %d", gain, l), 100, -10, 10));
	}

	for (int ch=0; ch<cali::nChannels; ch++)
	{
	    h1Channel[gain]["raw"].push_back(new TH1F(Form("%s_raw_ch%d", gain, ch), Form("%s: Ch %d", gain, ch), 100, 0, xmax["ch_raw_e"][gain]));
	    h1Channel[gain]["cor"].push_back(new TH1F(Form("%s_cor_ch%d", gain, ch), Form("%s: Ch %d", gain, ch), 100, 0, xmax["ch_cor_e"][gain]));
	}

	h2[gain]["hit_xy"] = new TH2F(Form("%s_hit_xy", gain), Form("%s: hit xy;x(cm);y(cm)", gain), 100, -10, 10, 100, -10, 10);

	// formats
	h1[gain]["event_rate"]->GetXaxis()->SetTimeDisplay(1);
	// h1["event_rate"]->GetXaxis()->SetTimeOffset(0, "gmt");
	h1[gain]["event_rate"]->GetXaxis()->SetTimeFormat("%H:%M");
	h1[gain]["event_rate"]->SetStats(false);

	h2[gain]["hit_xy"]->SetStats(false);

	double layer_e[cali::layerMax], layer_x[cali::layerMax], layer_y[cali::layerMax];
	int layer_mul[cali::layerMax];
	int layer;
	double e, x, y;
	int mul, mul1, mul2;
	double event_e, event_x, event_y, event_z;

	for (int ei=0; ei<tin->GetEntries(); ei++)
	{
	    tin->GetEntry(ei);

	    // initialize variables
	    mul = 0; mul1 = 0; mul2 = 0;
	    event_e = 0; 
	    event_x = 0; event_y = 0; event_z = 0;

	    for (int l=0; l<cali::nLayers; l++)
	    {
		layer_mul[l]  = 0;
		layer_e[l] = 0;
		layer_x[l] = 0;
		layer_y[l] = 0;
	    }
	    for (int ch=0; ch<cali::nChannels; ch++)
	    {
		h1Channel[gain]["raw"][ch]->Fill(rawValue[gain][ch]);

		corValue[gain][ch] = rawValue[gain][ch] - ped[gain][ch].mean;

		h1Channel[gain]["cor"][ch]->Fill(corValue[gain][ch]);

		// check LG only now
		if (corValue[gain][ch] < 1*ped[gain][ch].rms)
		    continue;

		e = corValue[gain][ch];
		x = pos[ch].x/cm; y = pos[ch].y/cm;
		layer = layerNumber[ch];

		mul++;
		if (e > 3*ped[gain][ch].rms)
		    mul1++;
		if (e > 5*ped[gain][ch].rms)
		    mul2++;

		h1[gain]["hit_e"]->Fill(e);
		h2[gain]["hit_xy"]->Fill(x, y);

		layer_mul[layer]++;
		layer_e[layer] += e;
		layer_x[layer] += x*e;
		layer_y[layer] += y*e;

		event_e += e;
		event_x += x*e;
		event_y += y*e;
		event_z += layer*e;
	    }

	    if (event_e)
	    {
		h1[gain]["event_e"]->Fill(event_e);
		h1[gain]["event_x"]->Fill(event_x/event_e);
		h1[gain]["event_y"]->Fill(event_y/event_e);
		h1[gain]["event_z"]->Fill(event_z/event_e);
	    }
	    for (int l=0; l<cali::nLayers; l++)
	    {
		if (layer_mul[l] && layer_e[l])
		{
		    h1Layer[gain]["mul"][l]->Fill(layer_mul[l]);
		    h1Layer[gain]["e"][l]->Fill(layer_e[l]);
		    h1Layer[gain]["x"][l]->Fill(layer_x[l]/layer_e[l]);
		    h1Layer[gain]["y"][l]->Fill(layer_y[l]/layer_e[l]);
		}
	    }

	    h1[gain]["event_rate"]->Fill(TS, 1/(TS - preTS));
	    preTS = TS;

	    h1[gain]["event_mul"]->Fill(mul);
	    h1[gain]["event_mul1"]->Fill(mul1);
	    h1[gain]["event_mul2"]->Fill(mul2);
	}

	TCanvas* c = new TCanvas("c", "c", 1000, 600);
	for (auto const & x : h1[gain])
	{
	    if (x.first == "event_e" || x.first == "hit_e")
		c->SetLogy(1);
	    x.second->Draw("HIST");
	    c->SaveAs(Form("%s/%s_%s.png", fdir, gain, x.first.c_str()));
	    x.second->Delete();
	    c->SetLogy(0);
	}
	for (auto const & x : h2[gain])
	{
	    x.second->Draw("text");
	    reverseXAxis(x.second);
	    c->SaveAs(Form("%s/%s_%s.png", fdir, gain, x.first.c_str()));
	    x.second->Delete();
	}
	delete c;

	// layer plot
	TCanvas* c1 = new TCanvas("c1", "c1", 2000, 1200);
	for (auto const & x : h1Layer[gain])
	{
	    c1->Clear();
	    c1->SetTopMargin(0.12);
	    c1->SetBottomMargin(0.1);
	    c1->SetLeftMargin(0.1);
	    c1->SetRightMargin(0.1);
	    c1->Divide(4, 3, 0, 0);
	    TLatex *title = new TLatex();
	    title->SetName(Form("%s_layer_%s", gain, x.first.c_str()));
	    title->SetTextSize(0.03);
	    title->DrawLatexNDC(.4, 0.97, Form("%s: layer %s", gain, x.first.c_str()));
	    for (int l=0; l<cali::nLayers; l++)
	    {
		c1->cd(l+1);
		x.second[l]->Draw();
	    }
	    c1->SaveAs(Form("%s/%s_layer_%s.png", fdir, gain, x.first.c_str()));
	}
	delete c1;

	// channel plot
	TCanvas* c2 = new TCanvas("c2", "c2", 6000, 2000);
	for (auto const & x : h1Channel[gain])
	{
	    for (int cn=0; cn<cali::nCAENs; cn++)
	    {
		c2->Clear();
		c2->SetTopMargin(0.12);
		c2->SetBottomMargin(0.1);
		c2->SetLeftMargin(0.1);
		c2->SetRightMargin(0.1);
		c2->Divide(16, 4, 0, 0);
		TLatex *title = new TLatex();
		title->SetTextSize(0.03);
		title->DrawLatexNDC(.4, 0.97, Form("%s: %s ADC (CAEN %d)", gain, x.first.c_str(), cn));
		for (int ch=0; ch<cali::nCAENChannels; ch++)
		{
		    int gCh = ch + cn*cali::nCAENChannels;
		    if (gCh >= cali::nChannels)
			continue;
		    TVirtualPad *p = c2->cd(ch+1);
		    p->SetLogy(1);
		    x.second[gCh]->SetStats(false);
		    x.second[gCh]->Draw();
		}
		c2->SaveAs(Form("%s/%s_channel_%s_%d.png", fdir, gain, x.first.c_str(), cn));
	    }
	}
	delete c2;
    }
    // tout->write();
    tin->Delete();
}

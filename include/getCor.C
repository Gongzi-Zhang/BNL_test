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

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
	cerr << FATAL << "no run number provided" << endl;
	exit(4);
    }

    gErrorIgnoreLevel = kWarning;
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

    tin->GetEntry(0);

    for (auto gain : cali::gains)
    {
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

		// check LG only now
		if (corValue[gain][ch] < 1*ped[gain][ch].rms)
		    continue;

		h1Channel[gain]["cor"][ch]->Fill(corValue[gain][ch]);

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
		    // x.second[gCh]->SetStats(false);
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

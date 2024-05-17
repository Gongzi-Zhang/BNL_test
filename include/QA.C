#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
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
    Ped ped = getPedestal(pedRun);
    if (!ped.size())
    {
	cerr << FATAL << "unable to read pedestal file" << endl;
	exit(2);
    }

    const char* runFile = Form("%s/data/Run%d.root", cali::CALIROOT, run);
    TFile* fin = new TFile(runFile, "update");
    if (!fin->IsOpen())
    {
	cerr << FATAL << "can't open the root file for run: " << run << endl;
	exit(4);
    }
    
    TTree *tin = (TTree*) fin->Get("raw");
    if (!tin->GetEntries())
    {
	cerr << FATAL << "no entry in the root file: " << runFile << endl;
	exit(4);
    }

    map<int, cali::ADC_L> rawValue;
    map<int, cali::ADC>	  corValue;
    double TS, preTS = -1e9;
    TTree *tout = new TTree("cor", "calibevent_rated values");
    for (int ch=0; ch<cali::nChannels; ch++)
    {
	rawValue[ch] = {0, 0};
	corValue[ch] = {0, 0};
	tin->SetBranchAddress(Form("Ch_%d_LG", ch), &rawValue[ch].LG); // Long64_t ==> long long
	tin->SetBranchAddress(Form("Ch_%d_HG", ch), &rawValue[ch].HG);
	tout->Branch(Form("Ch_%d", ch), &corValue[ch], "LG/F:HG/F");
    }
    tin->SetBranchAddress("TS", &TS);

    tin->GetEntry(0);
    double startTS = TS - 10;
    tin->GetEntry(tin->GetEntries() - 1);
    double endTS = TS + 10;

    // QA plots
    map<string, TH1F*> h1;
    h1["hit_e"] = new TH1F("hit_e",   Form("run %d: hit energy;ADC", run), 100, 0, 500);

    h1["event_rate"]  = new TH1F("event_rate",  Form("run %d: rate", run), 100, startTS, endTS);
    h1["event_mul"]   = new TH1F("event_mul",   Form("run %d: mul", run), 100, 0, 300);
    h1["event_mul1"]  = new TH1F("event_mul1",  Form("run %d: mul (3#sigma)", run), 100, 0, 300);
    h1["event_mul2"]  = new TH1F("event_mul2",   Form("run %d: mul (5#sigma)", run), 100, 0, 300);
    h1["event_e"] = new TH1F("event_e",   Form("run %d: event energy;ADC", run), 100, 0, 5000);
    h1["event_x"] = new TH1F("event_x",   Form("run %d: COG X;cm", run), 100, -10, 10);
    h1["event_y"] = new TH1F("event_y",   Form("run %d: COG Y;cm", run), 100, -10, 10);
    h1["event_z"] = new TH1F("event_z",   Form("run %d: COG Z;layer", run), 100, 0, cali::nLayers);

    map<string, vector<TH1F*>> h1Layer;
    for (int l=0; l<cali::nLayers; l++)
    {
	h1Layer["mul"].push_back( new TH1F(Form("mul_l%d", l), Form("layer %d", l), 100, 0, 100));
	h1Layer["e"].push_back(   new TH1F(Form("e_l%d", l),   Form("layer %d", l), 100, 0, 1000));
	h1Layer["x"].push_back(	  new TH1F(Form("x_l%d", l),   Form("layer %d", l), 100, -10, 10));
	h1Layer["y"].push_back(	  new TH1F(Form("y_l%d", l),   Form("layer %d", l), 100, -10, 10));
    }

    map<string, TH2F*> h2;
    h2["hit_xy"] = new TH2F("hit_xy", Form("run%d: hit xy;x(cm);y(cm)", run), 100, -10, 10, 100, -10, 10);

    // formats
    h1["event_rate"]->GetXaxis()->SetTimeDisplay(1);
    h1["event_rate"]->GetXaxis()->SetTimeFormat("%H:%M");
    h1["event_rate"]->SetStats(false);

    cali::sipmXY pos[cali::channelMax];
    int layerNumber[cali::channelMax];
    for (int ch=0; ch<cali::nChannels; ch++)
    {
	layerNumber[ch] = cali::getSipm(ch).layer;
	pos[ch] = cali::getSipmXY(ch);
    }
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
	    corValue[ch].LG = rawValue[ch].LG - ped[ch].LG.mean;
	    corValue[ch].HG = rawValue[ch].HG - ped[ch].HG.mean;

	    // check LG only now
	    if (corValue[ch].LG < 1*ped[ch].LG.rms)
		continue;

	    e = corValue[ch].LG;
	    x = pos[ch].x/cm; y = pos[ch].y/cm;
	    layer = layerNumber[ch];

	    mul++;
	    if (e > 3*ped[ch].LG.rms)
		mul1++;
	    if (e > 5*ped[ch].LG.rms)
		mul2++;

	    h1["hit_e"]->Fill(e);
	    h2["hit_xy"]->Fill(x, y);

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
	    h1["event_e"]->Fill(event_e);
	    h1["event_x"]->Fill(event_x/event_e);
	    h1["event_y"]->Fill(event_y/event_e);
	    h1["event_z"]->Fill(event_z/event_e);
	}
	for (int l=0; l<cali::nLayers; l++)
	{
	    if (layer_mul[l] && layer_e[l])
	    {
		h1Layer["mul"][l]->Fill(layer_mul[l]);
		h1Layer["e"][l]->Fill(layer_e[l]);
		h1Layer["x"][l]->Fill(layer_x[l]/layer_e[l]);
		h1Layer["y"][l]->Fill(layer_y[l]/layer_e[l]);
	    }
	}

	h1["event_rate"]->Fill(TS, 1/(TS - preTS));
	preTS = TS;

	h1["event_mul"]->Fill(mul);
	h1["event_mul1"]->Fill(mul1);
	h1["event_mul2"]->Fill(mul2);

	tout->Fill();
    }

    const char* dir = Form("%s/figures/QA/run%d", cali::CALIROOT, run);
    if (!dirExists(dir))
	mkdir(dir, 0755);

    TCanvas* c = new TCanvas("c", "c", 1000, 600);
    for (auto const & x : h1)
    {
	x.second->Draw();
	c->SaveAs(Form("%s/%s.png", dir, x.first.c_str()));
    }
    for (auto const & x : h2)
    {
        x.second->Draw("colz");
	c->SaveAs(Form("%s/%s.png", dir, x.first.c_str()));
    }

    TCanvas* c1 = new TCanvas("c1", "c1", 2000, 1200);
    c1->Divide(4, 3, 0, 0);
    for (auto const & x : h1Layer)
    {
	for (int l=0; l<cali::nLayers; l++)
	{
	    c1->cd(l+1);
	    x.second[l]->Draw();
	}
	c1->SaveAs(Form("%s/layer_%s.png", dir, x.first.c_str()));
    }
}

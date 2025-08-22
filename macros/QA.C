#include "cali.h"

void QA(const char* inname, const char*outname = "out.root") 
{
    TFile *fin = new TFile(inname, "read");
    TTree *tin = (TTree*) fin->Get("mip");
    tin->AddFriend("raw");

    cali::sipmXY pos[cali::nChannels];
    int layerNumber[cali::nChannels];
    double TS;
    float rate;
    float mip[cali::nChannels];
    tin->SetBranchAddress("raw.TS", &TS);                                         
    tin->SetBranchAddress("raw.rate", &rate);
    for (int ch=0; ch<cali::nChannels; ch++)
    {
	layerNumber[ch] = cali::getSipm(ch).layer;
	pos[ch] = cali::getSipmXY(ch);

	tin->GetBranch(Form("ch_%d", ch))->GetLeaf("LG")->SetAddress(&mip[ch]);
    }

    TFile *fout = new TFile(outname, "recreate");
    map<string, TH1F*> h1;
    map<string, TH2F*> h2;
    h1["event_rate"] = new TH1F("event_rate", "Event Rate", 100, 1e8, 1e10);
    h1["nHits"] = new TH1F("nHits", "nHits", 200, 0, 200);
    h1["hit_energy"] = new TH1F("hit_energy", "Hit Energy;MIP;", 100, 0, 100);
    h2["event_MIP_vs_hit_mul"] = new TH2F("event_MIP_vs_hit_mul", "Event Energy (MIPs) vs Hit Multiplicity;Hit Multiplicity;Event Energy (MIPs)", 100, 0, 100, 100, 0, 1000);
    h2["layer_energy"] = new TH2F("layer_energy", "Layer Energy;Layer;Energy (NIP);", 11, 0, 11, 100, 0, 20);
    for (size_t l=0; l<cali::nLayers; l++)
    {
	h2[Form("layer_%zu_hit_x_y", l)] = new TH2F(Form("layer_%zu_hit_x_y", l), Form("Layer %zu Hit;X (cm);Y (cm)", l), 100, -10, 10, 100, -10, 10);
	h2[Form("layer_%zu_hit_x_y_weighted", l)] = new TH2F(Form("layer_%zu_hit_x_y_weighted", l), Form("Layer %zu Hit Weighted by Energy;X (cm);Y (cm)", l), 100, -10, 10, 100, -10, 10);
    }


    double deltaT = -5*3600;	// time difference between NY time and UCT
    tin->GetEntry(0);
    double startTS = TS + deltaT - 10;
    tin->GetEntry(tin->GetEntries() - 1);
    double endTS = TS + deltaT + 10;
    h1["event_rate"]->GetXaxis()->SetLimits(startTS, endTS);

    int nHits;
    double e;
    double x, y;
    int z;
    double layer_e[cali::nLayers];
    double event_e;
    const int nEntries = tin->GetEntries();
    for (int ei=0; ei<nEntries; ei++)
    {
	tin->GetEntry(ei);

	h1["event_rate"]->Fill(TS + deltaT, rate);
	nHits = 0;
	event_e = 0;
	for (size_t l=0; l<cali::nLayers; l++)
	    layer_e[l] = 0;
	for (int ch=0; ch<cali::nChannels; ch++)
	{
	    if (mip[ch] < 0.3)
		continue;

	    e = mip[ch];
	    x = pos[ch].x;
	    y = pos[ch].y;
	    z = layerNumber[ch];

	    layer_e[z] += e;
	    event_e += e;

	    nHits++;
	    h1["hit_energy"]->Fill(mip[ch]);
	    h2[Form("layer_%d_hit_x_y", z)]->Fill(x/cm, y/cm);
	    h2[Form("layer_%d_hit_x_y_weighted", z)]->Fill(x/cm, y/cm, e);
	}
	h1["nHits"]->Fill(nHits);
	h2["event_MIP_vs_hit_mul"]->Fill(nHits, event_e);

	for (size_t l=0; l<cali::nLayers; l++)
	    if (layer_e[l] > 0)
		h2["layer_energy"]->Fill(l, layer_e[l]);
    }

    /*
    TCanvas* c = new TCanvas("c", "c", 800, 600);
    for (auto var : {"nHits", "hit_energy"})
    {
	h1[var]->Draw();
	c->SaveAs(Form("%s.png", var));
    }
     */

    fin->Close();
    fout->Write();
    fout->Close();
}

void QA(const int run) 
{
    string fname = cali::getFile(Form("Run%d.root", run));
    QA(fname.c_str(), Form("Run%d_QA.root", run));
}


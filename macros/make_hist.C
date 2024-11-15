#include "calo.h"
#include "cali.h"

const int nChannels = 192;
			
void make_hist(const char *, const char*);

void make_hist(const int run)
{
    make_hist(cali::getRootFile(run).c_str(), Form("Run%d", run));
}

void make_hist(const char *fname = "input.edm4hep.root", 
	  const char*out_prefix = "output")
{
    float hit_energy_cut = 0.5;
    string unit = "MIP";

    cali::sipmXY pos[cali::channelMax];
    int layerNumber[cali::channelMax];
    for (int ch=0; ch<cali::nChannels; ch++)
    {
        layerNumber[ch] = cali::getSipm(ch).layer;
        pos[ch] = cali::getSipmXY(ch);
    }

    TFile *fin = new TFile(fname, "read");
    TTree *tin = (TTree*) fin->Get("mip");
    float chMIP[nChannels];
    float eventMIP;
    for (int ch=0; ch<nChannels; ch++)
    {
	TBranch *b = tin->GetBranch(Form("ch_%d", ch));
	b->GetLeaf("LG")->SetAddress(&chMIP[ch]);
    }

    string eRanges[] = {"50MIP", "50-150MIP", "150-300MIP", "300-600MIP", "600MIP"};
    map<string, const double> minEventEnergy = {
	{"50MIP",	10},
	{"50-150MIP",	10},
	{"150-300MIP",	100},
	{"300-600MIP",	250},
	{"600MIP",	550},
    };
    map<string, const double> maxEventEnergy = {
	{"50MIP",	1000},
	{"50-150MIP",	200},
	{"150-300MIP",	350},
	{"300-600MIP",	650},
	{"600MIP",	1000},
    };

    map<string, TFile*> fout;
    map<string, map<string, TH1F*>> h1;
    map<string, map<string, TH2F*>> h2;
    for (const string eRange : eRanges)
    {
	fout[eRange] = new TFile(Form("%s_hit_%.1fMIP_event_%s_hist.root", out_prefix, hit_energy_cut, eRange.c_str()), "recreate");
	fout[eRange]->cd();
	h1[eRange]["hit_MIP"] = new TH1F("hit_MIP", "Hit Energy;MIP", 100, 0, 50);
	h1[eRange]["hit_mul"] = new TH1F("hit_mul", "Hit Multiplicity", 200, 0, 200);
	h1[eRange]["hit_mul1"] = new TH1F("hit_mul1", "Hit Multiplicity (0.5 - 2 MIPs)", 100, 0, 100);
	h1[eRange]["hit_mul2"] = new TH1F("hit_mul2", "Hit Multiplicity (2 - 5 MIPs)", 100, 0, 100);
	h1[eRange]["hit_mul3"] = new TH1F("hit_mul3", "Hit Multiplicity (5 - 10 MIPs)", 100, 0, 100);
	h1[eRange]["hit_mul4"] = new TH1F("hit_mul4", "Hit Multiplicity (> 10 MIPs)", 100, 0, 100);
	h1[eRange]["event_MIP"] = new TH1F("event_MIP", "Event Energy;MIP", 100, minEventEnergy[eRange], maxEventEnergy[eRange]);
	h1[eRange]["event_x"] = new TH1F("event_x", "COG X;cm", 100, -10, 10);
	h1[eRange]["event_y"] = new TH1F("event_y", "COG Y;cm", 100, -10, 10);
	h1[eRange]["event_z"] = new TH1F("event_z", "COG Z", 100, 0, cali::nLayers);
	h2[eRange]["event_MIP_vs_hit_mul"] = new TH2F("event_MIP_vs_hit_mul", "event MIP vs hit mul", 200, 0, 200, 100, minEventEnergy[eRange], maxEventEnergy[eRange]);
	h2[eRange]["event_MIP_vs_eta"] = new TH2F("event_MIP_vs_eta", "event MIP vs eta", 100, 2.5, 3.5, 100, minEventEnergy[eRange], maxEventEnergy[eRange]);
	h2[eRange]["x_vs_y"] = new TH2F("x_vs_y", "X vs Y;X cm;Y cm", 100, -10, 10, 100, -10, 10);
    }

    int hit_mul, hit_mul1, hit_mul2, hit_mul3, hit_mul4;
    double e, event_e;
    double event_x, event_y, event_z;
    double x, y, z, theta, eta;
    string eRange;

    const int ne = tin->GetEntries();
    for (int ei = 0; ei<ne; ei++)
    {
	tin->GetEntry(ei);

	hit_mul = hit_mul1 = hit_mul2 = hit_mul3 = hit_mul4 = 0;
        event_e = 0;
	event_x = event_y = event_z = 0;

	for (int ch=0; ch<nChannels; ch++)
	{
	    if (chMIP[ch] < hit_energy_cut)
		continue;
	    hit_mul++;
	    event_e += chMIP[ch];
	}

	if (event_e < 50)   // 30 MIP ~ 1GeV
	    continue;
	if (event_e < 150)   // 30 MIP ~ 1GeV
	    eRange = "50-150MIP";
	else if (event_e < 300)	    // 5 - 10 GeV
	    eRange = "150-300MIP";
	else if (event_e < 600)	    // 10 - 20 GeV 
	    eRange = "300-600MIP";
	else 
	    eRange = "600MIP";	    // 20 GeV -

	hit_mul = 0;
	event_e = 0;
	for (int ch=0; ch<nChannels; ch++)
	{
	    if (chMIP[ch] < hit_energy_cut)
		continue;
	    hit_mul++;
	    e = chMIP[ch];
	    event_e += e;
	    event_x += pos[ch].x*e;
	    event_y += pos[ch].y*e;
	    event_z += layerNumber[ch]*e;

	    if (e > 10)
		hit_mul4++;
	    else if (e > 5)
		hit_mul3++;
	    else if (e > 2)
		hit_mul2++;
	    else if (e > 0.5)
		hit_mul1++;

	    h1["50MIP"]["hit_MIP"]->Fill(chMIP[ch]);
	    h1[eRange]["hit_MIP"]->Fill(chMIP[ch]);
	}

	{
	    x = event_x/event_e + cali::X;
	    y = event_y/event_e + cali::Y;
	    z = event_z/event_e*cali::layerZ + cali::Z;
	    theta = atan(sqrt(x*x + y*y)/z);
	    eta = -log(tan(theta/2));
	    for (const string r : {"50MIP", eRange.c_str()})
	    {
		h1[r]["event_MIP"]->Fill(event_e);
		h1[r]["event_x"]->Fill(event_x/event_e/cm);
		h1[r]["event_y"]->Fill(event_y/event_e/cm);
		h1[r]["event_z"]->Fill(event_z/event_e);
		h1[r]["hit_mul"]->Fill(hit_mul);
		h1[r]["hit_mul1"]->Fill(hit_mul1);
		h1[r]["hit_mul2"]->Fill(hit_mul2);
		h1[r]["hit_mul3"]->Fill(hit_mul3);
		h1[r]["hit_mul4"]->Fill(hit_mul4);

		h2[r]["event_MIP_vs_hit_mul"]->Fill(hit_mul, event_e);
		h2[r]["event_MIP_vs_eta"]->Fill(eta, event_e);
		h2[r]["x_vs_y"]->Fill(event_x/event_e/cm, event_y/event_e/cm);
	    }
	}
    }

    fin->Close();
    for (const string eRange : eRanges)
    {
	fout[eRange]->cd();
	fout[eRange]->Write();
	fout[eRange]->Close();
    }
}

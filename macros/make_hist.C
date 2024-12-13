#include "calo.h"
#include "cali.h"
#include "caliType.h"

void make_hist(const char *fname = "output.hit.root", 
	  const char*out_prefix = "output")
{
    float hit_energy_cut = 0.5;
    string unit = "MIP";

    TFile *fin = new TFile(fname, "read");
    TTree *tin = (TTree*) fin->Get("events");

    TTreeReader tr(tin);
    // TTreeReaderValue<float> T1(tr, "T1");
    // TTreeReaderValue<float> T3(tr, "T3");
    TTreeReaderArray<float> hit_e(tr, "CALIHits.e");
    TTreeReaderArray<float> hit_x(tr, "CALIHits.x");
    TTreeReaderArray<float> hit_y(tr, "CALIHits.y");
    TTreeReaderArray<int>   hit_z(tr, "CALIHits.z");
    TTreeReaderArray<float> clu_e(tr, "CALIClusters.e");
    TTreeReaderArray<size_t> clu_nhits(tr, "CALIClusters.nhits");

    string eRanges[] = {"100MIP", "100-150MIP", "150MIP", "150-300MIP", "300-600MIP", "600MIP"};
    map<string, const double> minEventEnergy = {
	{"100MIP",	50},
	{"100-150MIP",	50},
	{"150MIP",	100},
	{"150-300MIP",  100},
	{"300-600MIP",  250},
	{"600MIP",	550},
    };
    map<string, const double> maxEventEnergy = {
	{"100MIP",	1000},
	{"100-150MIP",	200}, 
	{"150MIP",	1000},
	{"150-300MIP",  350}, 
	{"300-600MIP",  650}, 
	{"600MIP",	1000},
    };                                 
    map<string, TFile*> fout;
    map<string, map<string, TH1F*>> h1;
    map<string, map<string, TH2F*>> h2;

    for (const string eRange : eRanges)
    {
	fout[eRange] = new TFile(Form("%s_hit_%.1fMIP_event_%s_hist.root", out_prefix, hit_energy_cut, eRange.c_str()), "recreate");
	fout[eRange]->cd();

	h1[eRange]["hit_MIP"] = new TH1F("hit_MIP", "Hit Energy;MIP", 100, 0, 200);
	h1[eRange]["hit_mul"] = new TH1F("hit_mul", "Hit Multiplicity", 100, 0, 100);
	h1[eRange]["hit_mul1"] = new TH1F("hit_mul1", "Hit Multiplicity (0.5 - 2 MIPs)", 100, 0, 100);
	h1[eRange]["hit_mul2"] = new TH1F("hit_mul2", "Hit Multiplicity (2 - 5 MIPs)", 100, 0, 100);
	h1[eRange]["hit_mul3"] = new TH1F("hit_mul3", "Hit Multiplicity (5 - 10 MIPs)", 100, 0, 100);
	h1[eRange]["hit_mul4"] = new TH1F("hit_mul4", "Hit Multiplicity (> 10 MIPs)", 100, 0, 100);
	h1[eRange]["event_MIP"] = new TH1F("event_MIP", "Event Energy;MIP", 100, minEventEnergy[eRange], maxEventEnergy[eRange]);
	h1[eRange]["event_x"] = new TH1F("event_x", "COG X;cm", 100, -10, 10);
	h1[eRange]["event_y"] = new TH1F("event_y", "COG Y;cm", 100, -10, 10);
	h1[eRange]["event_z"] = new TH1F("event_z", "COG Z;layer", 100, 0, cali::nLayers);
	h1[eRange]["clu_mul"] = new TH1F("clu_mul", "Cluster Multiplicity", 10, 0, 10);
	h1[eRange]["clu_MIP"] = new TH1F("clu_MIP", "Cluster Energy;MIP", 100, 0, 200);
	h1[eRange]["clu_nhits"] = new TH1F("clu_nhits", "Cluster Number of Hits", 20, 0, 20);

	h2[eRange]["event_MIP_vs_hit_mul"] = new TH2F("event_MIP_vs_hit_mul", "event MIP vs hit mul", 100, 0, 100, 100, minEventEnergy[eRange], maxEventEnergy[eRange]);
	h2[eRange]["event_MIP_vs_eta"] = new TH2F("event_MIP_vs_eta", "event MIP vs #eta", 100, 2.5, 3.5, 100, minEventEnergy[eRange], maxEventEnergy[eRange]);
	h2[eRange]["x_vs_y"] = new TH2F("x_vs_y", "X vs Y;cm;cm", 100, -10, 10, 100, -10, 10);
    }

    double e, event_e;
    double event_x, event_y, event_z;
    double x, y, z, theta, eta;
    int hit_mul, hit_mul1, hit_mul2, hit_mul3, hit_mul4;
    vector<size_t> eRange_idx;

    const int ne = tin->GetEntries();
    for (int ei = 0; ei<ne; ei++)
    {
	// tin->GetEntry(ei);
	tr.Next();

	hit_mul = hit_mul1 = hit_mul2 = hit_mul3 = hit_mul4 = 0;
	event_e = 0;
	event_x = event_y = event_z = 0;

	// trigger
	// if (T1 < 0.18 || T2 < 0.05 || T3 < 0.6)
	// if (T1 < 5)
	//     continue;

	// Hits
	for (int hi=0; hi<hit_e.GetSize(); hi++)
	{
	    e = hit_e[hi];
	    if (e < hit_energy_cut)
		continue;
	    hit_mul++;
	    event_e += e;
	}

	eRange_idx.clear();

	if (event_e < 100)  // 30 MIP ~ 1 GeV
	    continue;

	eRange_idx.push_back(0);
	if (event_e < 150)  // 30 MIP ~ 1 GeV
	    eRange_idx.push_back(1);
	else 
	{
	    eRange_idx.push_back(2);
	    if (event_e < 300)   
		eRange_idx.push_back(3);  // 5 - 10 GeV
	    else if (event_e < 600)
		eRange_idx.push_back(4);  // 10 - 20 GeV
	    else 
		eRange_idx.push_back(5);  // 20 GeV -
	}

	for (int hi=0; hi<hit_e.GetSize(); hi++)
	{
	    e = hit_e[hi];
	    if (e < hit_energy_cut)
		continue;

	    x = hit_x[hi];
	    y = hit_y[hi];
	    z = hit_z[hi];
	    event_x += x*e;
	    event_y += y*e;
	    event_z += z*e;

	    if (e > 10)
		hit_mul4++;
	    else if (e > 5)
		hit_mul3++;
	    else if (e > 2)
		hit_mul2++;
	    else if (e > 0.5)
		hit_mul1++;       

	    for (const auto idx : eRange_idx)
	    {
		h1[eRanges[idx]]["hit_MIP"]->Fill(e);
	    }
	}

	// Clusters
	for (int ci=0; ci<clu_e.GetSize(); ci++)
	{
	    for (const auto idx : eRange_idx)
	    {
		h1[eRanges[idx]]["clu_MIP"]->Fill(clu_e[ci]);
		h1[eRanges[idx]]["clu_nhits"]->Fill(clu_nhits[ci]);
	    }
	}

	{
	    x = event_x/event_e + cali::x0;
	    y = event_y/event_e + cali::y0;
	    z = event_z/event_e*cali::lt + cali::z0;
	    theta = atan(sqrt(x*x + y*y)/z);
	    eta = -log(tan(theta/2));

	    for (const auto idx : eRange_idx)
	    {
		h1[eRanges[idx]]["hit_mul"]->Fill(hit_mul);
		h1[eRanges[idx]]["hit_mul1"]->Fill(hit_mul1);
		h1[eRanges[idx]]["hit_mul2"]->Fill(hit_mul2);
		h1[eRanges[idx]]["hit_mul3"]->Fill(hit_mul3);
		h1[eRanges[idx]]["hit_mul4"]->Fill(hit_mul4);
		h1[eRanges[idx]]["event_MIP"]->Fill(event_e);
		h1[eRanges[idx]]["event_x"]->Fill(event_x/event_e/cm);
		h1[eRanges[idx]]["event_y"]->Fill(event_y/event_e/cm);
		h1[eRanges[idx]]["event_z"]->Fill(event_z/event_e);
		h1[eRanges[idx]]["clu_mul"]->Fill(clu_e.GetSize());
		h2[eRanges[idx]]["event_MIP_vs_hit_mul"]->Fill(hit_mul, event_e);
		h2[eRanges[idx]]["event_MIP_vs_eta"]->Fill(eta, event_e);
		h2[eRanges[idx]]["x_vs_y"]->Fill(event_x/event_e/cm, event_y/event_e/cm);
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

#include "calo.h"
#include "cali.h"
#include "caliType.h"

void make_hist(const char *fname = "output.myrec.root", 
	  const char*out_prefix = "output")
{
    float low_hit_energy_cut = 0.5;
    float high_hit_energy_cut = 140;
    double scale = 1.1;
    string unit = "MIP";

    TFile *fin = new TFile(fname, "read");
    TTree *tin = (TTree*) fin->Get("events");
    tin->AddFriend("clusters");

    TTreeReader tr(tin);
    TTreeReaderArray<float> hit_e(tr, "CALIHits.e");
    TTreeReaderArray<float> hit_x(tr, "CALIHits.x");
    TTreeReaderArray<float> hit_y(tr, "CALIHits.y");
    TTreeReaderArray<int>   hit_z(tr, "CALIHits.z");
    TTreeReaderArray<float> clu_e(tr, "CALIClusters.e");
    TTreeReaderArray<float> clu_x(tr, "CALIClusters.x");
    TTreeReaderArray<float> clu_y(tr, "CALIClusters.y");
    TTreeReaderArray<float> clu_z(tr, "CALIClusters.z");
    TTreeReaderArray<size_t> clu_nhits(tr, "CALIClusters.nhits");

    string eRanges[] = {"100MIP", "100-150MIP", "150MIP", "150-300MIP", "300-600MIP", "600MIP"};
    map<string, const double> minEventEnergy = {
	{"100MIP",	50},
	{"150MIP",	100},
	{"150-300MIP",  100},
	{"300",		250},
	{"300-600MIP",  250},
	{"600MIP",	550},
    };
    map<string, const double> maxEventEnergy = {
	{"100MIP",	1400},
	{"150MIP",	1400},
	{"150-300MIP",  350}, 
	{"300MIP",	1400},
	{"300-600MIP",  650}, 
	{"600MIP",	1400},
    };                                 
    map<string, TFile*> fout;
    map<string, map<string, TH1F*>> h1;
    map<string, map<string, TH2F*>> h2;

    for (const string eRange : eRanges)
    {
	fout[eRange] = new TFile(Form("%s_event_%s_hist.root", out_prefix, eRange.c_str()), "recreate");
	fout[eRange]->cd();

	h1[eRange]["hit_MIP"] = new TH1F("hit_MIP", "Hit Energy;MIP", 50, 0, 150);
	h1[eRange]["hit_mul"] = new TH1F("hit_mul", "Hit Multiplicity", 80, 0, 160);
	h1[eRange]["hit_mul1"] = new TH1F("hit_mul1", "Hit Multiplicity (0.5 - 2 MIPs)", 100, 0, 100);
	h1[eRange]["hit_mul2"] = new TH1F("hit_mul2", "Hit Multiplicity (2 - 5 MIPs)", 100, 0, 100);
	h1[eRange]["hit_mul3"] = new TH1F("hit_mul3", "Hit Multiplicity (5 - 10 MIPs)", 100, 0, 100);
	h1[eRange]["hit_mul4"] = new TH1F("hit_mul4", "Hit Multiplicity (> 10 MIPs)", 100, 0, 100);
	h1[eRange]["event_MIP"] = new TH1F("event_MIP", "Event Energy;MIP", 50, minEventEnergy[eRange], maxEventEnergy[eRange]);
	h1[eRange]["event_x"] = new TH1F("event_x", "COG X;cm", 100, -10, 10);
	h1[eRange]["event_y"] = new TH1F("event_y", "COG Y;cm", 100, -10, 10);
	h1[eRange]["event_z"] = new TH1F("event_z", "COG Z;layer", 100, 0, cali::nLayers);
	h1[eRange]["clu_mul"] = new TH1F("clu_mul", "Cluster Multiplicity", 8, 0, 8);
	h1[eRange]["clu_MIP"] = new TH1F("clu_MIP", "Cluster Energy;MIP", 100, 0, 400);
	h1[eRange]["clu_x"] = new TH1F("clu_x", "Cluster X;cm", 100, -10, 10);
	h1[eRange]["clu_y"] = new TH1F("clu_y", "Cluster Y;cm", 100, -10, 10);
	h1[eRange]["clu_z"] = new TH1F("clu_z", "Cluster Z;cm", 100, 0, cali::nLayers);
	h1[eRange]["clu_nhits"] = new TH1F("clu_nhits", "Cluster Number of Hits", 50, 0, 50);
	h1[eRange]["pi0_mass"] = new TH1F("pi0_mass", "Invariant Mass of Top Two Clusters;MeV", 50, 0, 1000);

	h2[eRange]["hit_mul_vs_event_MIP"] = new TH2F("hit_mul_vs_event_MIP", "Hit mul vs event MIP", 100, minEventEnergy[eRange], maxEventEnergy[eRange], 80, 0, 160);
	h2[eRange]["eta_vs_event_MIP"] = new TH2F("eta_vs_event_MIP", "#eta vs event MIP", 100, minEventEnergy[eRange], maxEventEnergy[eRange], 100, 3, 3.5);
	h2[eRange]["x_vs_y"] = new TH2F("x_vs_y", "X vs Y;cm;cm", 100, -10, 10, 100, -10, 10);
	h2[eRange]["clu_x_vs_y"] = new TH2F("clu_x_vs_y", "Cluster X vs Y;cm;cm", 100, -10, 10, 100, -10, 10);
	h2[eRange]["clu_x_vs_y_weighted"] = new TH2F("clu_x_vs_y_weighted", "Cluster X vs Y (weighted by cluster energy);cm;cm", 100, -10, 10, 100, -10, 10);
	h2[eRange]["clu_e_vs_x"] = new TH2F("clu_e_vs_x", "Cluster Energy vs X;cm;cm", 100, -10, 10, 100, 0, 200);
	h2[eRange]["clu_e_vs_y"] = new TH2F("clu_e_vs_y", "Cluster Energy vs Y;cm;cm", 100, -10, 10, 100, 0, 200);
	h2[eRange]["clu_e_vs_z"] = new TH2F("clu_e_vs_z", "Cluster Energy vs Z;cm;cm", 100, 0, cali::nLayers, 100, 0, 200);
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

	// Hits
	for (int hi=0; hi<hit_e.GetSize(); hi++)
	{
	    e = hit_e[hi]*scale;
	    if (e < low_hit_energy_cut || e > high_hit_energy_cut)
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
	    e = hit_e[hi]*scale;
	    if (e < low_hit_energy_cut || e > high_hit_energy_cut)
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
		h1[eRanges[idx]]["clu_x"]->Fill(clu_x[ci]/cm);
		h1[eRanges[idx]]["clu_y"]->Fill(clu_y[ci]/cm);
		h1[eRanges[idx]]["clu_z"]->Fill(clu_z[ci]);
		h2[eRanges[idx]]["clu_x_vs_y"]->Fill(clu_x[ci]/cm, clu_y[ci]/cm);
		h2[eRanges[idx]]["clu_x_vs_y_weighted"]->Fill(clu_x[ci]/cm, clu_y[ci]/cm, clu_e[ci]);
		h2[eRanges[idx]]["clu_e_vs_x"]->Fill(clu_x[ci]/cm, clu_e[ci]);
		h2[eRanges[idx]]["clu_e_vs_y"]->Fill(clu_y[ci]/cm, clu_e[ci]);
		h2[eRanges[idx]]["clu_e_vs_z"]->Fill(clu_z[ci], clu_e[ci]);
	    }
	}
	// get pi0
	if (clu_e.GetSize() >= 2)
	{
	    auto swap = [](size_t& idx1, size_t& idx2) {
		size_t idx = idx2;
		idx2 = idx1;
		idx1 = idx;
	    };

	    size_t idx1 = 0;	// highest energy
	    size_t idx2 = 1;	// second highest
	    if (clu_e[idx1] < clu_e[idx2])
	    {
		swap(idx1, idx2);
	    }
	    for (size_t idx = 2; idx < clu_e.GetSize(); idx++)
	    {
                if (clu_e[idx] > clu_e[idx2])
		{
		    swap(idx, idx2);

		    if (clu_e[idx2] > clu_e[idx1])
			swap(idx1, idx2);
		}
	    }

	    float e1 = clu_e[idx1], e2 = clu_e[idx2];
	    float x1 = clu_x[idx1], x2 = clu_x[idx2];
	    float y1 = clu_y[idx1], y2 = clu_y[idx2];
	    float z1 = cali::z0 + cali::lt*clu_z[idx1];
	    float z2 = cali::z0 + cali::lt*clu_z[idx2];
	    float l1 = sqrt(x1*x1 + y1*y1 + z1*z1);
	    float l2 = sqrt(x2*x2 + y2*y2 + z2*z2);
	    float px1 = x1/l1*e1, px2 = x2/l2*e2;
	    float py1 = y1/l1*e1, py2 = y2/l2*e2;
	    float pz1 = z1/l1*e1, pz2 = z2/l2*e2;
	    float e = e1 + e2;
	    float px = px1 + px2;
	    float py = py1 + py2;
	    float pz = pz1 + pz2;
	    float mass = sqrt(e*e - px*px - py*py - pz*pz);

	    for (const auto idx : eRange_idx)
	    {
		h1[eRanges[idx]]["pi0_mass"]->Fill(mass/MeV);
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
		h2[eRanges[idx]]["hit_mul_vs_event_MIP"]->Fill(event_e, hit_mul);
		h2[eRanges[idx]]["eta_vs_event_MIP"]->Fill(event_e, eta);
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

void make_hist(const int run)
{
    string rootFile = cali::getFile(Form("Run%d.myrec.root", run));
    make_hist(rootFile.c_str(), Form("Run%d", run));
}

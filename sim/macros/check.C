#include "cali.h"
#include "caliType.h"

const int colors[] = {kBlack, kRed, kMagenta+2, kBlue, kTeal-6, kGreen+2};
void check(const char *fname = "input.rec.root", 
	  const char*out = "check.root")
{
    gROOT->SetBatch(1);
    gStyle->SetOptStat(0);

    float hit_energy_cut = 0.5;

    TFile *fin = new TFile(fname, "read");
    TTree *tin = (TTree*) fin->Get("events");

    TTreeReader tr(tin);
    TTreeReaderValue<float> T1(tr, "T1");
    TTreeReaderValue<float> T2(tr, "T2");
    TTreeReaderValue<float> T3(tr, "T3");
    TTreeReaderArray<float> hit_e(tr, "CALIHits.e");
    TTreeReaderArray<float> hit_x(tr, "CALIHits.x");
    TTreeReaderArray<float> hit_y(tr, "CALIHits.y");
    TTreeReaderArray<int>   hit_z(tr, "CALIHits.z");

    const double minEventEnergy = 0;
    const double maxEventEnergy = 1000;
    TFile* fout;
    map<string, map<string, TH1F*>> h1;
    map<string, map<string, TH2F*>> h2;

    const char* eRanges[] = {"0MIP", "100MIP", "100-150MIP", "150MIP", "150-300MIP", "300MIP"};

    fout = new TFile(out, "recreate");
    fout->cd();

    for (const auto eRange : eRanges)
    {
	// channel MIP
	h1[eRange]["T1"] = new TH1F(Form("T1_%s", eRange), "T1;MIP", 100, 0, 10);
	h1[eRange]["T2"] = new TH1F(Form("T2_%s", eRange), "T2;MIP", 100, 0, 10);
	h1[eRange]["T3"] = new TH1F(Form("T3_%s", eRange), "T3;MIP", 100, 0, 10);
	// h1[eRange]["T4"] = new TH1F(Form("T4_%s", eRange), "T4;MIP", 100, 0, 10);
	// for (size_t ch=0; ch<cali::nChannels; ch++)
	//     h1[eRange][Form("ch_%zu", ch)] = new TH1F(Form("ch_%zu", ch), Form("ch_%zu;MIP", ch), 100, 0, 10);
	
	// layer energy
	h2[eRange]["layer_energy"] = new TH2F(Form("layer_energy_%s", eRange), "Layer Energy;Layer;Energy (NIP);", 11, 0, 11, 100, 0, 20);
	for (size_t l=0; l<cali::nLayers; l++)
	{
	    h2[eRange][Form("layer_%zu_hit_x_y", l)] = new TH2F(Form("layer_%zu_hit_x_y_%s", l, eRange), Form("Layer %zu Hit;X (cm);Y (cm)", l), 100, -10, 10, 100, -10, 10);
	    h2[eRange][Form("layer_%zu_hit_x_y_weighted", l)] = new TH2F(Form("layer_%zu_hit_x_y_weighted_%s", l, eRange), Form("Layer %zu Hit Weighted by Energy;X (cm);Y (cm)", l), 100, -10, 10, 100, -10, 10);
	}

	h1[eRange]["hit_MIP"] = new TH1F(Form("hit_MIP_%s", eRange), "Hit Energy;MIP", 100, 0, 50);
	h2[eRange]["hit_x_vs_y"] = new TH2F(Form("hit_x_vs_y_%s", eRange), "Hit X vs Y;cm;cm", 100, -10, 10, 100, -10, 10);
	h2[eRange]["hit_x_vs_y_weighted"] = new TH2F(Form("hit_x_vs_y_weighted_%s", eRange), "Hit X vs Y (Weighted by Energy);cm;cm", 100, -10, 10, 100, -10, 10);
	h1[eRange]["hit_x"] = new TH1F(Form("hit_x_%s", eRange), "Hit X;cm", 100, -10, 10);
	h1[eRange]["hit_y"] = new TH1F(Form("hit_y_%s", eRange), "Hit Y;cm", 100, -10, 10);
	h1[eRange]["hit_z"] = new TH1F(Form("hit_z_%s", eRange), "Hit Z;cm", 10, 0, 10);
	h1[eRange]["hit_mul"] = new TH1F(Form("hit_mul_%s", eRange), "Hit Multiplicity", 100, 0, 150);
	h1[eRange]["hit_mul1"] = new TH1F(Form("hit_mul1_%s", eRange), "Hit Multiplicity (0.5 - MIPs)", 100, 0, 100);
	h1[eRange]["hit_mul2"] = new TH1F(Form("hit_mul2_%s", eRange), "Hit Multiplicity (2 - MIPs)", 100, 0, 100);
	h1[eRange]["hit_mul3"] = new TH1F(Form("hit_mul3_%s", eRange), "Hit Multiplicity (5 - MIPs)", 100, 0, 100);
	h1[eRange]["hit_mul4"] = new TH1F(Form("hit_mul4_%s", eRange), "Hit Multiplicity (10 - MIPs)", 100, 0, 100);
	h1[eRange]["event_MIP"] = new TH1F(Form("event_MIP_%s", eRange), "Event Energy;MIP", 100, minEventEnergy, maxEventEnergy);
	h1[eRange]["event_x"] = new TH1F(Form("event_x_%s", eRange), "COG X;cm", 100, -10, 10);
	h1[eRange]["event_y"] = new TH1F(Form("event_y_%s", eRange), "COG Y;cm", 100, -10, 10);
	h1[eRange]["event_z"] = new TH1F(Form("event_z_%s", eRange), "COG Z;layer", 100, 0, cali::nLayers);

	h2[eRange]["event_MIP_vs_hit_mul"] = new TH2F(Form("event_MIP_vs_hit_mul_%s", eRange), "event MIP vs hit mul", 100, 0, 100, 100, minEventEnergy, maxEventEnergy);
	h2[eRange]["event_MIP_vs_eta"] = new TH2F(Form("event_MIP_vs_eta_%s", eRange), "event MIP vs #eta", 100, 2.5, 3.5, 100, minEventEnergy, maxEventEnergy);
	h2[eRange]["x_vs_y"] = new TH2F(Form("x_vs_y_%s", eRange), "X vs Y;cm;cm", 100, -10, 10, 100, -10, 10);
    }

    double e, event_e;
    double event_x, event_y, event_z;
    double layer_e[cali::nLayers];
    double x, y, theta, eta;
    int z;
    int hit_mul, hit_mul1, hit_mul2, hit_mul3, hit_mul4;
    vector<size_t> eRange_idx;

    // const int ne = tin->GetEntries();
    const int ne = 100000;
    for (int ei = 0; ei<ne; ei++)
    {
	tr.Next();
	// if (T1 > 0)
	//     h1["T1"]->Fill(T1);
	// if (T2 > 0)
	//     h1["T2"]->Fill(T2);
	// if (T3 > 0)
	//     h1["T3"]->Fill(T3);
	// if (T4 > 0)
	//     h1["T4"]->Fill(T4);
	// if ((*T3) < 3)
	//     continue;

	hit_mul = hit_e.GetSize();
	hit_mul1 = hit_mul2 = hit_mul3 = hit_mul4 = 0;
	event_e = 0;
	event_x = event_y = event_z = 0;
	for (size_t l=0; l<cali::nLayers; l++)
	    layer_e[l] = 0;

	for (int hi=0; hi<hit_mul; hi++)
	{
	    event_e += hit_e[hi];
	}

	eRange_idx.clear();
	eRange_idx.push_back(0);    // 0MIP
        if (event_e > 100)  // 30 MIP ~ 1 GeV
	{
	    eRange_idx.push_back(1);	// 100MIP
	    if (event_e < 150)  
		eRange_idx.push_back(2);    // 100 - 150 MIP
	    else
	    {
		eRange_idx.push_back(3);    // 150 MIP
		if (event_e < 300)
		    eRange_idx.push_back(4);  // 150 - 300 MIP
		else
		    eRange_idx.push_back(5);  // 300 MIP
	    }
	}

	event_e  = 0;
	for (int hi=0; hi<hit_mul; hi++)
	{
	    e = hit_e[hi];
	    x = hit_x[hi];
	    y = hit_y[hi];
	    z = hit_z[hi];

	    event_e += e;
	    event_x += x*e;
	    event_y += y*e;
	    event_z += z*e;

	    layer_e[z] += e;

	    // h1[Form("ch_%d", ch)]->Fill(e);

	    // if (e > 10)
	    //     hit_mul4++;
	    // else if (e > 5)
	    //     hit_mul3++;
	    // else if (e > 2)
	    //     hit_mul2++;
	    // else if (e > 0.5)
	    //     hit_mul1++; 
	    if (e > 0.5)
		hit_mul1++;
	    if (e > 2)
		hit_mul2++;
	    if (e > 5)
		hit_mul3++;
	    if (e > 10)
		hit_mul4++;

	    for (auto const idx : eRange_idx)
	    {
		string eRange = eRanges[idx];
		h1[eRange]["hit_MIP"]->Fill(e);
		h1[eRange]["hit_x"]->Fill(x/cm);
		h1[eRange]["hit_y"]->Fill(y/cm);
		h1[eRange]["hit_z"]->Fill(z);
		h2[eRange][Form("layer_%d_hit_x_y", z)]->Fill(x/cm, y/cm);
		h2[eRange][Form("layer_%d_hit_x_y_weighted", z)]->Fill(x/cm, y/cm, e);
		h2[eRange]["hit_x_vs_y"]->Fill(x/cm, y/cm);
		h2[eRange]["hit_x_vs_y_weighted"]->Fill(x/cm, y/cm, e);
	    }
	}

	// if (event_e > 100)
	{
	    x = event_x/event_e + cali::x0;
	    y = event_y/event_e + cali::x0;
	    z = event_z/event_e*cali::lt + cali::z0;
	    theta = atan(sqrt(x*x + y*y)/z);
	    eta = -log(tan(theta/2));
	    
	    for (auto const idx : eRange_idx)
	    {
		string eRange = eRanges[idx];
		h1[eRange]["hit_mul"]->Fill(hit_mul);
		h1[eRange]["hit_mul1"]->Fill(hit_mul1);
		h1[eRange]["hit_mul2"]->Fill(hit_mul2);
		h1[eRange]["hit_mul3"]->Fill(hit_mul3);
		h1[eRange]["hit_mul4"]->Fill(hit_mul4);
		h1[eRange]["event_MIP"]->Fill(event_e);
		h1[eRange]["event_x"]->Fill(event_x/event_e/cm);
		h1[eRange]["event_y"]->Fill(event_y/event_e/cm);
		h1[eRange]["event_z"]->Fill(event_z/event_e);
		h2[eRange]["event_MIP_vs_hit_mul"]->Fill(hit_mul, event_e);
		h2[eRange]["event_MIP_vs_eta"]->Fill(eta, event_e);
		h2[eRange]["x_vs_y"]->Fill(event_x/event_e/cm, event_y/event_e/cm);

		for (size_t l=0; l<cali::nLayers; l++)
		    if (layer_e[l] > 0)
			h2[eRange]["layer_energy"]->Fill(l, layer_e[l]);
	    }
	}
    }

    fin->Close();

    TCanvas *c = new TCanvas("c", "c", 800, 600);
    TLegend* l = new TLegend(0.6, 0.6, 0.8, 0.9);
    // plot all h1 
    for (const auto& [var, value] : h1["0MIP"])
    {
	c->Clear();
	l->Clear();

	h1["0MIP"][var]->SetLineColor(colors[0]);
	h1["0MIP"][var]->Draw("HIST");
	l->AddEntry(h1["0MIP"][var], eRanges[0], "l");

	for (size_t idx=1; idx<6; idx++)
	{
	    h1[eRanges[idx]][var]->SetLineColor(colors[idx]);
	    h1[eRanges[idx]][var]->Draw("HIST SAME");
	    l->AddEntry(h1[eRanges[idx]][var], eRanges[idx], "l");
	}
	l->Draw();
	c->SaveAs(Form("%s.png", var.c_str()));
    }

    fout->cd();
    fout->Write();
    fout->Close();
}

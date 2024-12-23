// check various distributions for data
#include "cali.h"
#include "caliType.h"

void check(const char* , const char*);

void check(const int run)
{
    string rootFile = cali::getFile(Form("Run%d.rec.root", run));
    check(rootFile.c_str(), Form("./Run%d_hist.root", run));
}
void check(const char *fname = "input.rec.root", 
	  const char*out = "check.root")
{
    float hit_energy_cut = 0.5;
    string unit = "MIP";

    TFile *fin = new TFile(fname, "read");
    TTree *tin = (TTree*) fin->Get("events");

    TTreeReader tr(tin);
    TTreeReaderArray<float> hit_x(tr, "CALIHits.x");
    TTreeReaderArray<float> hit_y(tr, "CALIHits.y");
    TTreeReaderArray<int>   hit_z(tr, "CALIHits.z");
    TTreeReaderArray<float> hit_e(tr, "CALIHits.e");

    const double minEventEnergy = 0;
    const double maxEventEnergy = 1000;
    TFile* fout;
    map<string, TH1F*> h1;
    map<string, TH2F*> h2;

    fout = new TFile(out, "recreate");
    fout->cd();

    // layer energy
    h2["layer_energy"] = new TH2F("layer_energy", "Layer Energy;Layer;Energy (NIP);", 11, 0, 11, 100, 0, 20);
    for (size_t l=0; l<cali::nLayers; l++)
    {
	h2[Form("layer_%zu_hit_x_y", l)] = new TH2F(Form("layer_%zu_hit_x_y", l), Form("Layer %zu Hit;X (cm);Y (cm)", l), 100, -10, 10, 100, -10, 10);
	h2[Form("layer_%zu_hit_x_y_weighted", l)] = new TH2F(Form("layer_%zu_hit_x_y_weighted", l), Form("Layer %zu Hit Weighted by Energy;X (cm);Y (cm)", l), 100, -10, 10, 100, -10, 10);
    }

    h1["hit_MIP"] = new TH1F("hit_MIP", "Hit Energy;MIP", 100, 0, 50);
    h2["hit_x_vs_y"] = new TH2F("hit_x_vs_y", "Hit X vs Y;cm;cm", 100, -10, 10, 100, -10, 10);
    h2["hit_x_vs_y_weighted"] = new TH2F("hit_x_vs_y_weighted", "Hit X vs Y (Weighted by Energy);cm;cm", 100, -10, 10, 100, -10, 10);
    h1["hit_mul"] = new TH1F("hit_mul", "Hit Multiplicity", 100, 0, 100);
    h1["hit_mul1"] = new TH1F("hit_mul1", "Hit Multiplicity (0.5 - 2 MIPs)", 100, 0, 100);
    h1["hit_mul2"] = new TH1F("hit_mul2", "Hit Multiplicity (2 - 5 MIPs)", 100, 0, 100);
    h1["hit_mul3"] = new TH1F("hit_mul3", "Hit Multiplicity (5 - 10 MIPs)", 100, 0, 100);
    h1["hit_mul4"] = new TH1F("hit_mul4", "Hit Multiplicity (> 10 MIPs)", 100, 0, 100);
    h1["event_MIP"] = new TH1F("event_MIP", "Event Energy;MIP", 100, minEventEnergy, maxEventEnergy);
    h1["event_x"] = new TH1F("event_x", "COG X;cm", 100, -10, 10);
    h1["event_y"] = new TH1F("event_y", "COG Y;cm", 100, -10, 10);
    h1["event_z"] = new TH1F("event_z", "COG Z;layer", 100, 0, cali::nLayers);

    h2["event_MIP_vs_hit_mul"] = new TH2F("event_MIP_vs_hit_mul", "event MIP vs hit mul", 100, 0, 100, 100, minEventEnergy, maxEventEnergy);
    h2["event_MIP_vs_eta"] = new TH2F("event_MIP_vs_eta", "event MIP vs #eta", 100, 2.5, 3.5, 100, minEventEnergy, maxEventEnergy);
    h2["x_vs_y"] = new TH2F("x_vs_y", "X vs Y;cm;cm", 100, -10, 10, 100, -10, 10);

    int ch;
    double e, event_e;
    double event_x, event_y, event_z;
    double layer_e[cali::nLayers];
    double x, y, theta, eta;
    int z;
    int hit_mul, hit_mul1, hit_mul2, hit_mul3, hit_mul4;

    const int ne = tin->GetEntries();
    for (int ei = 0; ei<ne; ei++)
    {
	tr.Next();

	hit_mul = hit_e.GetSize();
	hit_mul1 = hit_mul2 = hit_mul3 = hit_mul4 = 0;
	event_e = 0;
	event_x = event_y = event_z = 0;
	for (size_t l=0; l<cali::nLayers; l++)
	    layer_e[l] = 0;

	for (int hi=0; hi<hit_mul; hi++)
	{
	    e = hit_e[hi];
	    x = hit_x[hi];
	    y = hit_y[hi];
	    z = hit_z[hi];
	    if (e < hit_energy_cut)
	        continue;

	    event_e += e;
	    event_x += x*e;
	    event_y += y*e;
	    event_z += z*e;

	    layer_e[z] += e;

	    // h1[Form("ch_%d", ch)]->Fill(e);

	    if (e > 10)
		hit_mul4++;
	    else if (e > 5)
		hit_mul3++;
	    else if (e > 2)
		hit_mul2++;
	    else if (e > 0.5)
		hit_mul1++; 

	    h1["hit_MIP"]->Fill(e);
	    h2[Form("layer_%d_hit_x_y", z)]->Fill(x/cm, y/cm);
	    h2[Form("layer_%d_hit_x_y_weighted", z)]->Fill(x/cm, y/cm, e);
	    h2["hit_x_vs_y"]->Fill(x/cm, y/cm);
	    h2["hit_x_vs_y_weighted"]->Fill(x/cm, y/cm, e);
	}

	// if (event_e > 100)
	{
	    x = event_x/event_e + cali::x0;
	    y = event_y/event_e + cali::x0;
	    z = event_z/event_e*cali::lt + cali::z0;
	    theta = atan(sqrt(x*x + y*y)/z);
	    eta = -log(tan(theta/2));
	    
	    h1["hit_mul"]->Fill(hit_mul);
	    h1["hit_mul1"]->Fill(hit_mul1);
	    h1["hit_mul2"]->Fill(hit_mul2);
	    h1["hit_mul3"]->Fill(hit_mul3);
	    h1["hit_mul4"]->Fill(hit_mul4);
	    h1["event_MIP"]->Fill(event_e);
	    h1["event_x"]->Fill(event_x/event_e/cm);
	    h1["event_y"]->Fill(event_y/event_e/cm);
	    h1["event_z"]->Fill(event_z/event_e);
	    h2["event_MIP_vs_hit_mul"]->Fill(hit_mul, event_e);
	    h2["event_MIP_vs_eta"]->Fill(eta, event_e);
	    h2["x_vs_y"]->Fill(event_x/event_e/cm, event_y/event_e/cm);

	    for (size_t l=0; l<cali::nLayers; l++)
		if (layer_e[l] > 0)
		    h2["layer_energy"]->Fill(l, layer_e[l]);
	}
    }

    fin->Close();
    fout->cd();
    fout->Write();
    fout->Close();
}

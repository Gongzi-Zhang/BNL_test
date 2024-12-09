#include "calo.h"
#include "cali.h"

void check(const char *fname = "output.hit.root", 
	  const char*out = "check.root")
{
    float hit_energy_cut = 0.5;
    string unit = "MIP";

    float T1, T2, T3, T4;
    struct hit_t {                                                              
	int mul;                                                                
	int ch[192];                                                            
	float e[192];                                                           
    };                                                                          
    hit_t hits;    

    cali::sipmXY pos[cali::channelMax];                                         
    int layerNumber[cali::channelMax];                                          
    for (int ch=0; ch<cali::nChannels; ch++)                                    
    {                                                                           
	layerNumber[ch] = cali::getSipm(ch).layer;                              
	pos[ch] = cali::getSipmXY(ch);                                          
    }               

    TFile *fin = new TFile(fname, "read");
    TTree *tin = (TTree*) fin->Get("events");

    tin->SetBranchAddress("T1", &T1);
    tin->SetBranchAddress("T2", &T2);
    tin->SetBranchAddress("T3", &T3);
    tin->SetBranchAddress("T4", &T4);
    tin->SetBranchAddress("Hits", &hits);
    // TTreeReader tr(tin);
    // TTreeReaderValue<float> T1(tr, "T1");
    // TTreeReaderValue<float> T3(tr, "T3");
    // TTreeReaderValue<int> hit_mul(tr, "hit.mul");
    // TTreeReaderArray<float> hit_energy(tr, "hit.e");

    const double minEventEnergy = 0;
    const double maxEventEnergy = 1000;
    TFile* fout;
    map<string, TH1F*> h1;
    map<string, TH2F*> h2;

    fout = new TFile(out, "recreate");
    fout->cd();

    h1["hit_MIP"] = new TH1F("hit_MIP", "Hit Energy;MIP", 100, 0, 50);
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
    double x, y, z, theta, eta;
    int hit_mul, hit_mul1, hit_mul2, hit_mul3, hit_mul4;
    string eRange;

    const int ne = tin->GetEntries();
    for (int ei = 0; ei<ne; ei++)
    {
	tin->GetEntry(ei);
	// tr.Next();
	hit_mul = hit_mul1 = hit_mul2 = hit_mul3 = hit_mul4 = 0;
	event_e = 0;
	event_x = event_y = event_z = 0;

	for (int hi=0; hi<hits.mul; hi++)
	{
	    ch = hits.ch[hi];
	    if ( (128 <= ch && ch <= 135)   // channel 128 - 135
	      || 5 == ch || 27 == ch || 32 == ch || 50 == ch
	       )
		continue;

	    e = hits.e[hi];
	    // if (e < hit_energy_cut)
	    //     continue;

	    hit_mul++;
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

	    h1["hit_MIP"]->Fill(e);
	}

	if (event_e > 100)
	{
	    x = event_x/event_e + cali::X;
	    y = event_y/event_e + cali::Y;
	    z = event_z/event_e*cali::layerZ + cali::Z;
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
	    h2["event_MIP_vs_hit_mul"]->Fill(hits.mul, event_e);
	    h2["event_MIP_vs_eta"]->Fill(eta, event_e);
	    h2["x_vs_y"]->Fill(event_x/event_e/cm, event_y/event_e/cm);
	}
    }

    fin->Close();
    fout->cd();
    fout->Write();
    fout->Close();
}

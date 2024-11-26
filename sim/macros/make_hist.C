#include "calo.h"
#include "cali.h"

void make_hist(const char *fname = "output.hit.root", 
	  const char*out_prefix = "output")
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

    string eRanges[] = {"150MIP", "150-300MIP", "300-600MIP", "600MIP"};
    map<string, const double> minEventEnergy = {                                
	{"150MIP",	100},                                                   
	// {"100-150MIP",	50},                                                   
	{"150-300MIP",  100},                                                   
	{"300-600MIP",  250},                                                   
	{"600MIP",	550},                                                   
    };                                                                          
    map<string, const double> maxEventEnergy = {                                
	{"150MIP",	1000},                                                   
	// {"100-150MIP",	200},                                                   
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

	h2[eRange]["event_MIP_vs_hit_mul"] = new TH2F("event_MIP_vs_hit_mul", "event MIP vs hit mul", 100, 0, 100, 100, minEventEnergy[eRange], maxEventEnergy[eRange]);
	h2[eRange]["event_MIP_vs_eta"] = new TH2F("event_MIP_vs_eta", "event MIP vs #eta", 100, 2.5, 3.5, 100, minEventEnergy[eRange], maxEventEnergy[eRange]);
	h2[eRange]["x_vs_y"] = new TH2F("x_vs_y", "X vs Y;cm;cm", 100, -10, 10, 100, -10, 10);
    }

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

	// b0_mul = b1_mul = b2_mul = 0;
	// b0_e = b1_e = b2_e = 0;

	// trigger
	// if (T1 < 0.18 || T2 < 0.05 || T3 < 0.6)
	if (T1 < 2.5)
	    continue;

	for (int hi=0; hi<hits.mul; hi++)
	{
	    ch = hits.ch[hi];
	    if ( (128 <= ch && ch <= 135)   // channel 128 - 135
	      || 5 == ch || 27 == ch || 32 == ch || 50 == ch
	       )
		continue;

	    e = hits.e[hi];
	    if (e < hit_energy_cut)
		continue;
	    hit_mul++;
	    event_e += e;
	}

	if (event_e < 150)  // 30 MIP ~ 1 GeV
	    continue;
	// if (event_e < 150)   
	//     eRange = "100-150MIP";
	else if (event_e < 300)   
	    eRange = "150-300MIP";  // 5 - 10 GeV
	else if (event_e < 600)
	    eRange = "300-600MIP";  // 10 - 20 GeV
	else 
	    eRange = "600MIP";	    // 20 GeV -

	hit_mul = 0;
	event_e = 0;
	for (int hi=0; hi<hits.mul; hi++)
	{
	    ch = hits.ch[hi];
	    if ( (128 <= ch && ch <= 135)   // channel 128 - 135
	      || 5 == ch || 27 == ch || 32 == ch || 50 == ch
	       )
		continue;

	    e = hits.e[hi];
	    if (e < hit_energy_cut)
		continue;

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

	    h1["150MIP"]["hit_MIP"]->Fill(e);
	    h1[eRange]["hit_MIP"]->Fill(e);
	}

	{
	    for (const string r : {"150MIP", eRange.c_str()})
	    {
		x = event_x/event_e + cali::X;
		y = event_y/event_e + cali::Y;
		z = event_z/event_e*cali::layerZ + cali::Z;
		theta = atan(sqrt(x*x + y*y)/z);                                    
		eta = -log(tan(theta/2));
		
		h1[r]["hit_mul"]->Fill(hit_mul);
		h1[r]["hit_mul1"]->Fill(hit_mul1);
		h1[r]["hit_mul2"]->Fill(hit_mul2);
		h1[r]["hit_mul3"]->Fill(hit_mul3);
		h1[r]["hit_mul4"]->Fill(hit_mul4);
		h1[r]["event_MIP"]->Fill(event_e);
		h1[r]["event_x"]->Fill(event_x/event_e/cm);
		h1[r]["event_y"]->Fill(event_y/event_e/cm);
		h1[r]["event_z"]->Fill(event_z/event_e);
		h2[r]["event_MIP_vs_hit_mul"]->Fill(hits.mul, event_e);
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

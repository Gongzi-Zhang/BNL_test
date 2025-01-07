#include "cali.h"
#include "calisim.h"

void calibrate(const char *fname = "output.edm4hep.root", 
	  const char*out_name = "output.root")
{
    gROOT->SetBatch(1);

    TFile *fin = new TFile(fname, "read");
    TTree *tin = (TTree*) fin->Get("events");
    TTreeReader tr(tin);

    const char * branch = "CALIHits";
    TTreeReaderArray<unsigned long> hit_cellID(tr, Form("%s.cellID", branch));
    TTreeReaderArray<float> hit_energy(tr, Form("%s.energy", branch));

    cali::sipmXY pos[cali::channelMax];                                         
    int layerNumber[cali::channelMax];                                          
    for (int ch=0; ch<cali::nChannels; ch++)                                    
    {                                                                           
	layerNumber[ch] = cali::getSipm(ch).layer;                              
	pos[ch] = cali::getSipmXY(ch);                                          
    }               

    TFile *fout = new TFile(out_name, "recreate");
    map<string, TH1F*> h1;
    double maxHitEnergy = 30;	// MeV
    double maxEventEnergy = 2000;   // MeV
    h1["hit_energy"] = new TH1F("hit_energy", "Hit Energy;MeV", 100, 0, maxHitEnergy);
    h1["hit_mul"] = new TH1F("hit_mul", "Hit Multiplicity", 200, 0, 200);
    h1["hit_mul1"] = new TH1F("hit_mul1", "Hit Multiplicity (0.5 - 2 MIPs)", 100, 0, 100);
    h1["hit_mul2"] = new TH1F("hit_mul2", "Hit Multiplicity (2 - 5 MIPs)", 100, 0, 100);
    h1["hit_mul3"] = new TH1F("hit_mul3", "Hit Multiplicity (> 5 MIPs)", 100, 0, 100);
    h1["event_energy"] = new TH1F("event_energy", "Event Energy;MeV", 100, 0, maxEventEnergy);
    h1["event_x"] = new TH1F("event_x", "COG X;cm", 100, -10, 10);
    h1["event_y"] = new TH1F("event_y", "COG Y;cm", 100, -10, 10);
    h1["event_z"] = new TH1F("event_z", "COG Z;cm", 150, 0, 15);

    map<string, TH2F*> h2;
    h2["event_energy_vs_hit_mul"] = new TH2F("event_energy_vs_hit_mul", "event energy vs hit mul", 100, 0, maxEventEnergy, 100, 0, 100);
    h2["event_energy_vs_x"] = new TH2F("event_energy_vs_x", "event energy vs x", 100, 0, maxEventEnergy, 100, -10, 10);
    h2["event_energy_vs_y"] = new TH2F("event_energy_vs_y", "event energy vs y", 100, 0, maxEventEnergy, 100, -10, 10);
    h2["event_energy_vs_z"] = new TH2F("event_energy_vs_z", "event energy vs z", 100, 0, maxEventEnergy, 20, 0, 20);

    int ch;
    int hit_mul, hit_mul1, hit_mul2, hit_mul3;
    double e, event_e;
    double event_x, event_y, event_z;

    const int ne = tin->GetEntries();
    for (int ei = 0; ei<ne; ei++)
    {
	tr.Next();
	event_e = 0;
	hit_mul = 0;
	event_x = event_y = event_z = 0;

	for (int hi=0; hi<hit_cellID.GetSize(); hi++)
	{
	    ch = getChId(hit_cellID[hi]);
	    if (ch < 0 || nChannels <= ch)
		continue;

	    hit_mul++;
	    e = hit_energy[hi];
	    event_e += e;
	    event_x += pos[ch].x*e;
	    event_y += pos[ch].y*e;
	    event_z += layerNumber[ch]*e;
	    h1["hit_energy"]->Fill(e/MeV);
	}
	if (event_e > 0)
	{
	    h1["event_energy"]->Fill(event_e/MeV);
	    h1["hit_mul"]->Fill(hit_mul);
	    h1["event_x"]->Fill(event_x/cm);
	    h1["event_y"]->Fill(event_y/cm);
	    h1["event_z"]->Fill(event_z);
	    h2["event_energy_vs_hit_mul"]->Fill(event_e/MeV, hit_mul);
	    h2["event_energy_vs_x"]->Fill(event_e/MeV, event_x/event_e/cm);
	    h2["event_energy_vs_y"]->Fill(event_e/MeV, event_y/event_e/cm);
	    h2["event_energy_vs_z"]->Fill(event_e/MeV, event_z/event_e);
	}
    }

    cout << "INFO\tbeam peaks at " << h1["event_energy"]->GetBinCenter(h1["event_energy"]->GetMaximumBin()) << endl;

    fin->Close();
    fout->cd();
    fout->Write();
    fout->Close();
}

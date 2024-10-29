#include "cali.h"

const int nChannels = 192;
			
void make_hist(const char *, const char*);

void make_hist(const int run)
{
    make_hist(cali::getRootFile(run).c_str(), Form("Run%d_hist.root", run));
}

void make_hist(const char *fname = "output.edm4hep.root", 
	  const char*out_name = "output.root")
{
    float energy_cut = 3;
    string unit = "MIP";

    TFile *fin = new TFile(fname, "read");
    TTree *tin = (TTree*) fin->Get("mip");
    float chMIP[nChannels];
    float eventMIP;
    for (int ch=0; ch<nChannels; ch++)
    {
	TBranch *b = tin->GetBranch(Form("ch_%d", ch));
	b->GetLeaf("LG")->SetAddress(&chMIP[ch]);
    }
    map<string, float> values;
    for(const char* var : {"hit_mul", "event_e"})
    {
	TBranch *b = tin->GetBranch(var);
	b->GetLeaf("LG")->SetAddress(&values[var]);
    }

    TFile *fout = new TFile(out_name, "recreate");
    map<string, TH1F*> h1;
    h1["hit_MIP"] = new TH1F("hit_MIP", "Hit Energy;MIP", 100, 0, 50);
    h1["hit_mul"] = new TH1F("hit_mul", "Hit Multiplicity", 200, 0, 200);
    h1["event_MIP"] = new TH1F("event_MIP", "Event Energy;MIP", 100, 0, 500);
    map<string, TH2F*> h2;
    h2["event_MIP_vs_hit_mul"] = new TH2F("event_MIP_vs_hit_mul", "event MIP vs hit mul", 200, 0, 200, 100, 0, 500);
    // TTree *tout = new TTree("mip", "mip values");
    // double ch_e[nChannels];
    // for (int ch=0; ch<nChannels; ch++)
    // {
    //     tout->Branch(Form("ch_%d", ch), &ch_e[ch]);
    // }

    const int ne = tin->GetEntries();
    int nhits;
    double e;
    for (int ei = 0; ei<ne; ei++)
    {
	tin->GetEntry(ei);

	nhits = 0;
        e = 0;
	for (int ch=0; ch<nChannels; ch++)
	{
	    if (chMIP[ch] < energy_cut)
		continue;
	    nhits++;
	    e += chMIP[ch];
	    h1["hit_MIP"]->Fill(chMIP[ch]);
	}
	if (e > 0)
	// if (nhits > 10)
	{
	    h1["event_MIP"]->Fill(e);
	    h1["hit_mul"]->Fill(nhits);
	    h2["event_MIP_vs_hit_mul"]->Fill(nhits, e);
	}
    }

    fin->Close();
    fout->cd();
    fout->Write();
    fout->Close();
}

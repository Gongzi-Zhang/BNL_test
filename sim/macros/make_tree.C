#include "calisim.h"

void make_tree(const char *fname = "output.edm4hep.root", 
	  const char*out_name = "output.root")
{
    float MIP = 0.495*MeV;
    const char * branch = "CALIRecHits";
    const char * prefix = "sim";

    TFile *fin = new TFile(fname, "read");
    TTree *tin = (TTree*) fin->Get("events");
    TTreeReader tr(tin);

    TTreeReaderArray<unsigned long> hit_cellID(tr, Form("%s.cellID", branch));
    TTreeReaderArray<float> hit_energy(tr, Form("%s.energy", branch));

    TFile *fout = new TFile(out_name, "recreate");
    TTree *tout = new TTree("events", "sim events");
    float T1, T2, T3, T4;
    struct hit_t {
	int mul;
	int ch[192];
	float e[192];
    };
    hit_t hits;

    tout->Branch("T1", &T1);
    tout->Branch("T2", &T2);
    tout->Branch("T3", &T3);
    tout->Branch("T4", &T4);
    tout->Branch("Hits", &hits, "mul/I:ch[192]/I:e[192]/F");

    const int ne = tin->GetEntries();
    int ch = 0;
    double e = 0;
    for (int ei = 0; ei<ne; ei++)
    {
	tr.Next();

	T1 = T2 = T3 = T4 = 0;
	hits.mul = 0;
	for (int hi=0; hi<hit_cellID.GetSize(); hi++)
	{
	    ch = calisim::getChId(hit_cellID[hi]);
	    if (ch == -1024)
		continue;

	    e = hit_energy[hi]/MIP;
	    if	    (ch == -1)    T1 = e;
	    else if (ch == -2)    T2 = e;
	    else if (ch == -9)    T3 = e;
	    else if (ch == -10)   T4 = e;
	    else
	    {
		hits.ch[hits.mul] = ch;
		hits.e[hits.mul] = e;
		hits.mul++;
	    }
	}
	tout->Fill();
	cout << DEBUG << "Entry " << ei << ":\t" << hit_cellID.GetSize() << " vs " << hits.mul << endl;
    }

    fin->Close();
    fout->cd();
    tout->Write();
    fout->Close();
}

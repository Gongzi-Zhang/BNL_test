// check position of reconstructed hits; making sure reconstruction is correct
#include "cali.h"
#include "calisim.h"

void check_rec(const char *fname = "rec.edm4hep.root", 
	  const char*out_name = "output.root")
{
    TFile *fin = new TFile(fname, "read");
    TTree *tin = (TTree*) fin->Get("events");
    TTreeReader tr(tin);

    TTreeReaderArray<unsigned long> rec_hit_cellID(tr, "CALIRawHits.cellID");
    TTreeReaderArray<float> rec_hit_energy(tr, "CALIRawHits.energy");
    TTreeReaderArray<float> rec_hit_x(tr, "CALIRawHits.position.x");
    TTreeReaderArray<float> rec_hit_y(tr, "CALIRawHits.position.y");
    TTreeReaderArray<float> rec_hit_z(tr, "CALIRawHits.position.z");

    TFile *fout = new TFile(out_name, "recreate");
    map<string, TH1F*> h1;
    h1["hit_e_diff"] = new TH1F("hit_e_diff", "hit_e_diff;MIP", 100, -10, 10);

    const int ne = tin->GetEntries();
    for (int ei = 0; ei<ne; ei++)
    {
	tr.Next();

	for (int hi=0; hi<sim_hit_cellID.GetSize(); hi++)
	{
	}
    }

    fin->Close();
    fout->cd();
    h1["hit_e_diff"]->Write();
    fout->Close();
}

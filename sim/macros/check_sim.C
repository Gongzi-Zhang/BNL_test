// check position of simulated hits; making sure they align with real prototype
#include "cali.h"
#include "calisim.h"

void check_sim(const char *fname = "sim.edm4hep.root", 
	  const char*out_name = "check_sim.root")
{
    float MIP = 0.495*MeV;
    const char * prefix = "sim";

    TFile *fin = new TFile(fname, "read");
    TTree *tin = (TTree*) fin->Get("events");
    TTreeReader tr(tin);

    TTreeReaderArray<unsigned long> hit_cellID(tr, "CALIHits.cellID");
    TTreeReaderArray<float> hit_energy(tr, "CALIHits.energy");
    TTreeReaderArray<float> hit_x(tr, "CALIHits.position.x");
    TTreeReaderArray<float> hit_y(tr, "CALIHits.position.y");
    TTreeReaderArray<float> hit_z(tr, "CALIHits.position.z");

    TFile *fout = new TFile(out_name, "recreate");
    map<string, TH1F*> h1;
    h1["hit_x_diff"] = new TH1F("hit_x_diff", "hit_x_diff;mm", 100, -50, 50);
    h1["hit_y_diff"] = new TH1F("hit_y_diff", "hit_y_diff;mm", 100, -50, 50);

    cali::sipmXY pos[cali::channelMax];
    int layerNumber[cali::channelMax];
    for (size_t ch=0; ch<cali::nChannels; ch++)
    {
        pos[ch] = cali::getSipmXY(ch);  
	layerNumber[ch] = cali::getSipm(ch).layer;                              
    }

    const int ne = tin->GetEntries();
    int ch = 0;
    double x, y, z;
    for (int ei = 0; ei<ne; ei++)
    {
	tr.Next();

	for (int hi=0; hi<hit_cellID.GetSize(); hi++)
	{
	    ch = calisim::getChId(hit_cellID[hi]);
	    if (ch < 0)
		continue;

	    x = hit_x[hi];
	    y = hit_y[hi];
	    z = hit_z[hi];
	    h1["hit_x_diff"]->Fill(pos[ch].x + cali::x0 - x);
	    h1["hit_y_diff"]->Fill(pos[ch].y + cali::y0 - y);

	    if (round((z - cali::z0)/cali::lt) - 1 != layerNumber[ch])
		cout << ERROR << "Wrong channel decoding:\t" 
		     << "cellID - " << hit_cellID[hi] << "\t"
		     << "ch - " << ch << "\t" 
		     << "hit z - " << z << "\t"
		     << "channel layer - " << layerNumber[ch] << endl;
	}
    }

    fin->Close();
    fout->cd();
    h1["hit_x_diff"]->Write();
    h1["hit_y_diff"]->Write();
    fout->Close();
}

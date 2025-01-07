// converted the raw hit into customized CALIHits for standalone 
// shower shap analysis
#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TClonesArray.h"

#include "calisim.h"
#include "caliType.h"

void make_raw_tree(const char *fname = "rec.edm4hep.root", 
	  const char*out_name = "output.raw.root")
{
    const float MIP = 0.495*MeV;

    TFile *fin = new TFile(fname, "read");
    TTree *tin = (TTree*) fin->Get("events");
    TTreeReader tr(tin);

    const char* branch = "CALIHits";
    TTreeReaderArray<unsigned long> hit_cellID(tr, Form("%s.cellID", branch));
    TTreeReaderArray<float> hit_energy(tr, Form("%s.energy", branch));

    TFile *fout = new TFile(out_name, "recreate");
    TTree *tout = new TTree("events", "CALIHits for clustering");
    float T1, T2, T3, T4;
    TClonesArray*  hits = new TClonesArray("caliHit");
    tout->Branch("T1", &T1);
    tout->Branch("T2", &T2);
    tout->Branch("T3", &T3);
    tout->Branch("T4", &T4);
    tout->Branch("CALIHits", &hits);

    cali::sipmXY pos[cali::nChannels];
    int layerNumber[cali::nChannels];
    for (size_t ch=0; ch<cali::nChannels; ch++)
    {
	layerNumber[ch] = cali::getSipm(ch).layer;
        pos[ch] = cali::getSipmXY(ch);
    }

    const int ne = tin->GetEntries();
    int ch = 0;
    double e = 0;
    for (int ei = 0; ei<ne; ei++)
    {
	tr.Next();

        T1 = T2 = T3 = T4 = 0;
	size_t nh = 0;
	for (int hi=0; hi<hit_cellID.GetSize(); hi++)
	{
	    ch = calisim::getChId(hit_cellID[hi]);
	    if (ch == -1024 
	    || (128 <= ch && ch <= 135)   // channel 128 - 135                
            || 5 == ch || 27 == ch || 32 == ch || 50 == ch)
		continue;

	    e = hit_energy[hi]/MIP;
	    if	    (ch == -1)	T1 = e;
	    else if (ch == -2)  T2 = e;
	    else if (ch == -9)  T3 = e;
	    else if (ch == -10) T4 = e;
	    else
	    {
		new((*hits)[nh]) caliHit(pos[ch].x, pos[ch].y, layerNumber[ch], e);
                nh++;
	    }
	}
	tout->Fill();
    }

    fin->Close();
    fout->cd();
    tout->Write();
    fout->Close();
}

// converted the reconstructed hit into customized rec hits for standalone 
// shower shap analysis
#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TClonesArray"

#include "calisim.h"
#include "caliType.h"

void make_rec_tree(const char *fname = "output.edm4hep.root", 
	  const char*out_name = "output.root")
{
    const char * prefix = "sim";

    TFile *fin = new TFile(fname, "read");
    TTree *tin = (TTree*) fin->Get("events");
    TTreeReader tr(tin);

    TTreeReaderArray<unsigned long> hit_cellID(tr, "CALIHits.cellID");
    TTreeReaderArray<float> hit_energy(tr, "CALIHits.energy");
    TTreeReaderArray<float> hit_x(tr, "CALIHits.x");
    TTreeReaderArray<float> hit_y(tr, "CALIHits.y");
    TTreeReaderArray<float> hit_z(tr, "CALIHits.z");

    TFile *fout = new TFile(out_name, "recreate");
    TTree *tout = new TTree("hits", "reconstructed CALIHits");
    TClonesArray*  hits = new TClonesArray("caliHit");
    tout->Branch("CALIHits", &hits);

    cali::sipmXY pos[cali::nChannels];
    int layerNumber[cali::nChannels];
    for (size_t ch=0; ch<cali::nChannels; ch++)
    {
	layerNumber[ch] = cali::getSipm(ch).layer;
        pos[ch] = cali::getSipmXY(ch)
    }

    const int ne = tin->GetEntries();
    int ch = 0;
    double e = 0;
    for (int ei = 0; ei<ne; ei++)
    {
	tr.Next();

	size_t nh = 0;
	for (int hi=0; hi<hit_cellID.GetSize(); hi++)
	{
	    ch = calisim::getChId(hit_cellID[hi]);
	    if (ch < 0 
	    || (128 <= ch && ch <= 135)   // channel 128 - 135                
            || 5 == ch || 27 == ch || 32 == ch || 50 == ch)
		continue;

	    e = hit_energy[hi];
	    if (e > 0)
	    {
		new((*hits)[nh]) caliHit(x, y, layerNumber[ch], e);
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

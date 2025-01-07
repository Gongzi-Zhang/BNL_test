// check position of reconstructed hits; making sure reconstruction is correct
#include "cali.h"
#include "calisim.h"
#include "caliType.h"

const int colors[] = {kBlack, kRed};
const double Y1[] = {0.5, 0.7};
const double height = 0.2;

void check_myrec(TString fname = "rec.edm4hep.root", 
	  const char*out_name = "output.root")
{
    gROOT->SetBatch(1);

    TFile *fin = new TFile(fname, "read");
    TTree *tin = (TTree*) fin->Get("events");

    TString fname_myrec = fname;
    fname_myrec.Replace(fname_myrec.Index("rec_"), 4, "");
    fname_myrec.Replace(fname_myrec.Index("edm4hep"), 7, "myrec");
    TFile *fin_myrec = new TFile(fname_myrec, "read");
    TTree *tin_myrec = (TTree*) fin_myrec->Get("events");
    tin->AddFriend(tin_myrec, "myrec");

    TTreeReader tr(tin);

    // TTreeReaderArray<unsigned long> rec_hit_cellID(tr, "CALIRecHits.cellID");
    TTreeReaderArray<float> rec_hit_e(tr, "CALIRecHits.energy");
    TTreeReaderArray<float> rec_hit_x(tr, "CALIRecHits.position.x");
    TTreeReaderArray<float> rec_hit_y(tr, "CALIRecHits.position.y");
    TTreeReaderArray<float> rec_hit_z(tr, "CALIRecHits.position.z");
    // TTreeReaderArray<unsigned long> hit_cellID(tr, "CALIHits.cellID");
    TTreeReaderArray<float> myrec_hit_e(tr, "myrec.CALIHits.e");
    TTreeReaderArray<float> myrec_hit_x(tr, "myrec.CALIHits.x");
    TTreeReaderArray<float> myrec_hit_y(tr, "myrec.CALIHits.y");
    TTreeReaderArray<int> myrec_hit_z(tr, "myrec.CALIHits.z");

    TFile *fout = new TFile(out_name, "recreate");
    map<string, map<string, TH1F*>> h1;
    for (const auto var : {"rec", "myrec"} )
    {
	h1[var]["hit_e"]   = new TH1F(Form("%s_hit_e", var), "hit_e;MIP", 100, 0, 200);
	h1[var]["hit_mul"] = new TH1F(Form("%s_hit_mul", var), "hit_mul", 200, 0, 200);
	h1[var]["hit_x"]   = new TH1F(Form("%s_hit_x", var), "hit_x;cm", 100, -10, 10);
	h1[var]["hit_y"]   = new TH1F(Form("%s_hit_y", var), "hit_y;cm", 100, -10, 10);
	h1[var]["hit_z"]   = new TH1F(Form("%s_hit_z", var), "hit_z", 10, 0, 10);
	// h1[var]["event_e"] = new TH1F(Form("%s_event_e", var), "event_e;MIP", 100, 0, 1000);
	// h1[var]["event_x"] = new TH1F(Form("%s_event_x", var), "event_x;cm", 100, -10, 10);
	// h1[var]["event_y"] = new TH1F(Form("%s_event_y", var), "event_y;cm", 100, -10, 10);
	// h1[var]["event_z"] = new TH1F(Form("%s_event_z", var), "event_z", 10, 0, 10);
    }

    const int ne = tin->GetEntries();
    float x, y, z, e;
    // float event_e, event_x, event_y, event_z;
    for (int ei = 0; ei<ne; ei++)
    {
	tr.Next();

	// raw hits
	// event_e = event_x = event_y = event_z = 0;
	if (rec_hit_e.GetSize() != myrec_hit_e.GetSize())
	{
	    cerr << ERROR << "Different number of hits in entry " << ei << ":\t"
		 << rec_hit_e.GetSize() << " vs " << myrec_hit_e.GetSize() << endl;
	    continue;
	}
	for (int hi=0; hi<rec_hit_e.GetSize(); hi++)
	{
	    // e = hit_e[hi];
	    // x = hit_x[hi] - cali::x0;
	    // y = hit_y[hi] - cali::y0;
	    // z = (hit_z[hi] - cali::z0)/cali::lt;
	    // event_e += e;
	    // event_x += x*e;
	    // event_y += y*e;
	    // event_z += z*e;

	    h1["rec"]["hit_e"]->Fill(rec_hit_e[hi]);
	    h1["rec"]["hit_x"]->Fill(rec_hit_x[hi]/cm);
	    h1["rec"]["hit_y"]->Fill(rec_hit_y[hi]/cm);
	    // h1["rec"]["hit_z"]->Fill((rec_hit_z[hi] - cali::z0)/cali::lt - 1);
	    h1["myrec"]["hit_e"]->Fill(myrec_hit_e[hi]);
	    h1["myrec"]["hit_x"]->Fill(myrec_hit_x[hi]/cm);
	    h1["myrec"]["hit_y"]->Fill(myrec_hit_y[hi]/cm);
	    // h1["myrec"]["hit_z"]->Fill(myrec_hit_z[hi]);
	}
	h1["rec"]["hit_mul"]->Fill(rec_hit_e.GetSize());
	h1["myrec"]["hit_mul"]->Fill(myrec_hit_e.GetSize());
    }

    fin_myrec->Close();
    fin->Close();

    TCanvas* c = new TCanvas("c", "c", 800, 600);
    for (const auto var : {"hit_e", "hit_x", "hit_y", "hit_z", "hit_mul"})
    {
	c->Clear();
	h1["rec"][var]->SetLineColor(kBlack);
	h1["myrec"][var]->SetLineColor(kRed);
	h1["rec"][var]->Draw("HIST");
	h1["myrec"][var]->Draw("HIST SAMES");
	c->Update();

	int i=0;
	for (const char* name : {"rec", "myrec"})
        {
            TPaveStats *st = (TPaveStats*) h1[name][var]->FindObject("stats");
            st->SetTextColor(colors[i]);
            st->SetY1NDC(Y1[i]);
            st->SetY2NDC(Y1[i] + height);
            i++;
        }

	c->SaveAs(Form("%s.png", var));
    }
    fout->cd();
    fout->Write();
    fout->Close();
}

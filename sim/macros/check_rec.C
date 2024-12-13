// check position of reconstructed hits; making sure reconstruction is correct
#include "cali.h"
#include "calisim.h"

const int colors[] = {kBlack, kRed};
const double Y1[] = {0.5, 0.7};
const double height = 0.2;

void check_rec(const char *fname = "rec.edm4hep.root", 
	  const char*out_name = "output.root")
{
    gROOT->SetBatch(1);

    const double MIP = 0.495*MeV;

    TFile *fin = new TFile(fname, "read");
    TTree *tin = (TTree*) fin->Get("events");
    TTreeReader tr(tin);

    // TTreeReaderArray<unsigned long> hit_cellID(tr, "CALIHits.cellID");
    TTreeReaderArray<float> hit_energy(tr, "CALIHits.energy");
    TTreeReaderArray<float> hit_x(tr, "CALIHits.position.x");
    TTreeReaderArray<float> hit_y(tr, "CALIHits.position.y");
    TTreeReaderArray<float> hit_z(tr, "CALIHits.position.z");
    // TTreeReaderArray<unsigned long> rec_hit_cellID(tr, "CALIRecHits.cellID");
    TTreeReaderArray<float> rec_hit_energy(tr, "CALIRecHits.energy");
    TTreeReaderArray<float> rec_hit_x(tr, "CALIRecHits.position.x");
    TTreeReaderArray<float> rec_hit_y(tr, "CALIRecHits.position.y");
    TTreeReaderArray<float> rec_hit_z(tr, "CALIRecHits.position.z");

    TFile *fout = new TFile(out_name, "recreate");
    map<string, map<string, TH1F*>> h1;
    for (const auto var : {"raw", "rec"} )
    {
	h1[var]["hit_e"]   = new TH1F(Form("%s_hit_e", var), "hit_e;MIP", 100, 0, 200);
	h1[var]["hit_mul"] = new TH1F(Form("%s_hit_mul", var), "hit_mul", 200, 0, 200);
	h1[var]["hit_x"]   = new TH1F(Form("%s_hit_x", var), "hit_x;cm", 100, -10, 10);
	h1[var]["hit_y"]   = new TH1F(Form("%s_hit_y", var), "hit_y;cm", 100, -10, 10);
	h1[var]["hit_z"]   = new TH1F(Form("%s_hit_z", var), "hit_z", 10, 0, 10);
	h1[var]["event_e"] = new TH1F(Form("%s_event_e", var), "event_e;MIP", 100, 0, 1000);
	h1[var]["event_x"] = new TH1F(Form("%s_event_x", var), "event_x;cm", 100, -10, 10);
	h1[var]["event_y"] = new TH1F(Form("%s_event_y", var), "event_y;cm", 100, -10, 10);
	h1[var]["event_z"] = new TH1F(Form("%s_event_z", var), "event_z", 10, 0, 10);
    }

    const int ne = tin->GetEntries();
    float x, y, z, e;
    float event_e, event_x, event_y, event_z;
    for (int ei = 0; ei<ne; ei++)
    {
	tr.Next();

	// raw hits
	event_e = event_x = event_y = event_z = 0;
	for (int hi=0; hi<hit_energy.GetSize(); hi++)
	{
	    e = hit_e[hi];
	    x = hit_x[hi];
	    y = hit_y[hi];
	    z = hit_z[hi];
	    event_e += e;
	    event_x += x*e;
	    event_y += y*e;
	    event_z += z*e;

	    h1["raw"]["hit_e"]->Fill(e/MIP);
	    h1["raw"]["hit_x"]->Fill(x/cm);
	    h1["raw"]["hit_y"]->Fill(y/cm);
	    h1["raw"]["hit_z"]->Fill(z/cm);
	}
	h1["raw"]["hit_mul"]->Fill(hit_energy.GetSize());
	h1["raw"]["event_e"]->Fill(event_e/MIP);
	h1["raw"]["event_x"]->Fill(event_x/event_e/cm);
	h1["raw"]["event_y"]->Fill(event_y/event_e/cm);
	h1["raw"]["event_z"]->Fill(event_z/event_e/cm);

	// rec hits
	event_e = event_x = event_y = event_z = 0;
	for (int hi=0; hi<rec_hit_energy.GetSize(); hi++)
	{
	    e = rec_hit_e[hi];
	    x = rec_hit_x[hi];
	    y = rec_hit_y[hi];
	    z = rec_hit_z[hi];
	    event_e += e;
	    event_x += x*e;
	    event_y += y*e;
	    event_z += z*e;

	    h1["rec"]["hit_e"]->Fill(e/MIP);
	    h1["rec"]["hit_x"]->Fill(x/cm);
	    h1["rec"]["hit_y"]->Fill(y/cm);
	    h1["rec"]["hit_z"]->Fill(z/cm);
	}
	h1["rec"]["hit_mul"]->Fill(rec_hit_energy.GetSize());
	h1["rec"]["event_e"]->Fill(event_e/MIP);
	h1["rec"]["event_x"]->Fill(event_x/event_e/cm);
	h1["rec"]["event_y"]->Fill(event_y/event_e/cm);
	h1["rec"]["event_z"]->Fill(event_z/event_e/cm);
    }

    fin->Close();

    TCanvas* c = new TCanvas("c", "c", 800, 600);
    for (const auto var : {"hit_e", "hit_x", "hit_y", "hit_z", "hit_mul", "event_e", "event_x", "event_y", "event_z"})
    {
	c->Clear();
	h1["raw"][var]->SetLineColor(kBlack);
	h1["rec"][var]->SetLineColor(kRed);
	h1["raw"][var]->Draw("HIST");
	h1["rec"][var]->Draw("HIST SAMES");
	c->Update();

	int i=0;
	for (const char* name : {"raw", "rec"})
        {
            TPaveStats *st = (TPaveStats*) h1[name][var]->FindObject("stats");
            st->SetTextColor(colors[i]);
            st->SetY1NDC(Y1[i]);
            st->SetY2NDC(Y1[i] + height);
            i++;
        }

	c->SaveAs(Form("%s.png", var));
    }
    // fout->cd();
    // fout->Close();
}

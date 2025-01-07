#include "utilities.h"

void compare_hepmc(const char* root1, const char* root2, const char*l1 = "1", const char* l2 = "2") 
{
    gROOT->SetBatch(1);
    TFile *fin1 = new TFile(root1, "read");
    if (!fin1)
    {
	cerr << ERROR << "file doesn't exist: " << root1 << endl;
	return;
    }
    TFile *fin2 = new TFile(root2, "read");
    if (!fin2)
    {
	cerr << ERROR << "file doesn't exist: " << root2 << endl;
	return;
    }

    TCanvas *c = new TCanvas("c", "c", 800, 600);
    TLegend *l = new TLegend(0.7, 0.7, 0.85, 0.88);
    TH1F *h1, *h2;
    const char* vars[] = {"mul", "eta", "e", "pi_e", "gamma_e", "pid"};
    for (TString var : vars)
    {
	c->Clear();
	l->Clear();
	if ("e" == var || "pi_e" == var || "gamma_e" == var)
	    c->SetLogy(1);
	else
	    c->SetLogy(0);

	h1 = (TH1F*) fin1->Get(var);
	h2 = (TH1F*) fin2->Get(var);
	h1->SetStats(0);
	h2->SetStats(0);
	h1->Scale(1/h1->Integral());
	h2->Scale(1/h2->Integral());
	h1->SetLineColor(kRed);
	h2->SetLineColor(kBlack);
	if (h1->GetMaximum() > h2->GetMaximum())
	{
	    h1->Draw("HIST");
	    h2->Draw("HIST SAME");
	}
	else
	{
	    h2->Draw("HIST");
	    h1->Draw("HIST SAME");
	}
	l->AddEntry(h1, l1, "l");
	l->AddEntry(h2, l2, "l");
	l->Draw();
	c->SaveAs(Form("%s.png", var.Data()));
    }
}

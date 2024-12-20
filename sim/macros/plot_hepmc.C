#include "cali_style.h"

void plot_hepmc()
{
    gROOT->SetBatch(1);
    cali_style();

    TCanvas *c = new TCanvas("c", "c", 800, 600);

    // pid
    const char* hepmc_dir = "/gpfs02/eic/wbzhang/epic/BNL_test/sim/freeze/roots/hepmc";
    TFile *fin = new TFile(Form("%s/HE_1GeV_T3_hist.root", hepmc_dir), "read");
    TH1F* pid = (TH1F*) fin->Get("pid");
    pid->SetStats(0);
    pid->Scale(1/pid->Integral());
    pid->SetTitle(";Particle ID;Count");
    pid->Draw("B");
    c->SaveAs("pid.pdf");
    fin->Close();

    // energy
    c->Clear();
    c->SetLogy(1);
    const char* files[] = {"HE_1GeV_gamma", "HE_1GeV_piminus", "HE_1GeV_proton", "HE_1GeV_neutron"};
    const char* legend_labels[] = {"photon", "pi-", "proton", "neutron"};
    int count = 0;
    TLegend *l = new TLegend(0.65, 0.63, 0.83, 0.88);
    for (auto file : files)
    {
	TFile *f = new TFile(Form("%s/%s_hist.root", hepmc_dir, file), "read");
	TH1F* e = (TH1F*) f->Get("e");
	e->Scale(1/e->Integral());
	e->SetStats(0);
	e->SetTitle(";Particle Energy [GeV];Count");
	e->SetLineColor(colors[count]);
	e->SetMarkerStyle(markers[count]);
	e->SetMarkerColor(colors[count]);
	c->cd();
	if (0 == count)
	    e->Draw("HIST P");
	else
	    e->Draw("HIST P SAME");
	l->AddEntry(e, legend_labels[count], "l");
	count++;
    }
    l->Draw();
    c->SaveAs("particle_e.pdf");
}

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
    pid->SetFillStyle(3004);
    pid->SetFillColor(kBlue);
    pid->SetLineColor(kBlue);
    pid->GetXaxis()->SetTitleSize(0.05);
    pid->GetXaxis()->SetLabelSize(0.04);
    pid->GetXaxis()->SetTitleOffset(0.9);
    pid->GetYaxis()->SetTitleSize(0.05);
    pid->GetYaxis()->SetLabelSize(0.04);
    pid->GetYaxis()->SetTitleOffset(0.9);
    pid->Draw("HIST");

    TLatex *latex = new TLatex();                                   
    latex->SetTextSize(0.045);                                       
    latex->SetTextAlign(22);   
    latex->SetTextColor(kBlack);
    latex->DrawLatexNDC(0.7, 0.83, "Pythia8 pp 200 GeV");
    latex->DrawLatexNDC(0.7, 0.76, "3.1 < #eta < 3.4");

    c->SaveAs("pid.pdf");
    fin->Close();

    // energy
    colors[0] = kBlack;
    c->Clear();
    c->SetLogy(1);
    const char* files[] = {"HE_1GeV_gamma", "HE_1GeV_piminus", "HE_1GeV_proton", "HE_1GeV_neutron"};
    const char* legend_labels[] = {"photon", "pi-", "proton", "neutron"};
    int count = 0;
    TLegend *l = new TLegend(0.7, 0.7, 0.9, 0.9);
    l->SetTextSize(0.045);
    for (auto file : files)
    {
	TFile *f = new TFile(Form("%s/%s_hist.root", hepmc_dir, file), "read");
	TH1F* e = (TH1F*) f->Get("e");
	e->Scale(1/e->Integral());
	e->Rebin(2);
	e->SetStats(0);
	e->SetTitle(";Particle Energy [GeV];Count");
	e->SetLineColor(colors[count]);
	e->SetMarkerStyle(markers[count]);
	e->SetMarkerColor(colors[count]);
	e->GetXaxis()->SetTitleSize(0.05);
	e->GetXaxis()->SetLabelSize(0.04);
	e->GetXaxis()->SetTitleOffset(0.9);
	e->GetYaxis()->SetTitleSize(0.05);
	e->GetYaxis()->SetLabelSize(0.04);
	e->GetYaxis()->SetTitleOffset(0.9);
	c->cd();
	if (0 == count)
	    e->Draw("HIST");
	else
	    e->Draw("HIST SAME");
	l->AddEntry(e, legend_labels[count], "l");
	count++;
    }
    l->Draw();
    c->SaveAs("particle_e.pdf");
}

void plot_sim()
{
    gROOT->SetBatch(1);
    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);

    TCanvas *c = new TCanvas("c", "c", 800, 600);

    const char* dir = "/gpfs02/eic/wbzhang/epic/BNL_test/sim/work/";
    const char* files[] = {"HE_1GeV_gamma", "HE_1GeV_piminus", "HE_1GeV_proton", "HE_1GeV_neutron"};
    TFile *fins[4];
    for (int i=0; i<4; i++)
    {
	fins[i] = new TFile(Form("%s/%s_hit_0.5MIP_event_150MIP_hist.root", dir, files[i]), "read");
    }

    const char* legend_labels[] = {"photon", "pi-", "proton", "neutron"};
    int colors[] = {kBlack, kRed, kBlue, kViolet};
    TLegend *l = new TLegend(0.6, 0.6, 0.8, 0.85);
    l->SetLineColor(0);
    l->SetLineStyle(0);
    l->SetFillStyle(0);
    for (auto var : {"event_MIP", "hit_MIP", "hit_mul"})
    {
	c->Clear();
	l->Clear();
	if (strcmp(var, "hit_mul") == 0)
	    c->SetLogy(0);
	else
	    c->SetLogy(1);
	for (int i=0; i<4; i++)
	{
	    TH1F* h1 = (TH1F*) fins[i]->Get(var);
	    h1->Scale(1/h1->Integral());
	    h1->SetStats(0);
	    h1->SetYTitle("Count");
	    c->cd();
	    h1->SetLineColor(colors[i]);
	    if (0 == i)
		h1->Draw("HIST");
	    else
		h1->Draw("HIST SAME");
	    l->AddEntry(h1, legend_labels[i], "l");
	}
	l->Draw();
	c->SaveAs(Form("sim_%s.pdf", var));
    }
}

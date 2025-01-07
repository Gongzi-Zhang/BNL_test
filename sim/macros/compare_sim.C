void compare_sim(const char*name1, const char* name2, const char* l1 = "1", const char* l2 = "2") 
{
    gROOT->SetBatch(1);
    TCanvas *c = new TCanvas("c", "c", 800, 600);

    TFile *fins[2];
    fins[0] = new TFile(name1, "read");
    fins[1] = new TFile(name2, "read");

    const char* legend_labels[] = {l1, l2};

    int colors[] = {kBlack, kRed, kBlue, kCyan};
    TLegend *l = new TLegend(0.7, 0.6, 0.9, 0.85);
    for (auto var : {"event_MIP", "hit_MIP", "hit_mul"})
    {
	c->Clear();
	l->Clear();
	if (strcmp(var, "hit_mul") == 0)
	    c->SetLogy(0);
	else
	    c->SetLogy(1);
	for (int i=0; i<2; i++)
	{
	    TH1F* h1 = (TH1F*) fins[i]->Get(var);
	    h1->Scale(1/h1->Integral());
	    h1->SetStats(0);
	    c->cd();
	    h1->SetLineColor(colors[i]);
	    if (0 == i)
		h1->Draw("HIST");
	    else
		h1->Draw("HIST SAME");
	    l->AddEntry(h1, legend_labels[i], "l");
	}
	l->Draw();
	c->SaveAs(Form("sim_%s.png", var));
    }
}

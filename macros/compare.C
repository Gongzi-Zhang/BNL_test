const int colors[] = {kOrange, kViolet};
const double Y1[] = {0.5, 0.7};
const double height = 0.2;

void compare(const char* f1, const char* f2, const char* name1 = "data", const char* name2 = "sim", const char *prefix = NULL, const char* cut = "")
{
    gROOT->SetBatch(1);
    gStyle->SetOptStat(0);
    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);

    map<string, TFile *> fin;
    fin[name1] = new TFile(f1, "read");
    if (!fin[name1] || !fin[name1]->IsOpen())
    {
	cerr << "Can't open root file: " << name1 << endl;
	exit(1);
    }
    fin[name2] = new TFile(f2, "read");
    if (!fin[name2] || !fin[name2]->IsOpen())
    {
	cerr << "Can't open root file: " << name2 << endl;
	exit(1);
    }

    TCanvas* c = new TCanvas("c", "c", 800, 600);
    TLegend* l = new TLegend(0.6, 0.75, 0.8, 0.85);
    l->SetLineColor(0);	// transparent
    l->SetLineStyle(0);
    l->SetFillStyle(0);
    c->SetLogy(1);
    map<string, TH1F *> h;
    for (const char* var : {"hit_MIP", "event_MIP", "hit_mul", "hit_mul1", "hit_mul2", "hit_mul3", "hit_mul4", "event_x", "event_y", "event_z", "clu_mul", "clu_MIP", "clu_x", "clu_y", "clu_z", "clu_nhits", "pi0_mass"})
    {
	c->Clear();
	l->Clear();
	if (strcmp(var, "hit_MIP") == 0 || strcmp(var, "event_MIP") == 0)
	    c->SetLogy(1);
	else
	    c->SetLogy(0);
	size_t i = 0;
	for (const char* name : {name1, name2})
	{
	    h[name] = (TH1F*) fin[name]->Get(var);
	    h[name]->SetName(name);
	    h[name]->SetLineColor(colors[i]);
	    h[name]->Scale(1/h[name]->Integral());
	    i++;
	}

	if (h[name1]->GetMaximum() > h[name2]->GetMaximum())
	{
	    h[name1]->Draw("HIST");
	    h[name2]->Draw("HIST sames");
	}
	else
	{
	    h[name2]->Draw("HIST");
	    h[name1]->Draw("HIST sames");
	}
	c->Update();

	l->AddEntry(h[name1], name1, "l");
	l->AddEntry(h[name2], name2, "l");
	l->SetTextSize(0.04);

	// i = 0;
	// for (const char* name : {name1, name2})
	// {
	//     TPaveStats *st = (TPaveStats*) h[name]->FindObject("stats");
	//     st->SetTextColor(colors[i]);
	//     st->SetY1NDC(Y1[i]);
	//     st->SetY2NDC(Y1[i] + height);
	//     i++;
	// }

	// TText *tcut = new TText(0.5, 0.92, cut);
	// tcut->SetNDC(true);
	// tcut->SetTextSize(0.03);
	// tcut->SetTextColor(kRed-2);
	// tcut->SetTextAlign(22);
	// tcut->Draw();

	l->Draw();

	if (prefix)
	    c->SaveAs(Form("%s_%s.pdf", prefix, var));
	else
	    c->SaveAs(Form("%s.pdf", var));
    }

    map<string, TH2F *> h2;
    TCanvas* c1 = new TCanvas("c1", "c1", 1200, 600);
    c1->Divide(2, 1);
    map<string, string> title = {
	{"event_MIP_vs_hit_mul", ";Hit Multiplicity;Event Energy [MIP]"},
	{"clu_x_vs_y", "Cluster Position;X [cm];Y [cm]"},
	{"clu_x_vs_y_weighted", "Energy Weighted Cluster Position;X [cm];Y [cm]"},
	{"clu_e_vs_x", "Cluster Energy vs X;X [cm]; E [MIP]"},
	{"clu_e_vs_y", "Cluster Energy vs Y;Y [cm]; E [MIP]"},
	{"clu_e_vs_z", "Cluster Energy vs Z;Z [cm]; E [MIP]"},
    };
    for(const char* var : {"event_MIP_vs_hit_mul", "clu_x_vs_y", "clu_x_vs_y_weighted", 
	    "clu_e_vs_x", "clu_e_vs_y", "clu_e_vs_z"}) 
    {
	size_t i = 0;
	for (const char* name : {name1, name2})
	{
	    h2[name] = (TH2F*) fin[name]->Get(var);
	    h2[name]->SetName(name);
	    h2[name]->SetTitle(title[var].c_str());
	    h2[name]->SetLineColor(colors[i]);
	    i++;
	}
	c1->cd(1);
	h2[name1]->Draw("colz");

	c1->cd(2);
	h2[name2]->Draw("colz");

	if (prefix)
	    c1->SaveAs(Form("%s_%s.pdf", prefix, var));
	else
	    c1->SaveAs(Form("%s.pdf", var));
    }
}

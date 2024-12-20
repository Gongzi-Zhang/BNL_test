#include "cali_style.C"

const double Y1[] = {0.5, 0.7};
const double height = 0.2;

void compare(const char* f1, const char* f2, const char* name1 = "data", const char* name2 = "sim", const char *prefix = NULL, const char* title = "")
{
    gROOT->SetBatch(1);
    cali_style();
    gStyle->SetOptStat(0);

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
    map<string, const char*> xtitle = 
    {
	{"hit_MIP", "Hit Energy [MIP]"},
	{"event_MIP", "Event Energy [MIP]"},
	{"hit_mul", "Hit Multiplicity"},
	{"hit_mul1", "Hit Multiplicity (0.5 - 2 MIP)"},
	{"hit_mul2", "Hit Multiplicity (2 - 5 MIP)"},
	{"hit_mul3", "Hit Multiplicity (5 - 10 MIP)"},
	{"hit_mul4", "Hit Multiplicity (> 10 MIP)"},
	{"event_x", "Event COG X [cm]"},
	{"event_y", "Event COG Y [cm]"},
	{"event_z", "Event COG Z"},
	{"clu_mul", "Cluster Multiplicity"},
	{"clu_MIP", "Cluster Energy [MIP]"},
	{"clu_x", "Cluster COG X [cm]"},
	{"clu_y", "Cluster COG Y [cm]"},
	{"clu_z", "Cluster COG Z"},
	{"clu_nhits", "Cluster # of Hits"},
	{"pi0_mass", "M_{inv} [MeV]"},
    };
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
	    h[name]->SetTitle(Form("%s;%s;Count", title, xtitle[var]));
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
    map<string, const char*> XYtitle = {
	{"event_MIP_vs_hit_mul", "Hit Multiplicity;Event Energy [MIP]"},
	{"clu_x_vs_y", "Cluster COG X [cm];Cluster COG Y [cm]"},
	{"clu_x_vs_y_weighted", "Energy Weighted Cluster COG X [cm]; Energy Weighted Cluster COG Y [cm]"},
	{"clu_e_vs_x", "Cluster COG X [cm]; Cluster Energy [MIP]"},
	{"clu_e_vs_y", "Cluster COG Y [cm]; Cluster Energy [MIP]"},
	{"clu_e_vs_z", "Cluster COG Z [cm]; Cluster Energy [MIP]"},
    };
    map<string, const char*> dataset = {
	{name1, "data"},
	{name2, "sim"},
    };
    for(const char* var : {"event_MIP_vs_hit_mul", "clu_x_vs_y", "clu_x_vs_y_weighted", 
	    "clu_e_vs_x", "clu_e_vs_y", "clu_e_vs_z"}) 
    {
	size_t i = 0;
	for (const char* name : {name1, name2})
	{
	    h2[name] = (TH2F*) fin[name]->Get(var);
	    h2[name]->SetName(name);
	    h2[name]->SetTitle(Form("%s (%s);%s", title, dataset[name], XYtitle[var]));
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

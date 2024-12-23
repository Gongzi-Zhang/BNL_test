#include "cali_style.h"

const double Y1[] = {0.5, 0.7};
const double height = 0.2;

void compare(const char* f1, const char* f2, const char* name1 = "data", const char* name2 = "sim", const char *prefix = NULL, const char* title = "T1")
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
    c->SetTopMargin(0.08);
    c->SetBottomMargin(0.13);
    c->SetLeftMargin(0.13);
    TLegend* l = new TLegend(0.55, 0.75, 0.75, 0.85);
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
	    h[name]->GetXaxis()->SetTitleSize(0.07);
	    h[name]->GetXaxis()->SetTitleOffset(0.8);
	    h[name]->GetXaxis()->SetLabelSize(0.05);
	    h[name]->GetYaxis()->SetTitleSize(0.07);
	    h[name]->GetYaxis()->SetTitleOffset(1.0);
	    h[name]->GetYaxis()->SetLabelSize(0.05);
	    i++;
	}

	// rebin pi0 mass histogram
	if (strcmp(var, "pi0_mass") == 0)
	{
	    h[name1]->Rebin(2);
	    h[name2]->Rebin(2);
	}

	string name;
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

	TPaveText* pt = (TPaveText*) c->GetPrimitive("title");
	// update title
	pt->SetTextSize(0.07);
	c->Modified();

	l->AddEntry(h[name1], name1, "l");
	l->AddEntry(h[name2], name2, "l");
	l->SetTextSize(0.055);

	l->Draw();

	if (strcmp(var, "pi0_mass") == 0)
	{
	    TLine *l_pi0 = new TLine(135, gPad->GetUymin(), 135, gPad->GetUymax());
	    l_pi0->SetLineColor(kRed);
	    l_pi0->SetLineWidth(2);
	    l_pi0->Draw();
	}
	if (prefix)
	    c->SaveAs(Form("%s_%s.pdf", prefix, var));
	else
	    c->SaveAs(Form("%s.pdf", var));
    }

    map<string, TH2F *> h2;
    TCanvas* c1 = new TCanvas("c1", "c1", 1200, 600);
    c1->SetMargin(0, 0, 0, 0);
    c1->Divide(2, 1, 0, 0);
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
	    h2[name]->GetXaxis()->SetTitleSize(0.07);
	    h2[name]->GetXaxis()->SetTitleOffset(0.9);
	    h2[name]->GetXaxis()->SetLabelSize(0.05);
	    h2[name]->GetYaxis()->SetTitleSize(0.07);
	    h2[name]->GetYaxis()->SetTitleOffset(1.18);
	    h2[name]->GetYaxis()->SetLabelSize(0.05);
	    i++;
	}
	c1->cd(1);
	gPad->SetLeftMargin(0.18);
	gPad->SetRightMargin(0.11);
	gPad->SetBottomMargin(0.15);
	gPad->SetTopMargin(0.07);
	h2[name1]->Draw("colz");
	gPad->Update();
	// update title
	TPaveText* pt1 = (TPaveText*) gPad->GetPrimitive("title");
	pt1->SetTextSize(0.07);
	gPad->Modified();

	c1->cd(2);
	gPad->SetLeftMargin(0.18);
	gPad->SetRightMargin(0.11);
	gPad->SetBottomMargin(0.15);
	gPad->SetTopMargin(0.07);
	h2[name2]->Draw("colz");
	gPad->Update();
	TPaveText* pt2 = (TPaveText*) gPad->GetPrimitive("title");
	pt2->SetTextSize(0.07);
	gPad->Modified();

	if (prefix)
	    c1->SaveAs(Form("%s_%s.pdf", prefix, var));
	else
	    c1->SaveAs(Form("%s.pdf", var));
    }
}

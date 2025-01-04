#include "cali_style.h"

const double Y1[] = {0.5, 0.7};
const double height = 0.2;

void compare(const char* f1, const char* f2, const char* name1 = "data", const char* name2 = "sim", const char *prefix = NULL, const char* title = "T1")
{
    gROOT->SetBatch(1);
    cali_style();
    gStyle->SetOptStat(0);
    colors[0] = kRed;
    colors[1] = kBlue;

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
    c->SetTopMargin(0.01);
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

    // 1D histograms
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
	    // h[name]->SetTitleOffset(1.5);
	    h[name]->SetMarkerStyle(20);
	    h[name]->SetMarkerColor(colors[i]);
	    h[name]->SetLineColor(colors[i]);
	    h[name]->SetLineWidth(2);
	    h[name]->Scale(1/h[name]->Integral());
	    h[name]->GetXaxis()->SetTitleSize(0.07);
	    h[name]->GetXaxis()->SetTitleOffset(0.8);
	    h[name]->GetXaxis()->SetLabelSize(0.05);
	    h[name]->GetYaxis()->SetTitleSize(0.07);
	    h[name]->GetYaxis()->SetTitleOffset(1.0);
	    h[name]->GetYaxis()->SetLabelSize(0.05);
	    i++;
	}

	if (strcmp(var, "hit_mul") == 0 && h[name1]->GetMaximum() < 0.048)
	    h[name1]->SetMaximum(0.048);
	else if (strcmp(var, "clu_mul") == 0)
	    h[name1]->SetMaximum(h[name1]->GetMaximum()*1.16);

	// rebin pi0 mass histogram
	// if (strcmp(var, "pi0_mass") == 0)
	// {
	//     h[name1]->Rebin(2);
	//     h[name2]->Rebin(2);
	// }

	string name;
	if (h[name1]->GetMaximum() > h[name2]->GetMaximum())
	{
	    h[name1]->Draw("P");
	    h[name2]->Draw("HIST SAMES");
	}
	else
	{
	    h[name2]->Draw("HIST");
	    h[name1]->Draw("P SAMES");
	}
	c->Update();

	TPaveText* pt = (TPaveText*) c->GetPrimitive("title");
	// update title
	pt->SetTextSize(0.07);
	pt->SetY1NDC(0.88);
	c->Modified();

	l->AddEntry(h[name1], name1, "lep");
	l->AddEntry(h[name2], name2, "lp");
	l->SetTextSize(0.055);

	l->Draw();

	if (strcmp(var, "pi0_mass") == 0)
	{
	    TLine *l_pi0 = new TLine(135, gPad->GetUymin(), 135, gPad->GetUymax());
	    l_pi0->SetLineColor(kViolet);
	    l_pi0->SetLineWidth(2);
	    l_pi0->Draw();
	}
	if (prefix)
	    c->SaveAs(Form("%s_%s.pdf", prefix, var));
	else
	    c->SaveAs(Form("%s.pdf", var));
    }

    // TProfile plot
    c->SetLeftMargin(0.1);
    TLegend* lp = new TLegend(0.2, 0.75, 0.3, 0.85);

    map<string, TProfile *> hp;
    map<string, pair<const char*, const char*>> hp_title = {
	{"hit_mul_vs_event_MIP", {"Event Energy [MIP]", "Hit Multiplicity"}},
    };
    for(const char* var : {"hit_mul_vs_event_MIP"})
    {
	c->Clear();
	lp->Clear();
	size_t i = 0;
	for (const char* name : {name1, name2})
	{
	    TH2F* h2 = (TH2F*) fin[name]->Get(var);
	    hp[name] = h2->ProfileX();
	    hp[name]->SetName(name);
	    hp[name]->SetTitle(Form("%s;%s;%s", title, hp_title[var].first, hp_title[var].second));
	    hp[name]->SetMarkerStyle(20);
	    hp[name]->SetMarkerColor(colors[i]);
	    hp[name]->SetLineColor(colors[i]);
	    hp[name]->SetLineWidth(2);
	    // hp[name]->Scale(1/h[name]->Integral());
	    hp[name]->GetXaxis()->SetTitleSize(0.07);
	    hp[name]->GetXaxis()->SetTitleOffset(0.8);
	    hp[name]->GetXaxis()->SetLabelSize(0.05);
	    hp[name]->GetYaxis()->SetTitleSize(0.07);
	    hp[name]->GetYaxis()->SetTitleOffset(0.6);
	    hp[name]->GetYaxis()->SetLabelSize(0.05);
	    i++;
	}

	if (hp[name1]->GetMaximum() > hp[name2]->GetMaximum())
	{
	    hp[name1]->Draw("P");
	    hp[name2]->Draw("HIST SAMES");
	}
	else
	{
	    hp[name2]->Draw("HIST");
	    hp[name1]->Draw("P SAMES");
	}
	c->Update();

	TPaveText* pt = (TPaveText*) c->GetPrimitive("title");
	// update title
	pt->SetTextSize(0.07);
	pt->SetY1NDC(0.88);
	c->Modified();

	lp->AddEntry(hp[name1], name1, "lep");
	lp->AddEntry(hp[name2], name2, "lp");
	lp->SetTextSize(0.055);
	lp->Draw();

	if (prefix)
	    c->SaveAs(Form("%s_%s_profileX.pdf", prefix, var));
	else
	    c->SaveAs(Form("%s_profileX.pdf", var));
    }

    // 2D histograms
    map<string, TH2F *> h2;
    TCanvas* c1 = new TCanvas("c1", "c1", 1200, 600);
    c1->SetMargin(0, 0, 0, 0);
    c1->Divide(2, 1, 0, 0);
    map<string, const char*> XYtitle = {
	{"hit_mul_vs_event_MIP", "Event Energy [MIP];Hit Multiplicity"},
	{"eta_vs_event_MIP", "Event Energy [MIP];#eta"},
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
    for(const char* var : {"hit_mul_vs_event_MIP", "eta_vs_event_MIP", "clu_x_vs_y", "clu_x_vs_y_weighted", 
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
	    h2[name]->GetXaxis()->SetTitleOffset(0.85);
	    h2[name]->GetXaxis()->SetLabelSize(0.045);
	    h2[name]->GetYaxis()->SetTitleSize(0.07);
	    h2[name]->GetYaxis()->SetTitleOffset(0.8);
	    h2[name]->GetYaxis()->SetLabelSize(0.045);
	    i++;
	}

	const char* names[] = {name1, name2};
	for (int i=0; i<2; i++)
	{
	    c1->cd(i+1);
	    gPad->SetLeftMargin(0.13);
	    gPad->SetRightMargin(0.11);
	    gPad->SetBottomMargin(0.15);
	    gPad->SetTopMargin(0.07);
	    h2[names[i]]->Draw("colz");
	    gPad->Update();
	    // update title
	    TPaveText* pt = (TPaveText*) gPad->GetPrimitive("title");
	    pt->SetTextSize(0.07);
	    gPad->Modified();

	    if (strcmp(var, "hit_mul_vs_event_MIP") == 0)
	    {
		TProfile* hp = h2[names[i]]->ProfileX();
		hp->SetMarkerStyle(20);
		hp->SetMarkerSize(0.6);
		hp->SetMarkerColor(kRed);
		hp->Draw("SAME HIST P");
		gPad->Update();
	    }
	}


	if (prefix)
	    c1->SaveAs(Form("%s_%s.pdf", prefix, var));
	else
	    c1->SaveAs(Form("%s.pdf", var));
    }
}

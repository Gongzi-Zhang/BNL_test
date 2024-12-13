const int colors[] = {kBlack, kRed};
const double Y1[] = {0.5, 0.7};
const double height = 0.2;

void compare(const char* f1, const char* f2, const char* name1 = "data", const char* name2 = "sim", const char *prefix = NULL, const char* cut = "")
{
    gROOT->SetBatch(1);
    // gStyle->SetOptStat(111111);
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
    c->SetLogy(1);
    map<string, TH1F *> h;
    for (const char* var : {"hit_MIP", "event_MIP", "hit_mul", "hit_mul1", "hit_mul2", "hit_mul3", "hit_mul4", "event_x", "event_y", "event_z", "clu_mul", "clu_MIP", "clu_nhits"})
    {
	c->Clear();
	if (strcmp(var, "hit_MIP") == 0 || strcmp(var, "event_MIP") == 0)
	    c->SetLogy(1);
	else
	    c->SetLogy(0);
	int i = 0;
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

	i = 0;
	for (const char* name : {name1, name2})
	{
	    TPaveStats *st = (TPaveStats*) h[name]->FindObject("stats");
	    st->SetTextColor(colors[i]);
	    st->SetY1NDC(Y1[i]);
	    st->SetY2NDC(Y1[i] + height);
	    i++;
	}

	TText *tcut = new TText(0.5, 0.92, cut);
	tcut->SetNDC(true);
	tcut->SetTextSize(0.03);
	tcut->SetTextColor(kRed-2);
	tcut->SetTextAlign(22);
	tcut->Draw();

	if (prefix)
	    c->SaveAs(Form("%s_%s.png", prefix, var));
	else
	    c->SaveAs(Form("%s.png", var));
    }
}

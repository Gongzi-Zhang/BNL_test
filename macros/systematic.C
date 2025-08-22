// calculate average value with UP and DOWN modification on hit energy

void systematic(const char* in, const char* up, const char* low, const char* output)
{
    TFile *fin = new TFile(in, "read");
    if (!fin || !fin->IsOpen())
    {
	cerr << "Can't open root file: " << in << endl;
	exit(1);
    }
    TFile *fup = new TFile(up, "read");
    if (!fup || !fup->IsOpen())
    {
	cerr << "Can't open root file: " << up << endl;
	exit(1);
    }
    TFile *flow = new TFile(low, "read");
    if (!flow || !flow->IsOpen())
    {
	cerr << "Can't open root file: " << low << endl;
	exit(1);
    }

    TFile *fout = new TFile(output, "recreate");
    // 1D histograms
    for (const char* var : {"hit_MIP", "event_MIP", "hit_mul", "hit_mul1", "hit_mul2", "hit_mul3", "hit_mul4", "event_x", "event_y", "event_z", "clu_mul", "clu_MIP", "clu_x", "clu_y", "clu_z", "clu_nhits", "pi0_mass"})
    {
	TH1F* h1up  = (TH1F*) fup->Get(var);
	TH1F* h1low = (TH1F*) flow->Get(var);
	h1up->Scale(1/h1up->Integral());
	h1low->Scale(1/h1low->Integral());

	TH1F* temp = (TH1F*) fin->Get(var);
	fout->cd();
	TH1F* nominal = (TH1F*) temp->Clone();
	nominal->Scale(1/nominal->Integral());
	double upError, lowError, error;
	for (int i=1; i<= nominal->GetNbinsX(); i++)
	{
	    upError  = abs(h1up->GetBinContent(i) - nominal->GetBinContent(i));
	    lowError = abs(h1low->GetBinContent(i) - nominal->GetBinContent(i));
	    error = (upError + lowError)/2;
	    if (0 == error)
		error = 1e-9;
	    nominal->SetBinError(i, error);
	}
	// nominal->Write();
    }

    // 2D histograms
    for(const char* var : {"hit_mul_vs_event_MIP", "eta_vs_event_MIP", "clu_x_vs_y", "clu_x_vs_y_weighted", 
	    "clu_e_vs_x", "clu_e_vs_y", "clu_e_vs_z"}) 
    {
	TH2F* h2up  = (TH2F*) fup->Get(var);
	TProfile *hpup = h2up->ProfileX();
	TH2F* h2low = (TH2F*) flow->Get(var);
	TProfile *hplow = h2low->ProfileX();

	TH2F* temp = (TH2F*) fin->Get(var);
	fout->cd();
	TH2F* nominal = (TH2F*) temp->Clone();
	TProfile *hp = nominal->ProfileX();
	hp->SetName(Form("%s_profileX", var));
	double upError, lowError, error;
	for (int i=1; i<= nominal->GetNbinsX(); i++)
	{
	    for (int j=1; j<= nominal->GetNbinsY(); j++)
	    {
		upError  = abs(h2up->GetBinContent(i, j) - nominal->GetBinContent(i, j));
		lowError = abs(h2low->GetBinContent(i, j) - nominal->GetBinContent(i, j));
		error = (upError + lowError)/2;
		nominal->SetBinError(i, j, error);
	    }
	    upError = abs(hpup->GetBinContent(i) - hp->GetBinContent(i));
	    lowError = abs(hplow->GetBinContent(i) - hp->GetBinContent(i));
	    error = (upError + lowError) / 2;
	    hp->SetBinError(i, error);
	}
	// nominal->Write();
	// hp->Write();
    }

    fout->Write();
    fout->Close();
}

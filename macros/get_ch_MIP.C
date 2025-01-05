#include "utilities.h"
#include "cali.h"

void get_ch_MIP(const int run, const int ch, const char* gain = "HG")
{
    int caen = ch/64;

    TFile *fin = new TFile(cali::getFile(Form("Run%d.root", run)).c_str(), "read");
    TTree *tin = (TTree*) fin->Get(Form("raw_CAEN%d", caen));
    if (!tin)
    {
	cerr << ERROR << "No raw_CAEN tree in run " << run << endl;
	fin->Close();
	return;
    }
    tin->SetBranchStatus("*", 0);
    tin->SetBranchStatus(Form("ch_%d", ch), 1);
    TBranch *b = (TBranch*) tin->GetBranch(Form("ch_%d", ch));
    int adc;
    b->GetLeaf(gain)->SetAddress(&adc);

    TCanvas *c = new TCanvas("c", "c", 1600, 1200);
    c->ToggleEventStatus();
    c->SetLogy(1);

    map<string, double> xmax = { {"LG", 1000}, {"HG", 8400}};
    TH1F* h1 = new TH1F(Form("ch%d_%s", ch, gain), Form("%s ch %d", gain, ch), 1000, 0, xmax[gain]);
    double width = h1->GetBinWidth(1);

    const int N = tin->GetEntries();
    vector<double> runADCs(N);
    for (int i=0; i<N; i++)
    {
	tin->GetEntry(i);
	h1->Fill(adc);
	runADCs[i] = adc;
    }

    /*
    // pedestal peak: the fit range starts from 1 to voaid the zero peak; 
    TKDE *kde = new TKDE(N, &rawADC[gain][0], 1, 0.9*xmax, "", 1);   
    TF1 *f1 = kde->GetFunction();
    double GausMean = f1->GetMaximumX();
    double norm = h->GetBinContent(h->FindBin(GausMean));
    double norm1 = f1->Eval(GausMean);
    double x = GausMean + width;
    while (f1->Eval(x) > 0.6*norm1)	// 1 sigma
	x += width;
    double GausSigma = x - GausMean;

    // a new kde to avoid effect from the ped peak, which, if exist, 
    // may make the MIP peak negligible; so the starting point should
    // be as far away from the ped peak as possible
    while (f1->Eval(x) > 0.05*norm1)
	x += width;
    double x1 = x;

    TF1 *fitPed = new TF1(Form("ch%d_%s_ped", ch, gain), "[0]*TMath::Gaus(x, [1], [2])", 0, x);
    fitPed->SetParameters(norm, GausMean, GausSigma);
    h->Fit(fitPed, "q", 0, x);
    pedPeak[ch][gain] = fitPed->GetParameter(1);
    g1[ch][gain]->SetPoint(0, pedPeak[ch][gain], fitPed->GetParameter(0));
    delete fitPed;

    delete f1;
    delete kde;
    kde = new TKDE(N, &rawADC[gain][0], x, 0.9*xmax, "", 1);
    f1 = kde->GetFunction();

    // local minimum between the ped peak and the MIP peak
    double pre_x = x;
    double pre_y = f1->Eval(pre_x);
    double y;

    // Sometimes, there is a raising in the kde, I don't know why 
    x = pre_x + width;
    y = f1->Eval(x);
    while (y > pre_y)
    {
	pre_x = x;
	pre_y = y;
	x += width;
	y = f1->Eval(x);
    }
    double x2 = x;	// in case of no raising, this is the MIP peak

    x = pre_x + 8*width;	// need 10 bins here to jump over possible fluctuations
    y = f1->GetMinimum(pre_x, x);
    while (y < pre_y && x < xmax)
    {
	pre_y = y;
	pre_x = x;
	x = pre_x + 8*width;
	y = f1->GetMinimum(pre_x, x);
    }

    if (h->GetBinContent(h->FindBin(pre_x)) < norm/2e5)
    {
	mipPeak[ch][gain] = 0;
	MIP[ch][gain] = 0;
	continue;
    }

    if (x >= xmax)
    {
	mipPeak[ch][gain] = x2;
    }
    else
    {
	x = pre_x;

	delete f1;
	delete kde;
	kde = new TKDE(N, &rawADC[gain][0], x, 0.9*xmax, "", 1);
	f1 = kde->GetFunction();
	mipPeak[ch][gain] = f1->GetMaximumX();
    }
    MIP[ch][gain] = mipPeak[ch][gain] - pedPeak[ch][gain];
     */

    // h1->Scale(1/h1->Integral());
    h1->Draw();
    // kde->Draw("SAME");
}

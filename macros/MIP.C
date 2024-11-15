#include "utilities.h"
#include "cali.h"

void MIP(const int run, const int ch, const char* gain = "HG")
{
    int caen = ch/64;

    TFile *fin = new TFile(cali::getRootFile(run).c_str(), "read");
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

    TCanvas *c = new TCanvas("c", "c", 800, 600);
    c->ToggleEventStatus();
    c->SetLogy(1);

    map<string, double> xmax = { {"LG", 1000}, {"HG", 8400}};
    double width = xmax[gain]/1000;
    TH1F* h1 = new TH1F(Form("ch%d_%s", ch, gain), Form("%s ch %d", gain, ch), 1000, 0, xmax[gain]);

    const int N = tin->GetEntries();
    vector<double> runADCs(N);
    for (int i=0; i<N; i++)
    {
	tin->GetEntry(i);
	h1->Fill(adc);
	runADCs[i] = adc;
    }

    TKDE *kde = new TKDE(N, &runADCs[0], 1, 0.9*xmax[gain], "", 1);
    TF1 *f1 = kde->GetFunction();
    double GausMean = f1->GetMaximumX();
    double norm = h1->GetBinContent(h1->FindBin(GausMean));
    double norm1 = f1->Eval(GausMean);
    double x = GausMean + width;
    while (f1->Eval(x) > 0.6*norm1)
	x += width;
    double GausSigma = x - GausMean;

    while (f1->Eval(x) > 0.05*norm1)
	x += width;
    TF1 *fitPed = new TF1("ped", "[0]*TMath::Gaus(x, [1], [2])", 1, x);
    fitPed->SetParameters(norm, GausMean, GausSigma);
    h1->Fit(fitPed, "", 0, x);
    cout << "ped: " << fitPed->GetParameter(1) << " +- " << fitPed->GetParameter(2) << endl;

    // delete f1;
    // delete kde;
    // kde = new TKDE(N, &runADCs[0], x, 0.9*xmax[gain], "", 1);
    // f1 = kde->GetFunction();
    // double pre_x = x;
    // double pre_y = f1->Eval(pre_x);
    // double y;
    // x = pre_x + width;
    // double y = f1->Eval(x);
    // while(y > pre_y)
    // {
    //     pre_x = x;
    //     pre_y = y;
    //     x += width;
    //     y = f1->Eval(x);
    // }
    // x = pre_x + 8*width;
    // y = f1->GetMinimum(pre_x, x);
    // while(y < pre_y)
    // {
    //     pre_y = y;
    //     pre_x = x;
    //     x = pre_x + 8*width;
    //     y = f1->GetMinimum(pre_x, x);
    // }
    // pre_x = x;
    // pre_y = y;
    // x = pre_x + width;
    // y = f1->Eval(x);
    // while (y > pre_y)
    // {
    //     pre_y = y;
    //     pre_x = x;
    //     x = pre_x + width;
    //     y = f1->Eval(x);
    // }

    // h1->Scale(1/h1->Integral());
    // h1->Draw();
    // kde->Draw("SAME");
}

/* plot HG ADC distribution for a few channels for checking MIPs */
#include "utilities.h"
#include "cali.h"
#include "db.h"

void plot_ch_MIP(const int run1, const int run2)
{
    gROOT->SetBatch(1);

    map<int, string> date;
    map<int, TFile*> fin;
    caliDB db;
    for (auto run : {run1, run2})
    {
	if (db.getRunType(run) != "mip")
	{
	    cerr << ERROR << "Not a mip run: " << run << endl;
	    return;
	}
	fin[run] = new TFile(cali::getFile(Form("Run%d.root", run)).c_str(), "read");
	if (!fin[run] || !fin[run]->IsOpen())
	{
	    cerr << ERROR << "Can't open root file for run: " << run << endl;
	    return;
	}
	date[run] = db.getRunStartTime(run).substr(0, 10);
    }

    const int channels[] = {25, 62, 100, 160};
    const char* labels[] = {"Hex tile, 3mm SiPM", "Hex tile, 1.3mm SiPM", "Square tile, 3mm SiPM", "Hex tile, 3mm SiPM, Unpainted"};
    const int nChannels = sizeof(channels)/sizeof(channels[0]);

    TCanvas *c = new TCanvas("c", "c", 600*4, 600*2);
    c->Divide(4, 2, 0, 0);
    for (int i=0; i<2; i++)
    {
	int run = run1;
	if (i == 1) 
	    run = run2;
	for (int ich=0; ich<nChannels; ich++)
	{
	    int ch = channels[ich];
	    TTree *tin = (TTree*) fin[run]->Get(Form("raw_CAEN%d", ch/64));
	    TH1F* h1 = new TH1F(Form("ch%d_%d", ch, i), Form("ch %d", ch), 1000, 0, 8400);
	    h1->SetStats(false);

	    c->cd(4*i+ich+1);
	    gPad->SetMargin(0, 0, 0, 0);
	    gPad->SetLogy(1);
	    tin->Draw(Form("ch_%d.HG >> ch%d_%d", ch, ch, i));
	    if (ich == 0)
		gPad->SetLeftMargin(0.1);
	    else
		h1->GetYaxis()->SetLabelSize(0); // Hide y-axis labels except leftmost pads
	    if (i == 1)
	    {
		gPad->SetBottomMargin(0.1);
		h1->SetXTitle("ADC");
	    }
	    else
	    {
		TLatex *latex = new TLatex();
		latex->SetTextSize(0.05);
		latex->SetTextAlign(22);
		latex->SetTextColor(kRed);
		latex->DrawLatexNDC(0.6, 0.8, labels[ich]);
		h1->GetXaxis()->SetLabelSize(0); // Hide x-axis labels except bottom row
	    }
	}
    }
    // title
    c->cd();
    TLatex *latex = new TLatex();
    latex->SetTextSize(0.04);
    latex->SetTextAlign(21);  // Centered alignment

    latex->DrawLatexNDC(0.5, 0.96, Form("Run %d (%s) vs %d (%s)", run1, date[run1].c_str(), run2, date[run2].c_str()));
    c->SaveAs(Form("run%d_vs_%d_HG_MIP.png", run1, run2));

    /*
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
     */
}

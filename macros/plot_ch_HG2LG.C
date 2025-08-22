#include "utilities.h"
#include "cali.h"
#include "db.h"
#include "analysis.h"
#include "cali_style.h"

void plot_ch_HG2LG(const int run=2019)
{
    gROOT->SetBatch(1);
    cali_style();

    // const int channels[] = {25, 62, 100, 160};
    const int channels[] = {160};
    const int nChannels = sizeof(channels)/sizeof(channels[0]);

    caliDB db;
    int pedRun = db.getPedRun(run);
    ped_t ped;
    getPedestal(pedRun, ped);

    string rootFile = cali::getFile(Form("Run%d.root", run));
    TFile *fin = new TFile(rootFile.c_str(), "read");
    TTree *traw = (TTree*) fin->Get("raw");
    map<int, map<string, int>> rawADC;
    map<int, map<string, int>> corADC;
    map<int, TH2F*> h2;
    for (int ch : channels)
    {
	TBranch* b = (TBranch*) traw->GetBranch(Form("ch_%d", ch));
	for (const char*gain : {"HG", "LG"})
        {
            rawADC[ch][gain] = 0;
            b->GetLeaf(gain)->SetAddress(&rawADC[ch][gain]);
        }
	h2[ch] = new TH2F(Form("ch_%d", ch), Form("Ch %d;LG [ADC];HG [ADC]", ch), 100, 0, 300, 100, 0, 8000);
    }

    // filling histograms
    for (int ei=0; ei<traw->GetEntries(); ei++)
    {
        traw->GetEntry(ei);
	for (int ch : channels)
	{
	    for (const char *gain : {"HG", "LG"})
            {
                corADC[ch][gain] = rawADC[ch][gain] - ped[ch][gain].mean;
                if (corADC[ch][gain] < 5*ped[ch][gain].rms)
                    corADC[ch][gain] = 0;
            }
            if (corADC[ch]["LG"] > 0 && corADC[ch]["HG"] < 7700)
                h2[ch]->Fill(corADC[ch]["LG"], corADC[ch]["HG"]);
	}
    }

    // plot and fit
    TCanvas* c = new TCanvas("c", "c", 800*nChannels, 600);
    c->SetMargin(0, 0, 0, 0);
    // c->Divide(4, 1, 0, 0);
    for (size_t i=0; i<nChannels; i++)
    {
	c->cd(i+1);
	gPad->SetLeftMargin(0.18);
	gPad->SetRightMargin(0.11);
	gPad->SetBottomMargin(0.15);
	gPad->SetTopMargin(0.05);

	int ch = channels[i];
	TProfile * proX = h2[ch]->ProfileX(Form("ch%d_profileX", ch));
        TF1 *fit = new TF1(Form("ch%d_fit", ch), "[0] + [1]*x", 0, 300);
        fit->SetParameters(0, 30);
        proX->Fit(fit, "q");

	h2[ch]->SetStats(false);
	h2[ch]->SetTitle(";LG [ADC];HG [ADC]");
	h2[ch]->GetXaxis()->SetTitleSize(0.07);
	h2[ch]->GetXaxis()->SetTitleOffset(0.9);
	h2[ch]->GetXaxis()->SetLabelSize(0.05);
	h2[ch]->GetYaxis()->SetTitleSize(0.07);
	h2[ch]->GetYaxis()->SetTitleOffset(1.18);
	h2[ch]->GetYaxis()->SetLabelSize(0.05);
        h2[ch]->Draw("COLZ");
	fit->Draw("SAME");

	TLatex *latex = new TLatex();
	latex->SetTextSize(0.06);
	latex->SetTextAlign(22);
	latex->SetTextColor(kRed);
	latex->DrawLatexNDC(0.65, 0.38, Form("Ch %d", ch));
	latex->DrawLatexNDC(0.65, 0.3, Form("Slope = %.2f #pm %.2f", fit->GetParameter(1), fit->GetParError(1)));
        latex->SetTextColor(kRed);
        latex->Draw();
    }
    c->Update();
    c->SaveAs(Form("%s/figures/%d/HG2LG.pdf", cali::CALIROOT, run));
}

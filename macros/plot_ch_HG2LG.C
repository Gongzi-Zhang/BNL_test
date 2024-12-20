#include "utilities.h"
#include "cali.h"
#include "db.h"
#include "analysis.h"

void plot_ch_HG2LG(const int run)
{
    const int channels[] = {25, 62, 100, 160};
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
    TCanvas* c = new TCanvas("c", "c", 600*nChannels, 600);
    c->Divide(4, 1);
    for (size_t i=0; i<nChannels; i++)
    {
	c->cd(i+1);
	gPad->SetLeftMargin(0.15);

	int ch = channels[i];
	TProfile * proX = h2[ch]->ProfileX(Form("ch%d_profileX", ch));
        TF1 *fit = new TF1(Form("ch%d_fit", ch), "[0] + [1]*x", 0, 300);
        fit->SetParameters(0, 30);
        proX->Fit(fit, "q");

	h2[ch]->SetStats(false);
        h2[ch]->Draw("COLZ");
	fit->Draw("SAME");

	TText *t = new TText(0.5, 0.2, Form("slope = %.2f", fit->GetParameter(1)));
        t->SetNDC();
        t->SetTextColor(kRed);
        t->Draw();
    }
    c->Update();
    c->SaveAs(Form("Run%d_HG2LG.pdf", run));
}

#include "db.h"
#include "calo.h"
#include "cali.h"
#include "analysis.h"

#include "cali_style.h"

const int nChannelMax = 4*64;

void plot_run25_mip() 
{
    gROOT->SetBatch(1);
    cali_style();

    caliDB db;
    vector<int> runs = db.getRuns("Run >= 3089 AND Run <= 4426 AND Type = 'mip' AND Flag = 'good' AND Events > 100000");

    map<int, TGraph*> mipMean;
    map<int, int> ipoint;
    map<int, double> mipSum;

    for (int ch=0; ch<nChannelMax; ch++)
    {
	mipMean[ch] = new TGraph();
	mipMean[ch]->SetMarkerStyle(20);
	mipMean[ch]->SetMarkerSize(2);
	mipSum[ch] = 0;
	ipoint[ch] = 0;
    }

    // read mip
    for (int run : runs)
    {
	string mipFile = cali::getFile(Form("Run%d_MIP.json", run));
	if (mipFile.empty())
	{
	    cout << WARNING << "Can't find mip file for run " << run << endl;
	    continue;
	}
	mip_t mip;
	if (run < 3974)
	    calo::setnCAENChannels({64, 64, 64});
	else
	    calo::setnCAENChannels({64, 64, 64, 64});

	getMIP(mipFile.c_str(), mip);

	for (int ch=0; ch<calo::nChannels; ch++)
	{
	    double mean = mip[ch]["HG"];

	    if (mean == 0)
		continue;

	    mipMean[ch]->SetPoint(ipoint[ch], run, mean);
	    mipSum[ch] += mean;
	    ipoint[ch]++;
	}
    }


    // plot
    // 4 pads
    TCanvas* c = new TCanvas("c", "c", 3000, 1600);

    // ---- Layout parameters ----
    float top_h    = 1.00;
    float bottom_h = 0.00;

    // ---- Create pads ----
    TPad* p1 = new TPad("p1", "top",
			0.00, 1 - top_h,
			1, 1);
    TPad* p2 = new TPad("p2", "bottom",
			0, 0,
			1.00, bottom_h);

    // Draw pads
    p1->Draw(); // p2->Draw();

    // ---- Margins ----
    // Common margins
    for (TPad* p : {p1,p2}) {
	p->SetLeftMargin(0.1);
	p->SetRightMargin(0.03);
	p->SetTopMargin(0.08);
    }
    // top pads → hide bottom space (no x-axis)
    // p1->SetBottomMargin(0);
    // bottom pads → hide top space (no x-axis)
    // p2->SetTopMargin(0);

    // bottom pads → show x-axis
    p1->SetBottomMargin(0.1);

    // ---- Begin PDF ----
    c->Print("run25_mip.pdf[");

    for (int ch=0; ch<nChannelMax; ch++) {
	if (ipoint[ch] > 0)
	    cout << INFO << "Channel " << ch << ", mip: " << mipSum[ch]/ipoint[ch] << endl;

	p1->cd();
	p1->Clear();
	mipMean[ch]->Draw("AP");
	mipMean[ch]->SetTitle(Form("Ch %d HG", ch));
	// mipMean[ch]->GetXaxis()->SetLabelSize(0);
	mipMean[ch]->GetXaxis()->SetTitle("Run");
	mipMean[ch]->GetYaxis()->SetTitle("Mean [ADC]");
	// p2->cd();
	// p2->Clear();
	// mipError[ch]["LG"]->Draw("AP");
	// mipError[ch]["LG"]->GetXaxis()->SetTitle("Run");
	// mipError[ch]["LG"]->GetYaxis()->SetTitle("RMS [ADC]");

	c->cd();
	c->Modified();
	c->Update();
	c->Print("run25_mip.pdf");
    }

    c->Print("run25_mip.pdf]");
}

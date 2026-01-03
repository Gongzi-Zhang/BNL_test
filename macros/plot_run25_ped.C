/* plot channel-by-channel MIP values and their ratio to pedestal */
#include <nlohmann/json.hpp> 
#include "db.h"
#include "cali.h"
#include "analysis.h"

#include "cali_style.h"

const int nChannels = 4*64;

void plot_run25_ped() 
{
    gROOT->SetBatch(1);
    cali_style();

    caliDB db;
    vector<int> runs = db.getRuns("Run >= 3089 AND Run <= 4426 AND Type = 'ptrg' AND Flag = 'good'");

    map<int, map<string, TGraph*>> pedMean;
    map<int, map<string, TGraph*>> pedError;
    map<int, map<string, int>> ipoint;

    for (int ch=0; ch<nChannels; ch++)
    {
	for (const char* gain : {"HG", "LG"})
	{
	    pedMean[ch][gain] = new TGraph();
	    pedMean[ch][gain]->SetMarkerStyle(20);
	    pedError[ch][gain] = new TGraph();
	    pedError[ch][gain]->SetMarkerStyle(20);
	    ipoint[ch][gain] = 0;
	}
    }

    // read pedestal
    for (int run : runs)
    {
	string pedFile = cali::getFile(Form("Run%d_ped.json", run));
	ped_t ped;
	getPedestal(pedFile.c_str(), ped);

	for (int ch=0; ch<3*64; ch++)
	{
	    for (const char* gain : {"HG", "LG"})
	    {
		double mean = ped[ch][gain].mean;
		double rms = ped[ch][gain].rms;

		pedMean[ch][gain]->SetPoint(ipoint[ch][gain], run, mean);
		pedError[ch][gain]->SetPoint(ipoint[ch][gain], run, rms);
		ipoint[ch][gain]++;
	    }
	}
	
	/*
	if (run >= 3974)
	{
	    for (int ch=3*64; ch<4*64; ch++)
	    {
		for (const char* gain : {"HG", "LG"})
		{
		    double mean = ped[ch][gain].mean;
		    double rms = ped[ch][gain].rms;

		    pedMean[ch][gain]->SetPoint(ipoint[ch][gain], run, mean);
		    pedError[ch][gain]->SetPoint(ipoint[ch][gain], run, rms);
		    ipoint[ch][gain]++;
		}
	    }
	}
	*/


	// plot
	// 4 pads
	TCanvas* c = new TCanvas("c", "c", 2000, 1600);

	// ---- Layout parameters ----
	float col_gap = 0.03;
	float top_h    = 0.60;
	float bottom_h = 0.36;

	float left_col_w  = 0.5 - col_gap/2;
	float right_col_w = 0.5 - col_gap/2;

	// ---- Create pads ----
	TPad* p1 = new TPad("p1", "top-left",
			    0.00, 1 - top_h,
			    left_col_w, 1);
	TPad* p2 = new TPad("p2", "top-right",
			    left_col_w + col_gap, 1 - top_h,
			    1.00, 1);

	TPad* p3 = new TPad("p3", "bottom-left",
			    0.00, 0,
			    left_col_w, bottom_h);
	TPad* p4 = new TPad("p4", "bottom-right",
			    left_col_w + col_gap, 0,
			    1.00, bottom_h);

	// Draw pads
	p1->Draw(); p2->Draw(); p3->Draw(); p4->Draw();

	// ---- Margins ----
	// top pads → hide bottom space (no x-axis)
	p1->SetBottomMargin(0.02);
	p2->SetBottomMargin(0.02);

	// bottom pads → show x-axis
	p3->SetBottomMargin(0.25);
	p4->SetBottomMargin(0.25);

	// Common margins
	for (TPad* p : {p1,p2,p3,p4}) {
	    p->SetLeftMargin(0.14);
	    p->SetRightMargin(0.05);
	    p->SetTopMargin(0.08);
	}

	// ---- Begin PDF ----
	c->Print("run25_ped.pdf[");

	for (int ch=0; ch<5; ch) {
	    p1->cd();
	    pedMean[ch]["LG"]->Draw("AL");
	    pedMean[ch]["LG"]->GetXaxis()->SetLabelSize(0);
	    pedMean[ch]["LG"]->GetXaxis()->SetTickLength(0);
	    pedMean[ch]["LG"]->GetXaxis()->SetTitleSize(0);
	    p2->cd();
	    pedError[ch]["LG"]->Draw();

	    p3->cd();
	    pedMean[ch]["HG"]->Draw();
	    pedMean[ch]["HG"]->GetXaxis()->SetLabelSize(0);
	    pedMean[ch]["HG"]->GetXaxis()->SetTickLength(0);
	    pedMean[ch]["HG"]->GetXaxis()->SetTitleSize(0);
	    p4->cd();
	    pedError[ch]["HG"]->Draw();

	    c->cd();
	    c->Modified();
	    c->Update();
	    c->Print("run25_ped.pdf")
	}

	c->Print("run25_ped.pdf]");
    }
}

#include "utilities.h"
#include "cali.h"
#include "db.h"
#include "analysis.h"

#include "cali_style.h"

/* compare HG ADC distribution for a few channels for checking MIP calibration */
void compare_ch_MIP(const int run1 = 1032, const int run2 = 2580)
{
    colors[0] = kRed;
    colors[1] = kBlue;
    const int channels[] = {25, 62, 100, 160};
    const char* labels[] = {"#splitline{Ch 25}{#splitline{Hex tile}{3mm SiPM}}", 
			    "#splitline{Ch 62}{#splitline{Hex tile}{1.3mm SiPM}}", 
			    "#splitline{Ch 100}{#splitline{Square tile}{3mm SiPM}}", 
			    "#splitline{Ch 160}{#splitline{Hex tile, Unpainted}{3mm SiPM}}", 
			    };
    const int nChannels = sizeof(channels)/sizeof(channels[0]);

    map<int, string> date;
    map<int, TFile*> fin;
    map<int, map<int, TH1F*>> h1;

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

	for (int ich=0; ich<nChannels; ich++)
	{
	    int ch = channels[ich];
	    TTree *tin = (TTree*) fin[run]->Get(Form("raw_CAEN%d", ch/64));
	    h1[run][ch] = new TH1F(Form("ch%d_%d", ch, run), ";HG ADC;Count", 1000, 0, 8400);
	    tin->Draw(Form("ch_%d.HG >> ch%d_%d", ch, ch, run));
	}
    }

    TCanvas *c = new TCanvas("c", "c", 400*2, 300*2);
    c->SetMargin(0, 0, 0, 0);
    c->Divide(2, 2, 0, 0);

    for (int ich=0; ich<nChannels; ich++)
    {
	c->cd(ich+1);
	gPad->SetLogy(1);
	if (ich == 0 || ich == 2)
	{
	    gPad->SetLeftMargin(0.13);
	}
	else
	{
	    gPad->SetRightMargin(0.13);
	}

	if (ich == 0 || ich == 1)
	{
	    gPad->SetTopMargin(0.13);
	}
	else
	{
	    gPad->SetBottomMargin(0.13);
	}

	size_t i = 0;
	for (auto run : {run1, run2})
	{
	    TH1F* h = h1[run][channels[ich]];
	    h->SetStats(false);
	    h->SetMarkerColor(colors[i]);
	    h->SetMarkerStyle(20);
	    h->SetMarkerSize(0.5);

	    if (ich == 0 || ich == 1)
	    {
		h->GetXaxis()->SetLabelSize(0); // Hide x-axis labels for top 2 pads
	    }
	    else
	    {
	        h->GetXaxis()->SetTitleOffset(0.7);
	        h->GetXaxis()->SetTitleSize(0.08);
	        h->GetXaxis()->SetLabelSize(0.06);
	        h->GetXaxis()->SetNdivisions(505);
	    }

	    if (ich == 1 || ich == 3)
	    {
		h->GetYaxis()->SetLabelSize(0); // Hide y-axis labels for right 2 pads
	    }
	    else
	    {
	        h->GetYaxis()->SetTitleOffset(0.7);
	        h->GetYaxis()->SetTitleSize(0.08);
	        h->GetYaxis()->SetLabelSize(0.06);
	    }

	    
	    if (0 == i)
	    {
		h->Draw("P");
		TLatex *latex = new TLatex();
		latex->SetTextSize(0.07);
		latex->SetTextAlign(22);
		latex->SetTextColor(kBlack);
		if (0 == ich)
		    latex->DrawLatexNDC(0.7, 0.7, labels[ich]);
		else if (1 == ich)
		    latex->DrawLatexNDC(0.6, 0.7, labels[ich]);
		else if (2 == ich)
		    latex->DrawLatexNDC(0.7, 0.8, labels[ich]);
		else if (3 == ich)
		    latex->DrawLatexNDC(0.6, 0.8, labels[ich]);
	    }
	    else
		h->Draw("P SAME");

	    i++;
	}
    }
    // title
    c->cd();
    TLatex *latex = new TLatex();
    latex->SetTextSize(0.05);
    latex->SetTextAlign(21);  // Centered alignment

    latex->DrawLatexNDC(0.5, 0.95, Form("Run #color[632]{%d (%s)} vs #color[600]{%d (%s)}", run1, date[run1].c_str(), run2, date[run2].c_str()));
    c->SaveAs(Form("run%d_vs_%d_HG_MIP.pdf", run1, run2));

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

/* plot MIP over whole Run for some channels */
void plot_ch_MIP_over_time() 
{
    size_t channels[] = {25, 62, 100, 160};
    const int nChannels = sizeof(channels)/sizeof(channels[0]);
    map<size_t, TGraph*> g;
    for (size_t i=0; i<nChannels; i++)
    {
	size_t ch = channels[i];
	g[ch] = new TGraph();
	g[ch]->SetTitle(";Date;MIP [ADC]");
	g[ch]->SetMinimum(0);
	g[ch]->SetMaximum(4200);
	g[ch]->GetXaxis()->SetTimeDisplay(1);
	g[ch]->GetXaxis()->SetTimeFormat("%m/%d");
	g[ch]->GetXaxis()->SetTimeOffset(0, "gmt");
	g[ch]->SetLineColor(colors[i]);
	g[ch]->SetFillColor(colors[i]);
	g[ch]->SetMarkerColor(colors[i]);
	g[ch]->SetMarkerStyle(markers[i]);
    }

    caliDB db;
    vector<int> runs = db.getRuns("Type = 'mip' AND Flag = 'good'");
    size_t ipoint = 0;
    int year, month, day, hour, minute;
    map<size_t, double> sum;
    map<size_t, double> mean;
    map<size_t, size_t> count;

    for (const auto run : runs)
    {
	// if (1249 <= run && run <= 1270)
	if (1297 == run || 1333 == run || 1702 == run || 1703 == run || 1704 == run)
	    continue;

	mip_t mip;
	if (!getMIP(run, mip))
	{
	    cerr << FATAL << "unable to read mip for run " << run << endl;
	    continue;
	}
        string datetime = db.getRunStartTime(run);
	sscanf(datetime.c_str(), "%4d-%2d-%2d %2d:%2d", &year, &month, &day, &hour, &minute);
        TDatime dt(year, month, day, 0, 0, 0);

	for (const auto ch : channels)
	{
	    if (mip[ch]["HG"] < 10)
		mip[ch]["HG"] = -999;

	    if (mip[ch]["HG"] > 0)
	    {
		sum[ch] += mip[ch]["HG"];
		count[ch] += 1;
	    }
	    g[ch]->SetPoint(ipoint, dt.Convert(), mip[ch]["HG"]); // the first data point is set to the day before the first good run
	}
	ipoint++;
    }

    for (const auto ch : channels)
    {
	mean[ch] = sum[ch] / count[ch];
    }

    // unceratinty
    map<size_t, double> error;
    for (const auto run : runs)
    {
	// if (1249 <= run && run <= 1270)
	if (1297 == run || 1333 == run || 1702 == run || 1703 == run || 1704 == run)
	    continue;

	mip_t mip;
	if (!getMIP(run, mip))
	{
	    cerr << FATAL << "unable to read mip for run " << run << endl;
	    continue;
	}

	for (const auto ch : channels)
	{
	    if (mip[ch]["HG"] < 10)
		continue;

	    double e = abs(mip[ch]["HG"] - mean[ch]) / mean[ch];
	    if (e > error[ch])
		error[ch] = e;
	}
    }

    for (const auto ch : channels)
    {
	cout << INFO << ch << "\t" << error[ch] << endl;
    }

    TCanvas *c = new TCanvas("c", "c", 800, 600);
    TLegend *l = new TLegend(0.13, 0.7, 0.26, 0.88);
    l->SetTextSize(0.045);

    for (size_t i=0; i<nChannels; i++)
    {
	size_t ch = channels[i];
	if (0 == i)
	    g[ch]->Draw("AP");
	else
	    g[ch]->Draw("P SAME");

	l->AddEntry(g[ch], Form("ch %zu", ch), "p");
    }
    l->Draw();


    c->SaveAs("MIP_over_time.pdf");
}

void plot_ch_MIP()
{
    gROOT->SetBatch(1);
    cali_style();
    plot_ch_MIP_over_time();
    compare_ch_MIP();
}



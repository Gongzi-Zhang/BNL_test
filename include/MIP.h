#ifndef __MIP__
#define __MIP__

/* MIP finder */

#include <fstream>
#include <string>
#include <map>
#include <nlohmann/json.hpp>
#include "TStyle.h"
#include "TError.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TLeaf.h"
#include "TH1F.h"
#include "TGraph.h"
#include "TPaveStats.h"
#include "TLatex.h"
#include "TList.h"
#include "TF1.h"
#include "TKDE.h"
#include "TVirtualPad.h"
#include "TCanvas.h"
#include "calo.h"
#include "utilities.h"

using namespace std;

#include "Math/ProbFuncMathCore.h"
double LanGaus(double *x, double *par)
{
    /* parameters
     * par[0]:	Normalization factor
     * par[1]:	Landau Most Probable Position
     * par[2]:	Landau scale width
     * par[3]:	Gauss Mean
     * par[4]:	Gauss Sigma
     */
    // double invsq2pi = 0.3989422804014;	// 1/sqrt(2π)
    
    // control constants
    double np = 100;	    // # of convolution steps
    double sc = 5;	    // convolution extends to ±sc Gaussian sigmas
    
    // variables
    double xx;
    double mpc;
    double fland;
    double sum = 0;
    double xlow, xup;
    double step;

    // range of convolution integral
    xlow = par[3] - sc*par[4];
    xup  = par[3] + sc*par[4];
    step = (xup - xlow)/np;

    // convolution integral of Landau and Gaussian by sum
    for (int i=0; i<=np; i++)
    {
	xx = xlow + (i-0.5)*step;
	fland = TMath::Landau(x[0] - xx, par[1], par[2]);
	sum += fland * ROOT::Math::normal_cdf(xx, par[4], par[3]);
    }

    return par[0] * sum * step;
}

class MIPfinder {
  public:
    MIPfinder() {}
   ~MIPfinder() {}
    void setRootFile(const char* f) { rootFile = f; }
    void setOutDir(const char* d) { fdir = d; }
    void setOutFile(const char* f) { mipOut = f; }
    void readADC();
    vector<pair<double, double>> getExtremum(const int ch, const string gain);
    void init();
    void findMIP();
    void getMIP(const int ch);
    void plot();
    void write();

  private:
    string rootFile;
    string fdir;
    string mipOut;
    map<int, map<string, TH1F*>> h1;
    map<int, map<string, TGraph*>> g1;
    map<int, map<string, TF1*>> f1;
    // map<int, map<string, TF1*>> fitNoise;
    // map<int, map<string, TF1*>> fitSignal;
    map<int, map<string, float>> pedPeak;
    map<int, map<string, float>> mipPeak;
    map<int, map<string, float>> MIP;
};

void MIPfinder::init()
{
    map<string, float> xmax = {
	{"LG", 1000},
	{"HG", 8400},
    };
    for (int ch=0; ch<calo::nChannels; ch++)
	for (const char* gain : calo::gains)
	{
	    h1[ch][gain] = new TH1F(Form("ch%d_%s", ch, gain), Form("ch %d", ch), 1000, 0, xmax[gain]);
	    g1[ch][gain] = new TGraph();
	}
}

void MIPfinder::readADC()
{
    TFile *fin = new TFile(rootFile.c_str(), "read");
    if (!fin->IsOpen())
    {
	cerr << FATAL << "Can't open root file: " << rootFile << endl;
	fin->Close();
	return;
    }

    int LG, HG;
    for (int ci=0; ci<calo::nCAENs; ci++)
    {
	cout << INFO << "reading CAEN " << ci << endl;
	TTree *tin = (TTree *) fin->Get(Form("raw_CAEN%d", ci));
	const int N = tin->GetEntries();
	map<string, vector<double>> rawADC;
	for (const char* gain : calo::gains)
	    rawADC[gain].reserve(N);

	for (int ch=calo::preChannels[ci]; ch<calo::preChannels[ci] + calo::nCAENChannels[ci]; ch++)
	{
	    tin->SetBranchStatus("*", 0);
	    tin->SetBranchStatus(Form("ch_%d", ch), 1);
	    TBranch *b = (TBranch*) tin->GetBranch(Form("ch_%d", ch));
	    b->GetLeaf("LG")->SetAddress(&LG);
	    b->GetLeaf("HG")->SetAddress(&HG);

	    for (int ei=0; ei<tin->GetEntries(); ei++)
	    {
		tin->GetEntry(ei);
		h1[ch]["LG"]->Fill(LG);
		h1[ch]["HG"]->Fill(HG);
		rawADC["LG"][ei] = LG;
		rawADC["HG"][ei] = HG;
	    }

	    // find MIP
	    // This MIP finding algorithm is very unstable, it depends on various 
	    // things, like number of bins, gain, statistics, etc.
	    // Many parameters are used, which are tuning result
	    // Don't depend on it. A manual check is required to correct some MIP values.
	    // FIXME: how to extract a smooth distribution function from a histogram ???
	    for (const char* gain : calo::gains)
	    {
		TH1F *h = h1[ch][gain];
		double xmax = h->GetXaxis()->GetXmax();
		double width = h->GetBinWidth(1);

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
		g1[ch][gain]->SetPoint(1, mipPeak[ch][gain], h->GetBinContent(h->FindBin(mipPeak[ch][gain])));
	    }
	}
	delete tin;
    }
    fin->Close();
}

vector<pair<double, double>> MIPfinder::getExtremum(const int ch, const string gain)
{
    vector<pair<double, double>> ret;
    TH1F *h = h1[ch][gain];
    float width = h->GetBinWidth(1);
    float step = width*3;
    int nBins = h->GetNbinsX();

    // pedestal peak
    int b1 = h->GetMaximumBin();
    double y1 = h->GetMaximum();
    ret.push_back({h->GetBinCenter(b1), y1});

    // Gaussian width
    int b2 =  b1 + 1;
    double y2;
    while (b2 < nBins)
    {
	y2 = h->GetBinContent(b2);
	if (abs(y2/y1 - 0.5) < 0.2)
	    break;
	b2 += 1;
    }
    if (b2 == nBins)
	b2 = b1 + 1;
    ret.push_back({h->GetBinCenter(b2), h->GetBinContent(b2)});

    // local minimum between ped peak and MIP peak
    int pre_b3 = b2;
    double pre_y3 = h->GetBinContent(pre_b3);
    int b3 = pre_b3 + 1;
    double y3;
    while (b3 <= nBins)
    {
	bool new_min = true;
	y3 = h->GetBinContent(b3);
	if (y3 > pre_y3)
	{
	    // jump over local maximum due to fluctuations
	    new_min = false;
	    int post_b3 = b3 + 1;
	    for (int i=0; i<30; i++)	// the #bins value depends on the total number of bins
	    {
		if (h->GetBinContent(post_b3) < pre_y3)
		{
		    b3 = post_b3;
		    new_min = true;
		    break;
		}
		post_b3++;
	    }
	}

	if (0 == y3)
	{
	    // jump over empty bins
	    int post_b3 = b3 + 1;
	    if (h->GetBinContent(post_b3) > y1/5000)
	    {
		// fake empty bin
		b3 = post_b3;
		continue;
	    }
	}

	if (new_min)
	{
	    pre_b3 = b3;
	    pre_y3 = h->GetBinContent(pre_b3);
	    b3++;
	}
	else
	    break;
    }
    b3 = pre_b3;
    ret.push_back({h->GetBinCenter(b3), 0});

    // MIP peak
    int b4 = b3 + 1;
    double xmin = h->GetXaxis()->GetXmin();
    double xmax = h->GetXaxis()->GetXmax();
    h->GetXaxis()->SetRangeUser(h->GetBinLowEdge(b4), xmax*0.9);
    b4 = h->GetMaximumBin();
    double y4 = h->GetBinContent(b4);
    if (y4/y1 < 1e-3)
	return ret;
    h->GetXaxis()->SetRangeUser(xmin, xmax);

    // Landau width
    int b5 = b4+1;
    while (b5 < nBins && h->GetBinContent(b5) > y4/5)
	b5++;

    ret.push_back({h->GetBinCenter(b5), 0});
    return ret;
}

void MIPfinder::findMIP()
{
    cout << INFO << "finding MIPs" << endl;
    map<string, float> xmax = {
	{"LG", 700},
	{"HG", 7800},
    };

    for (int ch=0; ch<calo::nChannels; ch++)
    {
	for (const char* gain : calo::gains)
	{
	    TH1F *h = h1[ch][gain];
	    int nBins = h->GetNbinsX();

	    // pedestal peak
	    int b1 = h->GetMaximumBin();
	    double y1 = h->GetMaximum();
	    pedPeak[ch][gain] = h->GetBinCenter(b1);
	    g1[ch][gain]->SetPoint(0, pedPeak[ch][gain], y1);

	    // local minimum between ped peak and MIP peak
	    int pre_b2 = b1;
	    double pre_y2 = h->GetBinContent(pre_b2);
	    int b2 = pre_b2 + 1;
	    double y2;
	    while (b2 <= nBins)
	    {
		bool new_min = true;
		y2 = h->GetBinContent(b2);
		if (y2 > pre_y2)
		{
		    // jump over local maximum due to fluctuations
		    new_min = false;
		    int post_b2 = b2 + 1;
		    for (int i=0; i<6; i++)
		    {
			if (h->GetBinContent(post_b2) < pre_y2)
			{
			    b2 = post_b2;
			    new_min = true;
			    break;
			}
			post_b2++;
		    }
		}

		if (0 == y2)
		{
		    // jump over empty bins
		    int post_b2 = b2 + 1;
		    if (h->GetBinContent(post_b2) > y1/5000)
		    {
			// fake empty bin
			b2 = post_b2;
			continue;
		    }
		}

		if (new_min)
		{
		    pre_b2 = b2;
		    pre_y2 = h->GetBinContent(pre_b2);
		    b2++;
		}
		else
		    break;
	    }
	    b2 = pre_b2;

	    // MIP peak
	    double xmin = h->GetXaxis()->GetXmin();
	    double xmax = h->GetXaxis()->GetXmax();
	    h->GetXaxis()->SetRangeUser(h->GetBinLowEdge(b2), h->GetBinCenter(nBins)*0.9);
	    int b3 = h->GetMaximumBin();
	    if (h->GetBinContent(b3)/y1 < 1e-3)
	    {
		mipPeak[ch][gain] = 0;
		MIP[ch][gain] = 0;
	    }
	    else
	    {
		mipPeak[ch][gain] = h->GetBinCenter(b3);
		MIP[ch][gain] = mipPeak[ch][gain] - pedPeak[ch][gain];
		g1[ch][gain]->SetPoint(1, mipPeak[ch][gain], h->GetBinContent(b3));
	    }
	    h->GetXaxis()->SetRangeUser(xmin, xmax);
	}
    }
}

/*
vector<pair<double, double>> MIPfinder::getExtremum(const int ch, const string gain)
{
    vector<pair<double, double>> ret;
    TH1F *h = h1[ch][gain];
    float width = h->GetBinWidth(1);
    float step = width*3;
    int nBins = h->GetNbinsX();

    // pedestal peak
    int b1 = h->GetMaximumBin();
    double y1 = h->GetMaximum();
    ret.push_back({h->GetBinCenter(b1), y1});

    // Gaussian width
    int b2 =  b1 + 1;
    double y2;
    while (b2 < nBins)
    {
	y2 = h->GetBinContent(b2);
	if (abs(y2/y1 - 0.5) < 0.2)
	    break;
	b2 += 1;
    }
    if (b2 == nBins)
	b2 = b1 + 1;
    ret.push_back({h->GetBinCenter(b2), h->GetBinContent(b2)});

    // local minimum between ped peak and MIP peak
    int pre_b3 = b2 + 1;
    double pre_y3 = h->GetBinContent(pre_b3);
    double x3 = h->GetBinLowEdge(pre_b3);
    // 5 bins to void local local minimum trap
    h->GetXaxis()->SetRangeUser(x3, x3 + step);
    int b3 = h->GetMinimumBin();
    double y3 = h->GetBinContent(b3);
    while (y3 < pre_y3)
    {	// this may crash if there is zero bin; enough statistics is required
	pre_b3 = b3;
	pre_y3 = y3;
	x3 += step;
	h->GetXaxis()->SetRangeUser(x3, x3 + step);
	b3 = h->GetMinimumBin();
	y3 = h->GetBinContent(b3);
    }
    ret.push_back({h->GetBinCenter(pre_b3), pre_y3});

    // MIP peak
    int b4 = pre_b3 + 1;
    h->GetXaxis()->SetRangeUser(h->GetBinLowEdge(b4), h->GetBinCenter(nBins)*0.9);
    b4 = h->GetMaximumBin();
    double y4 = h->GetBinContent(b4);
    if (y4/y1 < 1e-3)
	return ret;

    ret.push_back({h->GetBinCenter(b4), y4});

    // Landau width
    // higher end
    int b5High = b4+1;
    while (b5High < nBins && h->GetBinContent(b5High) > y4/2)
	b5High++;
    // lower end
    int b5Low = b4-1;
    while (b5Low > b3 && h->GetBinContent(b5Low) > y4/2)
	b5Low--;

    ret.push_back({width*(b5High - b5Low)/2.3548, 0});
    return ret;
}

void MIPfinder::findMIP_fit()	
{
    // find MIP by fitting the signal with the convolution of a Gaussian and a Landau distritbutions
    // which fails because the fitting is very unstable, it depends on the initial parameters and
    // the fit range.
    
    cout << INFO << "finding MIPs" << endl;
    map<string, float> xmax = {
	{"LG", 700},
	{"HG", 7800},
    };

    // for (int ch=0; ch<calo::nChannels; ch++)
    for (int ch=0; ch<64; ch++)
    {
	for (const char* gain : calo::gains)
	{
	    vector<pair<double, double>> extreme = getExtremum(ch, gain);
	    double p[5] = {0};

	    h1[ch][gain]->GetXaxis()->SetRangeUser(0, h1[ch][gain]->GetXaxis()->GetXmax());

	    p[0] = extreme[0].second;	// ped peak
	    p[1] = extreme[0].first;	// Gaus MP location
	    p[2] = extreme[1].first - extreme[0].first;	// Gaus sigma
	    fitNoise[ch][gain]  = new TF1(Form("ch%d_%s_noise", ch, gain), "[0]*TMath::Gaus(x, [1], [2])", 0, extreme[2].first);
	    fitNoise[ch][gain]->SetParameters(p[0], p[1], p[2]);
	    h1[ch][gain]->Fit(fitNoise[ch][gain], "q", "", 0, extreme[2].first);

	    if (extreme.size() != 5)	// no signal, bad channel
	    {
		MIP[ch][gain] = 0;
		continue;
	    }

	    p[0] = extreme[3].second;	// Landau peak
	    p[1] = extreme[3].first;	// Landau MP location
	    p[2] = extreme[4].first;	// Landau scale width
	    p[3] = fitNoise[ch][gain]->GetParameter(1);
	    p[4] = fitNoise[ch][gain]->GetParameter(2);
	    TF1Convolution f("landau", "gaus", extreme[2].first, xmax[gain], false);
	    // f.SetNofPointsFFT(1000);
	    fitSignal[ch][gain] = new TF1(Form("ch%d_%s_signal", ch, gain), f, extreme[2].first, xmax[gain], f.GetNpar());
	    fitSignal[ch][gain]->SetParameters(p[0], p[1], p[2]);
	    fitSignal[ch][gain]->FixParameter(3, p[3]);
	    fitSignal[ch][gain]->FixParameter(4, p[4]);
	    h1[ch][gain]->Fit(fitSignal[ch][gain], "q", "", extreme[2].first, xmax[gain]);

	    MIP[ch][gain] = fitSignal[ch][gain]->GetParameter(1);
	    cout << DEBUG << ch << "\t" << gain << "\t" << p[0] << "\t" << p[1] << "\t" << p[2] << "\t" << p[3] << "\t" << p[4] << "\t" << MIP[ch][gain] << endl;
	}
    }
}
*/

void MIPfinder::plot()
{
    cout << INFO << "making plots" << endl;
    gStyle->SetOptStat(100);
    gErrorIgnoreLevel = kWarning;
    TCanvas* c = new TCanvas("c", "c", 600*8, 600*(1 + (calo::nChannels-1)/8));	// 8 plots per row
    for (const char* gain : calo::gains)
    {
	c->Clear();
	c->SetTopMargin(0.12);
	c->SetBottomMargin(0.1);
	c->SetLeftMargin(0.1);
	c->SetRightMargin(0.1);
	c->Divide(8, 1 + (calo::nChannels-1)/8, 0, 0);
	for (int ch=0; ch<calo::nChannels; ch++)
	{
	    TVirtualPad *p = c->cd(ch+1);
	    p->SetLogy(1);

	    h1[ch][gain]->Draw();
	    p->Update();
	    TPaveStats *ps = (TPaveStats*)p->GetPrimitive("stats");
	    ps->SetName(Form("ch%d_%s", ch, gain));
	    ps->SetX1NDC(0.55);
	    ps->SetX2NDC(0.85);
	    ps->SetY1NDC(0.65);
	    ps->SetY2NDC(0.85);
	    TList* list = ps->GetListOfLines();
	    list->Remove(ps->GetLineWith("Mean"));
	    TLatex *l1 = new TLatex(0, 0, Form("Ped Peak = %.2f", pedPeak[ch][gain]));
	    l1->SetTextSize(0.03);
	    l1->SetTextColor(kRed);
	    list->Add(l1);
	    TLatex *l2 = new TLatex(0, 0, Form("MIP Peak = %.2f", mipPeak[ch][gain]));
	    l2->SetTextSize(0.03);
	    l2->SetTextColor(kRed);
	    list->Add(l2);
	    TLatex *l3 = new TLatex(0, 0, Form("MIP = %.2f", MIP[ch][gain]));
	    l3->SetTextSize(0.03);
	    l3->SetTextColor(kRed);
	    list->Add(l3);
	    p->Modified();
	    h1[ch][gain]->SetStats(0);

	    g1[ch][gain]->SetMarkerSize(4);
	    g1[ch][gain]->SetMarkerStyle(29);
	    g1[ch][gain]->SetMarkerColor(kRed);
	    g1[ch][gain]->Draw("Psame");
	}
	TLatex *title = new TLatex();
	title->SetTextSize(0.03);
	title->DrawLatexNDC(.45, 0.97, Form("%s: ADC", gain));

	c->SaveAs(Form("%s/MIP_%s.png", fdir.c_str(), gain));
    }
    delete c;
}

void MIPfinder::write()
{
    nlohmann::json fmip;
    for (int ch=0; ch<calo::nChannels; ch++)
	for (const char *gain : calo::gains)
	    fmip[gain][to_string(ch)] = MIP[ch][gain];

    ofstream fout(mipOut.c_str());
    fout << fmip;
    fout.close();
}
#endif

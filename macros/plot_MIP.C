/* plot channel-by-channel MIP values and their ratio to pedestal */
#include "db.h"
#include "cali.h"
#include "analysis.h"

#include "cali_style.C"

void plot_MIP(const int run = 2580) 
{
    gROOT->SetBatch(1);
    cali_style();

    caliDB db;
    if (db.getRunType(run) != "mip")
    {
	cerr << FATAL << "not a mip run: " << run << endl;
	exit(1);
    }
    string mipFile = cali::getFile(Form("Run%d_MIP.json", run));
    mip_t mip;
    getMIP(mipFile.c_str(), mip);

    int pedRun = db.getPedRun(run);
    string pedFile = cali::getFile(Form("Run%d_ped.json", pedRun));
    ped_t ped;
    getPedestal(pedFile.c_str(), ped);

    map<int, vector<pair<int, int>>> ranges;
    ranges[0] = {{7, 34}, {42, 55}, {124, 127}};
    ranges[1] = {{56, 83}, {91, 97}};
    ranges[2] = {{112, 123}, {128, 191}};
    ranges[3] = {{0, 6}, {35, 41}, {84, 90}, {98, 111}};
    map<int, const char *> labels = {
	{0, "Hex tile, 3mm SiPM"}, 
	{1, "Hex tile, 1.3mm SiPM"},
	{2, "Square tile, 3mm SiPM"},
	{3, "Hex tile, 3mm SiPM, Unpainted"}, 
    };
    map<int, TGraph*> g1;
    map<int, TGraph*> g2;
    map<int, float> mean;

    for (size_t i=0; i<4; i++)
    {
	g1[i] = new TGraph();
	g2[i] = new TGraph();

	int ipoint = 0;
	float sum = 0;
	int count = 0;
	float mip_value;
	for (const auto [start, end] : ranges[i])
	{
	    for (int ch=start; ch<=end; ch++)
	    {
		mip_value = mip[ch]["HG"];
		if (mip_value > 0)
		{
		    sum += mip_value;
		    count++;
		}
		g1[i]->SetPoint(ipoint, ch, mip_value);
		g2[i]->SetPoint(ipoint, ch, mip_value/ped[ch]["HG"].rms);
		ipoint++;
	    }
	}
	mean[i] = sum/count;
    }

    // plot
    TCanvas* c = new TCanvas("c", "c", 1600, 600);
    c->Divide(2, 1);

    c->cd(1);
    for (size_t i=0; i<4; i++)
    {
	g1[i]->SetMarkerStyle(markers[i]);
	g1[i]->SetMarkerColor(colors[i]);
	g1[i]->GetXaxis()->SetLimits(0, 200);

	if (0 == i)
	{
	    g1[i]->SetTitle(";Ch;HG MIP [ADC]");
	    g1[i]->Draw("AP");
	}
	else
	    g1[i]->Draw("P SAME");

	TLine *l = new TLine(0, mean[i], 200, mean[i]);
	l->SetLineColor(colors[i]);
	l->Draw();
    }

    c->cd(2);
    TLegend *l = new TLegend(0.38, 0.7, 0.88, 0.9);
    for (size_t i=0; i<4; i++)
    {
	g2[i]->SetMarkerStyle(markers[i]);
	g2[i]->SetMarkerColor(colors[i]);
	g2[i]->GetXaxis()->SetLimits(0, 200);

	if (0 == i)
	{
	    g2[i]->SetTitle(";Ch;HG MIP/Ped RMS");
	    g2[i]->Draw("AP");
	}
	else
	    g2[i]->Draw("P SAME");

	l->AddEntry(g2[i], labels[i], "p");
    }
    l->Draw();

    c->SaveAs(Form("Run%d_HG_MIP.pdf", run));
}

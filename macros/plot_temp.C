#include <nlohmann/json.hpp>
#include "calo.h"
#include "cali.h"
#include "db.h"
#include "cali_style.h"

using namespace std;

void plot_temp(const char* input = "temp.csv")
{
    gROOT->SetBatch(1);
    cali_style();

    // plot channel ratio over time
    map<string, TGraph*> g[3];
    map<string, int> ipoint[3];
    for (size_t brd=0; brd<3; brd++)
    {
	for (const char* var : {"BrdTemp", "DetTemp", "FPGATemp", "HVTemp"})
	{
	    ipoint[brd][var] = 0;
	    g[brd][var] = new TGraph();
	    g[brd][var]->SetTitle(Form(";Run;%s Temp", var));
	    g[brd][var]->SetMinimum(0);
	    g[brd][var]->SetMaximum(100);
	    // g[brd][var]->GetXaxis()->SetTimeDisplay(1);
	    // g[brd][var]->GetXaxis()->SetTimeFormat("%m/%d");
	    // g[brd][var]->GetXaxis()->SetTimeOffset(0, "gmt");
	    g[brd][var]->GetXaxis()->SetRangeUser(1575, 2236);
	    g[brd][var]->SetLineColor(colors[brd]);
	    g[brd][var]->SetFillColor(colors[brd]);
	    g[brd][var]->SetMarkerColor(colors[brd]);
	    g[brd][var]->SetMarkerStyle(markers[brd]);
	}
    }

    ifstream fin(input);
    int run;
    map<string, double>  val[3];
    while (fin >> run >> val[0]["Brd"] >> val[0]["BrdTemp"] >> val[0]["DetTemp"] >> val[0]["FPGATemp"] >> val[0]["HVTemp"] >> val[0]["Vmon"] >> val[0]["Imon"] >> val[0]["HVstatus"]
		      >> val[1]["Brd"] >> val[1]["BrdTemp"] >> val[1]["DetTemp"] >> val[1]["FPGATemp"] >> val[1]["HVTemp"] >> val[1]["Vmon"] >> val[1]["Imon"] >> val[1]["HVstatus"]
		      >> val[2]["Brd"] >> val[2]["BrdTemp"] >> val[2]["DetTemp"] >> val[2]["FPGATemp"] >> val[2]["HVTemp"] >> val[2]["Vmon"] >> val[2]["Imon"] >> val[2]["HVstatus"]
	  )
    {
	if (run < 1000) 
	{
	    cout << "DEBUG\t" << run << endl;
	    continue;
	}
	for (int brd=0; brd<3; brd++)
	{
	    for (const char* var : {"BrdTemp", "DetTemp", "FPGATemp", "HVTemp"})
	    {
		g[brd][var]->SetPoint(ipoint[brd][var], run, val[brd][var]);
		ipoint[brd][var]++;
	    }
	}
    }

    for (size_t brd=0; brd<3; brd++)
    {
	g[brd]["DetTemp"]->GetYaxis()->SetRangeUser(21, 26);
    }


    TCanvas *c = new TCanvas("c", "c", 1000, 600);
    TLegend *l = new TLegend(0.13, 0.2, 0.26, 0.38);
    l->SetTextSize(0.045);

    for (const char* var : {"BrdTemp", "DetTemp", "FPGATemp", "HVTemp"})
    {
	c->Clear();
	l->Clear();
	for (size_t brd=0; brd<2; brd++)
	{
	    if (0 == brd)
		g[brd][var]->Draw("AP");
	    else
		g[brd][var]->Draw("P SAME");

	    l->AddEntry(g[brd][var], Form("Brd %zu", brd), "p");
	}
	l->Draw();
	c->SaveAs(Form("%s.pdf", var));
    }
}

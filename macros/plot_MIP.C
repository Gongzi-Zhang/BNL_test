/* plot channel-by-channel MIP values and their ratio to pedestal */
#include <nlohmann/json.hpp> 
#include "db.h"
#include "cali.h"
#include "analysis.h"

#include "cali_style.h"

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

    string HGvsLGFile = cali::getFile("HG_vs_LG.json");
    ifstream fjson(HGvsLGFile);
    auto ratio = nlohmann::json::parse(fjson);
    fjson.close();
    map<int, double> HG_vs_LG;
    for (int ch=0; ch<calo::nChannels; ch++)
    {
	HG_vs_LG[ch] = ratio[to_string(ch)];
    }

    map<int, vector<pair<int, int>>> ranges;
    ranges[0] = {{0, 6}, {35, 41}, {84, 90}, {98, 111}};
    ranges[1] = {{7, 34}, {42, 55}, {124, 127}};
    ranges[2] = {{56, 83}, {91, 97}};
    ranges[3] = {{112, 123}, {128, 191}};
    map<int, const char *> labels = {
	{0, "Hex tile, unpainted; 3mm SiPM"}, 
	{1, "Hex tile; 3mm SiPM"}, 
	{2, "Hex tile; 1.3mm SiPM"},
	{3, "Square tile; 3mm SiPM"},
    };
    map<int, map<string, TGraph*>> g1;
    map<int, map<string, TGraph*>> g2;
    map<int, map<string, float>> mean;

    for (size_t i=0; i<4; i++)
    {
	for (const char* gain : {"HG", "LG"})
	{
	    g1[i][gain] = new TGraph();
	    g2[i][gain] = new TGraph();
	}

	map<string, float> sum;
	map<string, int> count;
	float HG_mip, LG_mip;
	for (const auto [start, end] : ranges[i])
	{
	    for (int ch=start; ch<=end; ch++)
	    {
		HG_mip = mip[ch]["HG"];
		if (HG_mip == 0)
		    continue;

		sum["HG"] += HG_mip;
		g1[i]["HG"]->SetPoint(count["HG"], ch, HG_mip);
		g2[i]["HG"]->SetPoint(count["HG"], ch, HG_mip/ped[ch]["HG"].rms);
		count["HG"]++;

		if (HG_vs_LG[ch] == 0)
		{
		    cout << WARNING << ch << "\t" << HG_vs_LG[ch] << endl;
		    continue;
		}
		LG_mip = HG_mip / HG_vs_LG[ch];
		sum["LG"] += LG_mip;
		g1[i]["LG"]->SetPoint(count["LG"], ch, LG_mip);
		g2[i]["LG"]->SetPoint(count["LG"], ch, LG_mip/ped[ch]["LG"].rms);
		count["LG"]++;
	    }
	}

	for (const char* gain : {"HG", "LG"})
	{
	    mean[i][gain] = sum[gain]/count[gain];
	    cout << DEBUG << i << "\t" << gain << "\t" << mean[i][gain] << endl;
	}
    }

    map<int, map<string, float>> error;
    for (size_t i=0; i<4; i++)
    {
	float HG_mip, LG_mip;
	for (const auto [start, end] : ranges[i])
	{
	    for (int ch=start; ch<=end; ch++)
	    {
		HG_mip = mip[ch]["HG"];
		if (HG_mip == 0)
		    continue;

		double e = abs(HG_mip - mean[i]["HG"])/mean[i]["HG"];
		if (e > error[i]["HG"])
		    error[i]["HG"] = e;

		if (HG_vs_LG[ch] == 0)
		{
		    cout << WARNING << ch << "\t" << HG_vs_LG[ch] << endl;
		    continue;
		}

		LG_mip = HG_mip / HG_vs_LG[ch];
		e = abs(LG_mip - mean[i]["LG"])/mean[i]["LG"];
		if (e > error[i]["LG"])
		    error[i]["LG"] = e;
	    }
	}

	for (const char* gain : {"HG", "LG"})
	    cout << INFO << i << "\t" << gain << "\t" << error[i][gain]*100 << endl;
    }
    // plot
    TCanvas* c = new TCanvas("c", "c", 1600, 600);
    c->SetMargin(0, 0, 0, 0);
    c->Divide(2, 1);

    TLegend *lg = new TLegend(0.12, 0.7, 0.58, 0.9);
    lg->SetTextSize(0.045);

    map<string, double> ymax = {
	{ "LG", 260},
	{ "HG", 7400},
    };
    for (const char* gain : {"HG", "LG"})
    {
	lg->Clear();

	c->cd(1);
	for (size_t i=0; i<4; i++)
	{
	    g1[i][gain]->SetMarkerStyle(markers[i]);
	    g1[i][gain]->SetMarkerColor(colors[i]);
	    g1[i][gain]->GetXaxis()->SetLimits(0, 200);

	    if (0 == i)
	    {
		g1[i][gain]->SetTitle(Form(";Ch;%s MIP [ADC]", gain));
		g1[i][gain]->GetYaxis()->SetRangeUser(0, ymax[gain]);
		g1[i][gain]->GetYaxis()->SetTitleOffset(1.05);
		g1[i][gain]->Draw("AP");
	    }
	    else
		g1[i][gain]->Draw("P SAME");

	    TLine *l = new TLine(0, mean[i][gain], 200, mean[i][gain]);
	    l->SetLineColor(colors[i]);
	    l->Draw();

	    lg->AddEntry(g1[i][gain], labels[i], "p");
	}
	lg->Draw();

	c->cd(2);
	for (size_t i=0; i<4; i++)
	{
	    g2[i][gain]->SetMarkerStyle(markers[i]);
	    g2[i][gain]->SetMarkerColor(colors[i]);
	    g2[i][gain]->GetXaxis()->SetLimits(0, 200);
	    g2[i][gain]->GetYaxis()->SetRangeUser(0, 33);

	    if (0 == i)
	    {
		g2[i][gain]->SetTitle(Form(";Ch;%s MIP/Ped RMS", gain));
		g2[i][gain]->Draw("AP");
	    }
	    else
		g2[i][gain]->Draw("P SAME");
	}

	c->SaveAs(Form("Run%d_%s_MIP.pdf", run, gain));
    }
}

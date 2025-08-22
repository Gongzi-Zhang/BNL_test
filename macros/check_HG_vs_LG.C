#include <nlohmann/json.hpp>
#include "calo.h"
#include "cali.h"
#include "db.h"
#include "cali_style.h"

using namespace std;

void check_HG_vs_LG()
{
    gROOT->SetBatch(1);
    cali_style();

    caliDB db;
    vector<int> runs = db.getRuns("Run < 2000 AND Run > 1000 AND Type = 'data' AND Flag = 'good' AND Events > 70000 AND Events < 200000");

    map<int, double> sum;
    map<int, size_t> count;
    map<int, double> mean;
    for (int ch=0; ch<calo::nChannels; ch++)
    {
	sum[ch] = 0;
	count[ch] = 0;
	mean[ch] = 0;
    }

    // plot channel ratio over time
    size_t selectedChannels[] = {25, 62, 100, 160};
    const int nChannels = sizeof(selectedChannels)/sizeof(selectedChannels[0]);
    map<size_t, TGraph*> g;
    map<size_t, int> ipoint;
    for (size_t i=0; i<nChannels; i++)
    {
        size_t ch = selectedChannels[i];
	ipoint[ch] = 0;
        g[ch] = new TGraph();
        g[ch]->SetTitle(";Run;HG/LG");
        g[ch]->SetMinimum(0);
        g[ch]->SetMaximum(45);
        // g[ch]->GetXaxis()->SetTimeDisplay(1);
        // g[ch]->GetXaxis()->SetTimeFormat("%m/%d");
        // g[ch]->GetXaxis()->SetTimeOffset(0, "gmt");
        g[ch]->GetXaxis()->SetRangeUser(1000, 2000);
        g[ch]->SetLineColor(colors[i]);
        g[ch]->SetFillColor(colors[i]);
        g[ch]->SetMarkerColor(colors[i]);
        g[ch]->SetMarkerStyle(markers[i]);
    }

    for (const auto run : runs)
    {
	string jsonFile = cali::getFile(Form("Run%d_HG_vs_LG.json", run));
	if (jsonFile.empty())
	    continue;

	// string datetime = db.getRunStartTime(run);
	// int year, month, day, hour, minute;
        // sscanf(datetime.c_str(), "%4d-%2d-%2d %2d:%2d", &year, &month, &day, &hour, &minute);
        // TDatime dt(year, month, day, 0, 0, 0);

	ifstream fjson(jsonFile);
	auto HG_vs_LG = nlohmann::json::parse(fjson);
	fjson.close();

	for (int ch=0; ch<calo::nChannels; ch++)
	{
	    double ratio = HG_vs_LG[to_string(ch)];
	    if (ratio < 20)
		continue;

	    if (mean[ch] > 0 && abs(ratio - mean[ch])/mean[ch] > 0.3)
	    {
		continue;
		cout << WARNING << run << "\t" << ch << "\t" << ratio << "\t" << mean[ch] << endl;
	    }

	    sum[ch] += ratio;
	    count[ch] += 1;
	    mean[ch] = sum[ch] / count[ch];
	}

	for (const auto ch : selectedChannels)
	{
	    double ratio = HG_vs_LG[to_string(ch)];
	    if (ratio < 20)
		continue;

	    if (mean[ch] > 0 && abs(ratio - mean[ch])/mean[ch] > 0.3)
		continue;

	    g[ch]->SetPoint(ipoint[ch], run, HG_vs_LG[to_string(ch)]);
	    ipoint[ch]++;
	}
    }
    
    nlohmann::json HG_vs_LG;
    for (int ch=0; ch<calo::nChannels; ch++)
    {
	HG_vs_LG[to_string(ch)] = mean[ch];
	if (mean[ch] < 20)
	    cout << WARNING << ch << "\t" << mean[ch] << endl;
    }

    ofstream fout(Form("%s/data/HG_vs_LG.json", cali::CALIROOT));
    fout << HG_vs_LG;
    fout.close();

    TCanvas *c = new TCanvas("c", "c", 1000, 600);
    TLegend *l = new TLegend(0.13, 0.2, 0.26, 0.38);
    l->SetTextSize(0.045);

    for (size_t i=0; i<nChannels; i++)
    {
        size_t ch = selectedChannels[i];
        if (0 == i)
            g[ch]->Draw("AP");
        else
            g[ch]->Draw("P SAME");

        l->AddEntry(g[ch], Form("ch %zu", ch), "p");
    }
    l->Draw();
    c->SaveAs("HG_vs_LG_over_time.pdf");
}

#include "cali.h"

const int nChannels = 192;
const float mip_cut = 0.3;
const int colors[] = {1, 2, 3, 4, 6, 7, 8, 9};
const int markers[] = {20, 21, 22, 23, 33, 34, 35};

gROOT->SetBatch(1);

void rate_scan()                                                           
{                                                                               
    gROOT->SetBatch(1);
    string inputs[] = {"T1.txt", "T2.txt", "T3.txt"};                            
    string legends[] = {"T1", "T2", "T3"};                                       
    TGraph* g[3];                                                               
    float volt, rate;                                                           
    for (int i=0; i<3; i++)                                                     
    {                                                                           
        g[i] = new TGraph();                                                    
        ifstream fin(inputs[i]);                                                 
        int ipoint = 0;                                                         
        while (fin >> volt >> rate)                                             
            g[i]->SetPoint(ipoint++, volt, rate);                               
        fin.close();                                                            
    }                                                                           
                                                                                
    TCanvas *c = new TCanvas("c", "c", 800, 600);                               
    TLegend *l = new TLegend(0.8, 0.7, 0.9, 0.9);                               
    for (int i=0; i<3; i++)                                                     
    {                                                                           
	g[i]->SetMarkerStyle(20);
        g[i]->SetMarkerColor(colors[i]);                                         
        g[i]->SetLineColor(colors[i]);                                           
        if (0 == i)                                                             
	{
	    g[i]->SetTitle("Threshold Scan;Voltage;Rate");
            g[i]->Draw("AP");                                                   
	}
        else                                                                    
            g[i]->Draw("P same");                                               
        l->AddEntry(g[i], legends[i].c_str(), "p");                                      
    }                                                                           
    l->Draw();                                                                  
    c->SetLogx();
    c->Update();
    c->SaveAs("scan.png");                                                      
}


void energy_scan()
{
    // int runs[] = {1856, 1857, 1858, 1859, 1860}; // T1
    // int runs[] = {1954, /* 1955, 1956, */ 1962, 1961, 1960, 1959, 1958}; // T1
    // int runs[] = {1866, 1867, 1868, 1869, 1870}; // T2
    // int runs[] = {1877, 1878, 1879, 1880, 1881}; // T3
    int runs[] = {2053, 1963, 2055, 2056, 2058, 1968};
    map<int, string> legend = {
	{1856, "T1 = 0.05 V"},
	{1857, "T1 = 0.08 V"},
	{1858, "T1 = 0.1 V"},
	{1859, "T1 = 0.2 V"},
	{1860, "T1 = 0.5 V"},

	{1866, "T2 = 0.05 V"},
	{1867, "T2 = 0.08 V"},
	{1868, "T2 = 0.1 V"},
	{1869, "T2 = 0.2 V"},
	{1870, "T2 = 0.5 V"},

	{1876, "T3 = 0.05 V"},
	{1877, "T3 = 0.08 V"},
	{1878, "T3 = 0.1 V"},
	{1879, "T3 = 0.2 V"},
	{1880, "T3 = 0.3 V"},
	{1881, "T3 = 0.4 V"},

	{1954, "T1 = 0.02 V"},
	{1955, "T1 = 0.03 V"},
	{1956, "T1 = 0.04 V"},
	{1962, "T1 = 0.05 V"},
	{1961, "T1 = 0.08 V"},
	{1960, "T1 = 0.1 V"},
	{1959, "T1 = 0.2 V"},
	{1958, "T1 = 0.3 V"},

	{1963, "T3 = 0.08 V"},
	{1968, "T3 = 0.3 V"},
	{2003, "T3 = 0.2 V"},
	{2018, "T3 = 0.1 V"},
	{2053, "T3 = 0.06 V"},
	{2054, "T3 = 0.07 V"},
	{2055, "T3 = 0.09 V"},
	{2056, "T3 = 0.1 V"},
	{2058, "T3 = 0.2 V"},
    };

    float mip[nChannels];
    float event_energy = 0;
    map<int, TH1F*> h1;
    for (int run : runs)
    {
	h1[run] = new TH1F(Form("event_energy_%d", run), "Event Energy;MIP", 400, 0, 400);
	string fname = cali::getRootFile(run);
	TFile *fin = new TFile(fname.c_str(), "read");
	TTree *tin = (TTree*) fin->Get("mip");
	for (int ch=0; ch<nChannels; ch++)
	{
	    tin->GetBranch(Form("ch_%d", ch))->GetLeaf("LG")->SetAddress(&mip[ch]);
	}

	for (int ei=0; ei<tin->GetEntries(); ei++)
	{
	    tin->GetEntry(ei);
	    event_energy = 0;
	    for (int ch=0; ch<nChannels; ch++)
	    {
		if (mip[ch] < mip_cut)
		    continue;
		event_energy += mip[ch];
	    }
	    if (event_energy > 0)
		h1[run]->Fill(event_energy);
	}
	h1[run]->Scale(1./tin->GetEntries());
    }

    TCanvas *c = new TCanvas("c", "c", 800, 600);
    TLegend *l = new TLegend(0.7, 0.6, 0.9, 0.9);
    bool first = true;
    int ci = 0;
    for (int run : runs)
    {
	h1[run]->SetMarkerStyle(markers[ci]);
	h1[run]->SetMarkerColor(colors[ci]);
	h1[run]->SetLineColor(colors[ci]);
	if (first)
	{
	    h1[run]->Draw("P0 HIST");
	    h1[run]->SetStats(0);
	    first = false;
	}
	else
	{
	    h1[run]->Draw("P0 HIST SAME");
	}

	l->AddEntry(h1[run], legend[run].c_str(), "lp");
	ci++;
    }
    l->Draw();
    c->SetLogy(1);
    c->SaveAs("threshold_scan.png");
}

void threshold_scan()
{
    energy_scan();
}

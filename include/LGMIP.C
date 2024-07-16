#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <nlohmann/json.hpp>
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TLeaf.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TF1.h"
#include "TCanvas.h"
#include "utilities.h"
#include "calo.h"
#include "cali.h"
#include "analysis.h"

using namespace std;

void usage()
{
    cout << INFO << "usage" << endl;
    cout << "\tbin/LGMIP -m mipRun -f rootfile -p figure_prefix" << endl;
}

int main(int argc, char *argv[])
{
    int mipRun = 0;
    string rootFile;
    string prefix("run");
    char opt;
    while ((opt = getopt(argc, argv, "f:m:p:")) != -1)
	switch(opt)
	{
	    case 'm':
		mipRun = stoi(optarg);
		cali::setRun(mipRun);
		break;
	    case 'f':
		rootFile = optarg;
		break;
	    case 'p':
		prefix = optarg;
		break;
	    default:
		usage();
		exit(0);
	}

    mip_t inMIP;
    if (0 == mipRun)
    {
	cerr << FATAL << "no mip run specified" << endl;
	exit(4);
    }
    if (!getMIP(mipRun, inMIP))
    {
	cerr << FATAL << "unable to read mip" << endl;
	exit(2);
    }

    if (rootFile.empty())
    {
	cerr << FATAL << "no root file specified" << endl;
	exit(4);
    }

    char fdir[1024];
    sprintf(fdir, "%s/figures/misc", cali::CALIROOT);
    if (!dirExists(fdir))
	mkdir(fdir, 0755);

    TFile *fin = new TFile(rootFile.c_str(), "read");
    if (fin->IsOpen())
    {
	cerr << FATAL << "can't open the root file: " << rootFile << endl;
	exit(4);
    }
    map<int, map<string, float>> corADC;
    map<int, TH2F*> h2;
    TTree* tcor = (TTree*) fin->Get("cor");
    for (int ch=0; ch<calo::nChannels; ch++)
    {
	for (const char* gain : calo::gains)
	{
	    corADC[ch][gain] = 0;
	    tcor->GetBranch(Form("ch_%d", ch))->GetLeaf(gain)->SetAddress(&corADC[ch][gain]);
	}
	h2[ch] = new TH2F(Form("ch_%d", ch), Form("ch %d", ch), 100, 0, 500, 100, 0, 8000);
    }

    for (int ei=0; ei<tcor->GetEntries(); ei++)
    {
	tcor->GetEntry(ei);
	for (int ch=0; ch<calo::nChannels; ch++)
	{
	    for (const char *gain : calo::gains)
	    {
		if (corADC[ch]["LG"] > 0 && corADC[ch]["HG"] < 7800)
		    h2[ch]->Fill(corADC[ch]["LG"], corADC[ch]["HG"]);
	    }
	}
    }
    fin->Close();

    int ncol = 8;
    int nrow = 1 + (calo::nChannels - 1)/ncol;
    TCanvas *c = new TCanvas("c", "c", ncol*600, nrow*600);
    c->Divide(ncol, nrow, 0, 0);
    for (int ch=0; ch<calo::nChannels; ch++)
    {
	if (h2[ch]->GetEntries() < 1000)
	{
	    cerr << WARNING << "not enough statistics in channel: " << ch << endl;
	    continue;
	}

	c->cd(ch+1);
	h2[ch]->Draw("COLZ");
	TProfile * proX = h2[ch]->ProfileX(Form("ch%d_profileX", ch));
	TF1 *fit = new TF1(Form("ch%d_fit", ch), "[0] + [1]*x", 0, 500);
	fit->SetParameters(0, 30);
	proX->Fit(fit, "q");
	cout << INFO << "update MIP value in channel " << ch << " from: " << inMIP[ch]["LG"] << "\tto\t";
	inMIP[ch]["LG"] = inMIP[ch]["HG"] / fit->GetParameter(1);
	cout << inMIP[ch]["LG"] << endl;
    }
    c->SaveAs(Form("%s/%s_HG_vs_LG.png", fdir, prefix.c_str()));

    // write result
    nlohmann::json outMIP;
    for (int ch=0; ch<calo::nChannels; ch++)
	for (const char *gain : calo::gains)
	    outMIP[gain][to_string(ch)] = inMIP[ch][gain];

    char outFile[1024];
    sprintf(outFile, "%s/data/Run%d_MIP_1.json", cali::CALIROOT, mipRun);
    ofstream fout(outFile);
    fout << outMIP;
    fout.close();
}

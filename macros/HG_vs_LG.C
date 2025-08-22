#include <nlohmann/json.hpp>
#include "calo.h"
#include "cali.h"
#include "analysis.h"

using namespace std;

void HG_vs_LG(const char* input, const char* output)
{
    gROOT->SetBatch(1);

    TFile *fin = new TFile(input, "read");
    TTree *tcor = (TTree*) fin->Get("cor");

    map<int, map<string, float>> corADC;
    map<int, TH2F*> h2;
    for (int ch=0; ch<calo::nChannels; ch++)
    {
	for (const char* gain : calo::gains)
	{
	    corADC[ch][gain] = 0;
	    tcor->GetBranch(Form("ch_%d", ch))->GetLeaf(gain)->SetAddress(&corADC[ch][gain]);
	}
	h2[ch] = new TH2F(Form("ch_%d", ch), Form("ch %d", ch), 100, 0, 800, 100, 0, 8000);
    }

    for (int ei=0; ei<tcor->GetEntries(); ei++)
    {
        tcor->GetEntry(ei);
	for (int ch=0; ch<calo::nChannels; ch++)
	{
	    if (corADC[ch]["LG"] > 0 && corADC[ch]["HG"] < 7900)
		h2[ch]->Fill(corADC[ch]["LG"], corADC[ch]["HG"]);
	}
    }

    nlohmann::json fmip;
    for (int ch=0; ch<calo::nChannels; ch++)
    {
	TProfile * proX = h2[ch]->ProfileX(Form("ch%d_profileX", ch));
	TF1 *fit = new TF1(Form("ch%d_fit", ch), "[0] + [1]*x", 0, 800);
	fit->SetParameters(0, 30);
	proX->Fit(fit, "q");
	if (h2[ch]->GetEntries() > 300)
	    fmip[to_string(ch)] = fit->GetParameter(1);
	else
	    fmip[to_string(ch)] = 0;

    }

    ofstream fout(output);
    fout << fmip;
    fout.close();
}

void HG_vs_LG(const int run)
{
    string rootFile = cali::getFile(Form("Run%d.root", run));
    string output = Form("%s/data/Run%d_HG_vs_LG.json", cali::CALIROOT, run);
    HG_vs_LG(rootFile.c_str(), output.c_str());
}

#include "cali.h"
#include "caliType.h"

void cluster_QA(const int run) 
{
    gROOT->SetBatch(1);
    string fileName = cali::getFile(Form("Run%d.rec.root", run));
    TFile *fin = new TFile(fileName.c_str(), "read");
    TTree *tin = (TTree*) fin->Get("cluster");

    TClonesArray *clus = new TClonesArray("caliCluster");
    tin->SetBranchAddress("CALIClusters", &clus);

    map<string, TH1F*> h1;
    h1["nclusters"] = new TH1F("nclusters", "nclusters", 10, 0, 10);
    h1["clu_nhits"] = new TH1F("clu_nhits", "clu nhits", 70, 0, 70);
    h1["clu_e"] = new TH1F("clu_e", "clu energy;MIP", 100, 0, 1000);

    const int N = tin->GetEntries();
    for (int ei=0; ei<N; ei++)
    {
	tin->GetEntry(ei);
	int nc = clus->GetEntries();
	h1["nclusters"]->Fill(nc);
	for (int ci=0; ci<nc; ci++)
	{
	    caliCluster *clu = (caliCluster*) clus->At(ci);
	    h1["clu_nhits"]->Fill(clu->nhits);
	    h1["clu_e"]->Fill(clu->e);
	}
    }

    TCanvas* c = new TCanvas("c", "c", 800, 600);
    for (auto var : {"nclusters", "clu_nhits", "clu_e"})
    {
	h1[var]->Draw();
	c->SaveAs(Form("%s.png", var));
    }
}

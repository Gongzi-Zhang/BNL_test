// print SiPM position in simulation
#include "cali.h"
#include "calisim.h"

void printSipmPos(const char* fname)
{
    gROOT->SetBatch(1);

    TCanvas *c[cali::nLayers];
    pair<float, float> quaPos[] = {
	{ 5,  6},
	{-5,  6},
	{-5, -6},
	{ 5, -6},
    };
    for (int l=0; l<cali::nLayers; l++)
    {
	c[l] = new TCanvas(Form("c%d", l), Form("Layer %d", l), 800, 600);
	c[l]->Range(-10, -10, 10, 10);
	TLatex *t = new TLatex(0, 9, Form("Layer %d", l+1));
	t->SetTextAlign(22);
	t->Draw();

	for (int b=0; b<nLayerBoards; b++)
	{
	    TLatex *t1 = new TLatex(quaPos[b].first, quaPos[b].second, Form("%d", cali::boardLabel[l][b]));
	    t1->SetTextColor(kRed);
	    t1->Draw();
	}
    }

    TFile *fin = new TFile(fname, "read");
    TTree *tin = (TTree*) fin->Get("events");
    TTreeReader tr(tin);

    TTreeReaderArray<unsigned long> hit_cellID(tr, "CALIHits.cellID");
    TTreeReaderArray<float> hit_x(tr, "CALIHits.position.x");
    TTreeReaderArray<float> hit_y(tr, "CALIHits.position.y");

    bool visits[cali::nChannels];
    for (size_t ch=0; ch<cali::nChannels; ch++)
	visits[ch] = false;

    const int ne = tin->GetEntries();
    int ch = 0;
    for (int ei = 0; ei<ne; ei++)
    {
	cout << DEBUG << ei << endl;
	tr.Next();

	for (int hi=0; hi<hit_cellID.GetSize(); hi++)
	{
	    ch = calisim::getChId(hit_cellID[hi]);
	    if (ch < 0 || visits[ch])
		continue;

	    x = hit_x[hi] - cali::x0;
	    y = hit_y[hi] - cali::y0;
	    z = round((hit_z[hi] - cali::z0) / cali::lt);  // round to closet integer
            c[z]->cd();
	    TLatex *text = new TLatex(-pos.x/cm, pos.y/cm, Form("%d", ch));
            c[z]->Update();

	    visits[ch] = true;
	}
    }

    c[0]->Print("SipmPos.pdf(", "pdf");
    for (int l=1; l<cali::nLayers-1; l++)
	c[l]->Print("SipmPos.pdf", "pdf");
    c[cali::nLayers-1]->Print("SipmPos.pdf)", "pdf");
}

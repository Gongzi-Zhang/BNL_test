#include "cali.h"

using namespace cali;
void printSipmPos()
{
    gROOT->SetBatch(1);
    setRun(1000);
    nLayers = 15;
    nChannels = 268;
    TCanvas *c[nLayers];
    pair<float, float> quaPos[] = {
	{ 5,  6},
	{-5,  6},
	{-5, -6},
	{ 5, -6},
    };
    for (int l=0; l<nLayers; l++)
    {
	c[l] = new TCanvas(Form("c%d", l), Form("Layer %d", l), 800, 600);
	c[l]->Range(-10, -10, 10, 10);
	TLatex *t = new TLatex(0, 9, Form("Layer %d", l+1));
	t->SetTextAlign(22);
	t->Draw();

	for (int b=0; b<nLayerBoards; b++)
	{
	    TLatex *t1 = new TLatex(quaPos[b].first, quaPos[b].second, Form("%d", boardLabel[l][b]));
	    t1->SetTextColor(kRed);
	    t1->Draw();
	}
    }

    for (int ch=0; ch<nChannels; ch++)
    {
	SiPM sp = getSipm(ch);
	sipmXY pos = getSipmXY(ch);
	int layer = sp.layer;
	c[layer]->cd();
	TLatex *text = new TLatex(-pos.x/cm, pos.y/cm, Form("%d", ch));
	text->Draw();
	c[layer]->Update();
    }

    c[0]->Print("SipmPos.pdf(", "pdf");
    for (int l=1; l<nLayers-1; l++)
	c[l]->Print("SipmPos.pdf", "pdf");
    c[nLayers-1]->Print("SipmPos.pdf)", "pdf");
}

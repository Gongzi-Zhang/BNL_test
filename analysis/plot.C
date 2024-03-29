const int nChannels = 64;

void plot(string inFile)
{
    gROOT->SetBatch(1);

    TFile *fin = new TFile(inFile.c_str(), "read");

    TCanvas *c = new TCanvas("c", "c", 800, 600);
    for (int ch=0; ch<nChannels; ch++)
    {
	TH1F* h = (TH1F*) fin->Get(Form("Ch_%d_LG", ch));
	h->Draw();
	c->SaveAs(Form("Ch_%d_LG.png", ch));
    }
}

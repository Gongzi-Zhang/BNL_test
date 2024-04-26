const int nBoards = 3;
const int nChannels = nBoards*64;

void plot(string inFile)
{
    gROOT->SetBatch(1);
    gStyle->SetOptStat(111110);

    TFile *fin = new TFile(inFile.c_str(), "read");

    TCanvas *c = new TCanvas("c", "c", 3200, 3200);
    c->Divide(8, 8);
    for (int bd=0; bd<nBoards; bd++)
    {
        for (int i=0; i<64; i++)
        {
            c->cd(i+1);
	    int ch = i + 64*bd;
            TH1F* h = (TH1F*) fin->Get(Form("Ch_%d_LG", ch));
            h->Draw();
        }
        c->SaveAs(Form("Bd_%d_LG.png", bd));
    }
}

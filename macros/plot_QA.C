/* plot channel-by-channel MIP values and their ratio to pedestal */
#include "cali.h"
#include "analysis.h"

#include "cali_style.h"

void plot_QA(const int run = 1227) 
{
    gROOT->SetBatch(1);
    TGaxis::SetMaxDigits(3);
    cali_style();

    string QAFile = cali::getFile(Form("Run%d_QA.root", run));
    TFile *fin = new TFile(QAFile.c_str(), "read");

    // rate plot
    TCanvas* c = new TCanvas("c", "c", 800, 600);
    c->SetLeftMargin(0.08);
    TH1F* h1 = (TH1F*) fin->Get("event_rate");
    h1->SetTitle(";BNL Time;Rate [Hz]");
    h1->SetMaximum(h1->GetMaximum()*1.1);
    h1->SetStats(false);
    h1->GetXaxis()->SetTimeDisplay(1);
    h1->GetXaxis()->SetTimeFormat("%H:%M");
    h1->GetXaxis()->SetTitleSize(0.04);
    h1->GetXaxis()->SetTitleOffset(1.1);
    h1->GetYaxis()->SetTitleSize(0.04);
    h1->GetYaxis()->SetTitleOffset(0.9);
    h1->Draw("HIST");

    // c->Update();
    // TPaveText* pt = (TPaveText*) c->GetPrimitive("title");
    // pt->SetTextSize(0.07);
    // pt->SetY1NDC(0.8);
    // c->Modified();

    c->SaveAs(Form("Run%d_rate.pdf", run));


    c->Clear();
    TGaxis::SetMaxDigits(4);
    c->SetLeftMargin(0.1);
    c->SetRightMargin(0.11);

    TH2F* h2 = (TH2F*) fin->Get("event_MIP_vs_hit_mul");
    h2->SetTitle(";Hit Multiplicity;Event Energy [MIP]");
    h2->SetStats(false);
    h2->GetXaxis()->SetRangeUser(0, 60);
    h2->GetXaxis()->SetTitleSize(0.04);
    h2->GetXaxis()->SetTitleOffset(1.1);
    h2->GetYaxis()->SetRangeUser(0, 600);
    h2->GetYaxis()->SetTitleSize(0.04);
    h2->GetYaxis()->SetTitleOffset(1.2);
    h2->Draw("colz");

    // c->Update();
    // pt = (TPaveText*) c->GetPrimitive("title");
    // pt->SetTextSize(0.07);
    // pt->SetY1NDC(0.8);

    c->SaveAs(Form("Run%d_event_MIP_vs_hit_mul.pdf", run));
}

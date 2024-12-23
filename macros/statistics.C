#include "utilities.h"
#include "db.h"
#include "cali.h"
#include "cali_style.h"

int fill(TGraph* g, const int startRun, const int endRun, int shift = 0)
{
    if (!g)
    {
	cerr << ERROR << "empty graph" << endl;
	return -1;
    }

    caliDB db;
    string datetime;
    int year, month, day, hour, minute;
    int preMonth, preDay;
    int total = shift;
    int ipoint = 0;
    int run = startRun;

    // find the date of the first good run
    for (; run<=endRun; run++)
    {
	if (db.getRunType(run) != "data"
	 || db.getRunFlag(run) != "good")
	    continue;

	datetime = db.getRunStartTime(run);
	sscanf(datetime.c_str(), "%4d-%2d-%2d %2d:%2d", &year, &month, &day, &hour, &minute);
	TDatime dt(year, month, day, 0, 0, 0);
	g->SetPoint(ipoint++, dt.Convert() - 86400, 0);	// the first data point is set to the day before the first good run
	preMonth = month;
	preDay = day;
	break;
    }

    for (; run<=endRun; run++)
    {
	if (db.getRunType(run) != "data"
	 || db.getRunFlag(run) != "good")
	    continue;

	datetime = db.getRunStartTime(run);
	sscanf(datetime.c_str(), "%4d-%2d-%2d %2d:%2d", &year, &month, &day, &hour, &minute);
	if (day != preDay || month != preMonth )
	{
	    TDatime dt(year, preMonth, preDay, 0, 0, 0);
	    g->SetPoint(ipoint++, dt.Convert(), total);
	    preMonth = month;
	    preDay = day;
	}
	total += db.getRunEventNumber(run);
    }
    // The last day
    TDatime dt(year, month, day, 0, 0, 0);
    g->SetPoint(ipoint++, dt.Convert(), total);
    g->SetPoint(ipoint++, dt.Convert(), 0); // stupid thing with the 'F' option

    cout << INFO << "Find " << total-shift << " good events between run " << startRun << " - " << endRun << endl;
    return total-shift;
}

void statistics()
{
    gROOT->SetBatch(1);
    cali_style();

    TDatime xminDate(2024, 4, 20, 0, 0, 0);
    // TDatime commission(2024, 4, 22, 0, 0, 0);
    TDatime collision(2024, 4, 26, 0, 0, 0);	// first physical collision
    TDatime firstGoodRun(2024, 6, 4, 0, 0, 0);	// first physical collision
    TDatime endDate(2024, 10, 21, 0, 0, 0);	// collision end data
    TDatime xmaxDate(2024, 10, 25, 0, 0, 0);

    const double ymin = 0;
    const double ymax = 1.5e8;
    TGraph *gPP = new TGraph();
    gPP->SetTitle(";Date;Count");
    gPP->SetMinimum(ymin);
    gPP->SetMaximum(ymax);
    gPP->GetXaxis()->SetTimeDisplay(1);
    gPP->GetXaxis()->SetTimeFormat("%m/%d");
    gPP->GetXaxis()->SetTimeOffset(0, "gmt");
    gPP->SetLineColor(kOrange);
    gPP->SetFillColor(kOrange);

    TGraph *gAuAu = new TGraph();
    gAuAu->SetLineColor(kMagenta+1);
    gAuAu->SetFillColor(kMagenta+1);

    int ppEvents = fill(gPP, cali::run24PPStartRun, cali::run24PPEndRun);
    int AuAuEvents = fill(gAuAu, cali::run24AuAuStartRun, cali::run24AuAuEndRun);

    TLegend *lgd = new TLegend(0.62, 0.7, 0.82, 0.9);

    int x = collision.Convert();
    TLine *l1 = new TLine(x, ymin, x, ymax);
    l1->SetLineWidth(2);
    l1->SetLineColor(kRed);
    TText *t1 = new TText(x, ymax/2, "First PP Collision");
    t1->SetTextSize(0.04);
    t1->SetTextColor(kRed);
    t1->SetTextAngle(90);
    t1->SetTextAlign(23);

    x = firstGoodRun.Convert();
    TLine *l2 = new TLine(x, ymin, x, ymax);
    l2->SetLineWidth(2);
    l2->SetLineColor(kRed);
    TText *t2 = new TText(x, ymax/2, "First Good CALI Run");
    t2->SetTextSize(0.04);
    t2->SetTextColor(kRed);
    t2->SetTextAngle(90);
    t2->SetTextAlign(23);

    TCanvas *c = new TCanvas("c", "c", 800, 600);
    gPP->Draw("AF");
    c->Update();
    gPP->GetXaxis()->SetLimits(xminDate.Convert(), xmaxDate.Convert());
    gAuAu->Draw("F SAME");
    lgd->AddEntry(gPP, Form("PP: %.1e", (float)ppEvents), "F");
    lgd->AddEntry(gAuAu, Form("AuAu: %.1e", (float)AuAuEvents), "F");

    l1->Draw();
    t1->Draw();
    l2->Draw();
    t2->Draw();
    lgd->Draw();

    c->SaveAs("statistics.pdf");
}

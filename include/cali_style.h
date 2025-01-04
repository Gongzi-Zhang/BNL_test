int colors[] = {kBlack, kViolet, kRed, kBlue, kGreen+3, kAzure+9, kViolet+7};
int markers[] = {20, 21, 22, 23, 33, 34, 35};

void cali_style() 
{
    // pad
    gStyle->SetPadTopMargin(0.05);
    gStyle->SetPadRightMargin(0.05);

    // ticks on up and right axis
    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);

    // axis label size
    gStyle->SetLabelSize(0.04, "X");
    gStyle->SetLabelSize(0.04, "Y");
    gStyle->SetLabelSize(0.04, "Z");

    // axis label size
    gStyle->SetTitleSize(0.05, "X");
    gStyle->SetTitleSize(0.05, "Y");
    gStyle->SetTitleSize(0.05, "Z");

    // axis title offset
    gStyle->SetTitleOffset(0.9, "X");
    gStyle->SetTitleOffset(0.9, "Y");
    gStyle->SetTitleOffset(0.9, "Z");

    // legend style
    gStyle->SetLegendBorderSize(0);
    // gStyle->SetLegendLineColor(0);
    // gStyle->SetLegendLineStyle(0);
    // gStyle->SetLegendFillStyle(0);
    gStyle->SetLegendTextSize(0.045);
}

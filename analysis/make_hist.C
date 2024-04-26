const int nBoards = 3;
const int nChannels = 64*nBoards;

void make_hist(string inFile, string outFile = "")
{
    TFile *fin = new TFile(inFile.c_str(), "read");
    if (!fin->IsOpen())
    {
	cerr << "ERROR\tCan't open input file: " << inFile << endl
	     << "Please check it" << endl;
	return;
    }
    TTree *tin = (TTree*) fin->Get("events");
    
    if (outFile.empty())
    {
	outFile = inFile;
	outFile.insert(outFile.find(".root"), "_hist");
    }
    TFile *fout = new TFile(outFile.c_str(), "update");

    map<string, TH1F*> h1;
    Long64_t LG[nChannels];
    string chName[nChannels];
    // init histograms
    for (int ch=0; ch<nChannels; ch++)
    {
	chName[ch] = Form("Ch_%d_LG", ch);
	tin->SetBranchAddress(chName[ch].c_str(), &LG[ch]);
	h1[chName[ch]] = new TH1F(chName[ch].c_str(), Form("Ch_%d_LG;ADC;Count", ch), 100, 0, 200);
    }

    for (int i=0; i<tin->GetEntries(); i++)
    {
	tin->GetEntry(i);
	for (int ch=0; ch<nChannels; ch++)
	{
	    h1[chName[ch]]->Fill(LG[ch]);
	}
    }

    fout->cd();
    for (int ch=0; ch<nChannels; ch++)
    {
	h1[chName[ch]]->Write();
    }
}

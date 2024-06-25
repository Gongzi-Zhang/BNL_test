#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <ctime>
#include "calo.h"
#include "utilities.h"
#include "makeTree.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
time_t listReader::getStartTime() {
    if (!fin.is_open())
	fin.open(listFile);

    // skip the first 9 lines
    while (li<6)
    {
	li++;
	getline(fin, line);
    }
    // get the start time
    li++;
    getline(fin, line);
    fields = split(line);
    string st = fields[8] + "-" + fields[5] + "-" + fields[6] + " " + fields[7];
    tm t{};
    istringstream ss(st);
    ss >> get_time(&t, "%Y-%b-%d %H:%M:%S");
				   //
    while (li<9)
    {
	li++;
	getline(fin, line);
    }

    time_t startTime = mktime(&t); // UTC time
    return startTime;
}

void listReader::read(const int nRequest)
{
    int bid;
    double ts;
    int ch, HG, LG;
    while(boards.size() < nRequest && getline(fin, line))
    {
	li++;
	fields.clear();
	fields = split(line);
	const int n = fields.size();
	if (6 == n)
	{
	    addBoard(board);

	    ts  = stod(fields[0])*us;
	    bid = stoi(fields[2]);
	    ch  = stoi(fields[3]);
	    LG  = stoi(fields[4]);
	    HG  = stoi(fields[5]);
	    board = new boardReadout(bid, ts);
	}
	else if (4 == n)
	{
	    bid = stoi(fields[0]);
	    ch  = stoi(fields[1]);
	    LG  = stoi(fields[2]);
	    HG  = stoi(fields[3]);
	}
	else
	{
	    cerr << ERROR << "Invalid value in line " << li << endl;
	    continue;
	}
	ch += calo::preChannels[bid];
	board->addChannel(ch, LG, HG);
    }
    
    if (boards.size() < nRequest)
    {
	// the last board
	addBoard(board);
	eof = true;
    }
    cout << INFO << nGoods << "/" << nBads << "  good/bad board entries read" << endl;
}

void listReader::addBoard(boardReadout *b)
{
    if (!b)
	return;

    int bid = b->getId();
    if (b->getnChannels() != calo::nCAENChannels[bid])
    {
	nBads++;
	cerr << WARNING << "bad board record in board " << bid << ", "
	     << b->getnChannels() << "/" << calo::nCAENChannels[bid] << " recorded; timestamps: " << b->getTS() << endl;
	return;
    }
    nGoods++;
    boards.push_back(b);
}

int listReader::getBoards(const int nRequest, vector<boardReadout*>& ret)
{
    ret.clear();
    int n =  nRequest;
    while (!eof && boards.size() < n)
	read(n - boards.size());

    if (boards.size() < n && eof)
	n = boards.size();

    vector<boardReadout*>::const_iterator first = boards.begin();
    ret = {first, first+n};
    boards.erase(first, first+n);
    return ret.size();
}



//////////////////////////////////////////////////////////////////////
void eventBuilder::addBoard(boardReadout* b)
{
    int bid = b->getId();
    boards[bid].push_back(b);
    TS[bid].push_back(b->getTS());
}

void eventBuilder::getTimeDiff()
{
    map<int, pair<double, double>> tdRange = {
	{1, {17.5*ms, 24.5*ms}},
       	{2, {36*ms, 44.5*ms}},
    };
    vector<int> ei;
    for (int ci=0; ci<calo::nCAENs; ci++)
	ei.push_back(0);

    while (ei[0] < boards[0].size())
    {
	double ts0 = TS[0][ei[0]];
	bool findEvent = true;
	for (int ci=1; ci<calo::nCAENs; ci++)
	{
	    bool findMatch = false;
	    while (ei[ci] < boards[ci].size())
	    {
		double ts = TS[ci][ei[ci]];
		if (ts > ts0)
		{
		    if (ts > ts0*1000)	// some insane values
		    {
			ei[ci]++;
			continue;
		    }
		    else
			break;
		}
		double diff = ts0 - ts;
		if (tdRange[ci].first <= diff and diff <= tdRange[ci].second)
		{
		    findMatch = true;
		    break;
		}
		ei[ci]++;
	    }

	    if (not findMatch)
	    {
		findEvent = false;
		break;
	    }
	}
	if (findEvent)
	{
	    for (int ci=0; ci<calo::nCAENs; ci++)
	    {
		timeDiff[ci] = ts0 - TS[ci][ei[ci]];
		cout << INFO << "first event at CAEN unit " << ci << ": " << TS[ci][ei[ci]]
		    << ", time difference to unit 0: " << timeDiff[ci]/us << endl;
	    }
	    break;
	}
	ei[0]++;
    }
}

void eventBuilder::build()
{
    if (not timeDiff.size())
    {
	getTimeDiff();
	if (timeDiff.size() != calo::nCAENs)
	{
	    cerr << FATAL << "can't find out the time difference" << endl;
	    cout << INFO << "here are the first 10 records" << endl;
	    for (int i=0; i<10; i++)
	    {
		for (int ci=0; ci<calo::nCAENs; ci++)
		{
		    if (i < boards[ci].size())
			cout << "\t" << ci << "\t" << boards[ci][i]->getTS();
		}
		cout << endl;
	    }
	    exit(4);
	}
    }

    for (int ci=1; ci<calo::nCAENs; ci++)
    {
	for (auto& ts : TS[ci])
	    ts += timeDiff[ci];
    }

    vector<int> ei;
    map<int, boardReadout*> eventCandidate;
    bool hasCandidate = true;
    for (int ci=0; ci<calo::nCAENs; ci++)
    {
	ei.push_back(0);
	hasCandidate &= (boards[ci].size() > 0);
	eventCandidate[ci] = NULL;
    }

    double ts;
    event* evt = NULL;
    while (hasCandidate)
    {
	double ts0 = TS[0][ei[0]];
	for (int ci=1; ci<calo::nCAENs; ci++)
	{
	    ts = TS[ci][ei[ci]];
	    if (ts < ts0)
		ts0 = ts;
	}

	bool findEvent = true;
	for (int ci=0; ci<calo::nCAENs; ci++)
	{
	    ts = TS[ci][ei[ci]];
	    while (ts > ts0*1000)
	    {
		delete boards[ci][ei[ci]];
		ei[ci]++;
		if (ei[ci] == boards[ci].size())
		{
		    ei[ci]--;
		    break;
		}
		else
		    ts = TS[ci][ei[ci]];
	    }
	    if ((ts - ts0) < 10*us)
	    {
		eventCandidate[ci] = boards[ci][ei[ci]];
	    }
	    else
	    {
		findEvent = false;
	    }
	}
	nEvents++;

	if (findEvent)
	{
	    nGoods++;
	    evt = new event();
	    evt->TS = ts0/s;
	    for (int ci=0; ci<calo::nCAENs; ci++)
	    {
		for (auto const ele : boards[ci][ei[ci]]->getLG())
		    (evt->LG).insert(ele);
		for (auto const ele : boards[ci][ei[ci]]->getHG())
		    (evt->HG).insert(ele);
	    }
	    events.push_back(evt);
	}
	else
	{
	    nBads++;
	    cerr << WARNING << "bad event " << nBads;
	    for (int ci=0; ci<calo::nCAENs; ci++)
		cerr << "\t" << ci << ": " << (to_string) (boards[ci][ei[ci]]->getTS());
	    cerr << endl;
	}

	for (int ci=0; ci<calo::nCAENs; ci++)
	{
	    if (eventCandidate[ci])
	    {
		delete eventCandidate[ci];
		eventCandidate[ci] = NULL;
		boards[ci][ei[ci]] = NULL;
		ei[ci]++;
		hasCandidate &= (ei[ci] < boards[ci].size());
	    }
	}
    }

    // remove the used records
    for (int ci=0; ci<calo::nCAENs; ci++)
    {
	boards[ci].erase(boards[ci].begin(), boards[ci].begin()+ei[ci]);
	TS[ci].erase(TS[ci].begin(), TS[ci].begin()+ei[ci]);
    }
    // recover the TS
    for (int ci=1; ci<calo::nCAENs; ci++)
    {
	for (auto& ts : TS[ci])
	    ts -= timeDiff[ci];
    }

}

int eventBuilder::getEvents(const int nRequest, vector<event*>& ret)
{
    ret.clear();
    if (reader->isEof() && 0 == events.size())
	return 0;

    int n = nRequest;
    while (events.size() < n && !reader->isEof())
    {
	vector<boardReadout*> vb;
	reader->getBoards(calo::nCAENs*(n-events.size()), vb);
	for (boardReadout* b : vb)
	    addBoard(b);
	build();
    }

    if (reader->isEof())
    {
	int left = 0;
	for (int ci=0; ci<calo::nCAENs; ci++)
	{
	    int n = boards[ci].size();
	    if (left < n)
		left = n;
	    for (int i=0; i<n; i++)
		delete boards[ci][i];
	    boards[ci].clear();
	}
	nEvents += left;
    }

    if (events.size() < n)
    {
	n = events.size();
	cout << INFO << nGoods << "/" << nEvents << " built." << endl;
    }

    vector<event*>::const_iterator first = events.begin();
    ret = {first, first+n};
    events.erase(first, first+n);
    return ret.size();
}




//////////////////////////////////////////////////////////////////////
void treeMaker::init()
{
    fout = new TFile(ofName.c_str(), "recreate");
    traw = new TTree("raw", "raw ADC values");	 // owned by fout
    tcor = new TTree("cor", "corrected ADC values");

    traw->Branch("TS", &TS);
    for (int ch=0; ch<calo::nChannels; ch++)
    {
	rawADC[ch] = {0, 0};
	corADC[ch] = {0, 0};
	traw->Branch(Form("ch_%d", ch), &rawADC[ch], "LG/I:HG/I");
	tcor->Branch(Form("ch_%d", ch), &corADC[ch], "LG/F:HG/F");
    }

    tcor->Branch("rate",  &rate);
    tcor->Branch("mul",   &mul,  "LG/I:HG/I");
    tcor->Branch("mul1",  &mul1, "LG/I:HG/I");
    tcor->Branch("mul2",  &mul2, "LG/I:HG/I");
}

void treeMaker::fill()
{
    if (!builder)
    {
	cerr << ERROR << "No event builder specified." << endl;
	return;
    }

    vector<event*> ve;
    while(builder->getEvents(10000, ve) != 0)
    {
	for (auto &evt : ve)
	{
	    nEvents++;
	    mul = {0, 0};
	    mul1 = {0, 0};
	    mul2 = {0, 0};

	    TS = evt->TS + st;
	    rate = 1/(TS - preTS);
	    preTS = TS;
	    for (int ch=0; ch<calo::nChannels; ch++)
	    {
		rawADC[ch].first = evt->LG[ch];
		rawADC[ch].second = evt->HG[ch];

		corADC[ch].first = rawADC[ch].first - ped["LG"][ch].mean;
		corADC[ch].second = rawADC[ch].second - ped["HG"][ch].mean;
		
		// LG
		if (corADC[ch].first < ped["LG"][ch].rms)
		    corADC[ch].first = 0;
		else
		{
		    mul.first++;
		    if (corADC[ch].first > 3*ped["LG"][ch].rms)
			mul1.first++;
		    if (corADC[ch].first > 5*ped["LG"][ch].rms)
			mul2.first++;

		}
		// HG
		if (corADC[ch].second < ped["HG"][ch].rms)
		    corADC[ch].second = 0;
		else
		{
		    mul.second++;
		    if (corADC[ch].second > 3*ped["HG"][ch].rms)
			mul1.second++;
		    if (corADC[ch].second > 5*ped["HG"][ch].rms)
			mul2.second++;
		}
	    }
	    traw->Fill();
	    tcor->Fill();
	    delete evt;
	}
	cout << INFO << nEvents << " events filled" << endl;
    }
}

void treeMaker::write()
{
    traw->Write();
    tcor->Write();
    fout->Close();

    delete fout;
}



//////////////////////////////////////////////////////////////////////
void cosmicTreeMaker::init()
{
    fout = new TFile(ofName.c_str(), "recreate");
    for (int ci=0; ci<calo::nCAENs; ci++)
    {
	preTS.push_back(1e32);
	nEvents.push_back(0);

	fout->cd();
	traw.push_back(new TTree(Form("raw_CAEN%d", ci), Form("CAEN%d: raw ADC values", ci)));
	tcor.push_back(new TTree(Form("cor_CAEN%d", ci), Form("CAEN%d: corrected ADC values", ci)));

	traw[ci]->Branch("TS", &TS);
	for (int ch=calo::preChannels[ci]; ch<calo::preChannels[ci] + calo::nCAENChannels[ci]; ch++)
	{
	    rawADC[ch] = {0, 0};
	    corADC[ch] = {0, 0};
	    traw[ci]->Branch(Form("ch_%d", ch), &rawADC[ch], "LG/I:HG/I");
	    tcor[ci]->Branch(Form("ch_%d", ch), &corADC[ch], "LG/F:HG/F");
	}

	tcor[ci]->Branch("rate",  &rate);
	tcor[ci]->Branch("mul",   &mul,  "LG/I:HG/I");
	tcor[ci]->Branch("mul1",  &mul1, "LG/I:HG/I");
	tcor[ci]->Branch("mul2",  &mul2, "LG/I:HG/I");
    }
}

void cosmicTreeMaker::addBoard(boardReadout *b)
{
    int bid = b->getId();
    boards[bid].push_back(b);
}

void cosmicTreeMaker::fill()
{
    while (!reader->isEof())
    {
	vector<boardReadout*> vb;
	reader->getBoards(calo::nCAENs*10000, vb);
	for (boardReadout* b : vb)
	    addBoard(b);

	for (int ci=0; ci<calo::nCAENs; ci++)
	    fill(ci);
    }
}

void cosmicTreeMaker::fill(const int ci)
{
    if (ci < 0 || ci >= calo::nCAENs)
    {
	cerr << ERROR << "invalid CAEN unit index: " << ci << ". Allowed range: 0-" << calo::nCAENs << endl;
	return;
    }

    boardReadout* b;
    map<int, int> LG, HG;
    for (int ei=0; ei<boards[ci].size(); ei++)
    {
	b = boards[ci][ei];
	LG = b->getLG();
	HG = b->getHG();


	mul = {0, 0};
	mul1 = {0, 0};
	mul2 = {0, 0};

	TS = b->getTS() + st;
	rate = 1/(TS - preTS[ci]);
	preTS[ci] = TS;
	for (int ch=calo::preChannels[ci]; ch<calo::preChannels[ci] + calo::nCAENChannels[ci]; ch++)
	{
	    rawADC[ch].first = LG[ch];
	    rawADC[ch].second = HG[ch];

	    corADC[ch].first = rawADC[ch].first - ped["LG"][ch].mean;
	    corADC[ch].second = rawADC[ch].second - ped["HG"][ch].mean;
	    
	    // LG
	    if (corADC[ch].first < ped["LG"][ch].rms)
		corADC[ch].first = 0;
	    else
	    {
		mul.first++;
		if (corADC[ch].first > 3*ped["LG"][ch].rms)
		    mul1.first++;
		if (corADC[ch].first > 5*ped["LG"][ch].rms)
		    mul2.first++;

	    }
	    // HG
	    if (corADC[ch].second < ped["HG"][ch].rms)
		corADC[ch].second = 0;
	    else
	    {
		mul.second++;
		if (corADC[ch].second > 3*ped["HG"][ch].rms)
		    mul1.second++;
		if (corADC[ch].second > 5*ped["HG"][ch].rms)
		    mul2.second++;
	    }
	}
	traw[ci]->Fill();
	tcor[ci]->Fill();

	delete b;
	b = NULL;
    }

    nEvents[ci] += boards[ci].size();
    boards[ci].erase(boards[ci].begin(), boards[ci].end());

    cout << INFO << nEvents[ci] << " events in CAEN " << ci << " filled" << endl;
}

void cosmicTreeMaker::write()
{
    for (int ci=0; ci<calo::nCAENs; ci++)
    {
	traw[ci]->Write();
	tcor[ci]->Write();
    }
    fout->Close();
    delete fout;
}

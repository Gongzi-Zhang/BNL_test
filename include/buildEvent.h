#ifndef _BUILD_EVENT_
#define _BUILD_EVENT_

#include <vector>
#include <map>
#include "cali.h"

using namespace std;

class boardReadout {
  public:
    boardReadout(const int bid, const double ts) { id = bid; TS =ts; }
    ~boardReadout() {}
    int getId() const { return id; }
    int getnChannels() const { return nChannels; }
    double getTS() const { return TS; }
    map<int, int> getLG() { return LG; }
    map<int, int> getHG() { return HG; }
    void addChannel(int ch, int vLG, int vHG)
    {
	LG[ch] = vLG;
	HG[ch] = vHG;
	nChannels++;
    }
    void clear()
    {
	LG.clear();
	HG.clear();
	nChannels = 0;
    }

  private:
    int id = -1;
    double TS = -1;
    int nChannels = 0;
    map<int, int> LG;
    map<int, int> HG;
};

struct event {
    double TS;
    map<int, int> LG;
    map<int, int> HG;
};

class buildEvent {
  public:
    buildEvent() {}
    ~buildEvent() {}
    void addBoard(boardReadout *b);
    void getTimeDiff();
    void build();
    void setEof() { eof = true; }
    int  getnEvents() { return events.size(); }
    vector<event*> getEvents(const int n);

  private:
    int nEvents = 0;
    int nGoods = 0;
    int nBads = 0;
    bool eof = false;
    map<int, double> timeDiff;
    map<int, vector<boardReadout*>> boards;
    map<int, vector<double>> TS;
    vector<event*> events;
};

void buildEvent::addBoard(boardReadout *b)
{
    int bid = b->getId();
    if (b->getnChannels() != cali::nCAENChannels && b->getnChannels() != cali::nChannels)
    {
	cerr << WARNING << "bad board record in board " << bid << ", "
	     << b->getnChannels() << "/" << cali::nCAENChannels << "recorded; timestamps: " << b->getTS() << endl;
	return;
    }
    boards[bid].push_back(b);
    TS[bid].push_back(b->getTS());

    bool batch = true;
    for (int ci=0; ci<cali::nCAENs; ci++)
    {
	if (boards[ci].size() < 50000)
	    batch = false;
    }
    if (batch)
	build();
}

void buildEvent::getTimeDiff()
{
    map<int, pair<double, double>> tdRange = {
	{1, {17.5*ms, 24.5*ms}},
       	{2, {36*ms, 44.5*ms}},
    };
    int ei[cali::CAENMax] = {0};

    while (ei[0] < boards[0].size())
    {
	double ts0 = TS[0][ei[0]];
	bool findEvent = true;
	for (int ci=1; ci<cali::nCAENs; ci++)
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
	    for (int ci=0; ci<cali::nCAENs; ci++)
	    {
		timeDiff[ci] = ts0 - TS[ci][ei[ci]];
		cout << INFO << "first event at CAEN unit " << ci << ": " << TS[ci][ei[ci]]
		     << ", time difference to unit 0: " << timeDiff[ci] << endl;
	    }
	    break;
	}
        ei[0]++;
    }
}

void buildEvent::build()
{
    if (not timeDiff.size())
    {
	getTimeDiff();
	if (timeDiff.size() != cali::nCAENs)
	{
	    cerr << FATAL << "can't find out the time difference" << endl;
	    cout << INFO << "here are the first 10 records" << endl;
	    for (int i=0; i<10; i++)
	    {
		for (int ci=0; ci<cali::nCAENs; ci++)
		{
		    if (i < boards[ci].size())
			cout << "\t" << ci << "\t" << boards[ci][i]->getTS();
		}
		cout << endl;
	    }
	    exit(4);
	}
    }

    for (int ci=1; ci<cali::nCAENs; ci++)
    {
	for (auto& ts : TS[ci])
	    ts += timeDiff[ci];
    }

    int ei[cali::CAENMax] = {0};
    map<int, boardReadout*> eventCandidate;
    bool hasCandidate = true;
    for (int ci=0; ci<cali::nCAENs; ci++)
    {
	hasCandidate &= (boards[ci].size() > 0);
	eventCandidate[ci] = NULL;
    }

    double ts;
    event* evt = NULL;
    while (hasCandidate)
    {
	double ts0 = TS[0][ei[0]];
	for (int ci=1; ci<cali::nCAENs; ci++)
	{
	    ts = TS[ci][ei[ci]];
	    if (ts < ts0)
		ts0 = ts;
	}

        bool findEvent = true;
	for (int ci=0; ci<cali::nCAENs; ci++)
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
	    for (int ci=0; ci<cali::nCAENs; ci++)
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
	    for (int ci=0; ci<cali::nCAENs; ci++)
		cerr << "\t" << ci << ": " << (to_string) (boards[ci][ei[ci]]->getTS());
	    cerr << endl;
	}

	for (int ci=0; ci<cali::nCAENs; ci++)
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
    for (int ci=0; ci<cali::nCAENs; ci++)
    {
	boards[ci].erase(boards[ci].begin(), boards[ci].begin()+ei[ci]);
	TS[ci].erase(TS[ci].begin(), TS[ci].begin()+ei[ci]);
    }
    // recover the TS
    for (int ci=1; ci<cali::nCAENs; ci++)
    {
	for (auto& ts : TS[ci])
	    ts -= timeDiff[ci];
    }

    if (eof)
    {
        int left = boards[0].size();
	for (int ci=1; ci<cali::nCAENs; ci++)
	{
	    if (left < boards[ci].size())
		left = boards[ci].size();
	}
	nEvents += left;
    }
    cout << INFO << nGoods << "/" << nEvents << " built." << endl;
}

vector<event*> buildEvent::getEvents(const int nRequest = -1)
{
    int n = nRequest;
    if (-1 == n)
	n = events.size();
    else if (n > events.size())
    {
	cerr << WARNING << n << " events requested, only " << events.size() 
	     << " events available" << endl;
	n = events.size();
    }

    vector<event*>::const_iterator first = events.begin();
    vector<event*> ret(first, first+n);
    events.erase(first, first+n);
    return ret;
}
#endif

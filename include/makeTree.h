#ifndef _MAKE_TREE_
#define _MAKE_TREE_

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include "TFile.h"
#include "TTree.h"
#include "analysis.h"

using namespace std;

vector<string> split(string in)
{
    vector<string> out;
    istringstream ss(in);
    string word;
    while (ss >> word)
	out.push_back(word);
    return out;
}

class boardReadout {
  public:
    boardReadout(const int bid, const double ts) { id = bid; TS =ts; }
    ~boardReadout() {}
    int getId() { return id; }
    int getnChannels() { return nChannels; }
    double getTS() { return TS; }
    map<int, int> getLG() { return LG; }
    map<int, int> getHG() { return HG; }
    void addChannel(int ch, int vLG, int vHG)
    {
	LG[ch] = vLG;
	HG[ch] = vHG;
	nChannels++;
    }

  private:
    int id = -1;
    double TS = -1;
    int nChannels = 0;
    map<int, int> LG;
    map<int, int> HG;
};

class listReader {
  public:
    listReader(string f) { listFile = f; }
    ~listReader() {}
    bool isEof() { return eof; }
    time_t getStartTime();
    void read(const int n);
    void addBoard(boardReadout *b);
    int  getBoards(const int n, vector<boardReadout*>&);

  private:
    ifstream fin;
    string listFile;
    string line;
    vector<string> fields;
    int li = 0;
    bool eof = false;
    boardReadout* board = 0;
    vector<boardReadout*> boards;
    int nGoods = 0;
    int nBads = 0;
};

struct event {
    double TS;
    map<int, int> LG;
    map<int, int> HG;
};

class eventBuilder {
  public:
    eventBuilder(listReader* r) { reader = r; }
    ~eventBuilder() {}
    void addBoard(boardReadout*);
    void getTimeDiff();
    void build();
    int  getnEvents() { return events.size(); }
    int  getEvents(const int n, vector<event*>& ret);

  private:
    listReader* reader;
    int nEvents = 0;
    int nGoods = 0;
    int nBads = 0;
    map<int, double> timeDiff;
    map<int, vector<boardReadout*>> boards;
    map<int, vector<double>> TS;
    vector<event*> events;
};

class treeMaker {
  public:
    treeMaker(eventBuilder* b) { builder = b; }
    ~treeMaker() {}
    void setStartTime(time_t t) { st = t; }
    void setOfName(string n) { ofName = n; }
    void init();
    void fill();
    void write();

  private:
    eventBuilder *builder = NULL;

    int nEvents = 0;
    time_t st;
    double TS, preTS = 1e32;
    float rate;

    map<int, pair<int, int>> rawADC;

    string ofName;
    TFile *fout = NULL;
    TTree *traw = NULL;
};

class cosmicTreeMaker {
  public:
    cosmicTreeMaker(listReader* r) { reader = r; }
    ~cosmicTreeMaker() {}
    void setStartTime(time_t t) { st = t; }
    void setOfName(string n) { ofName = n; }
    void addBoard(boardReadout*);
    void init();
    void fill();
    void fill(const int ci);
    void write();

  private:
    listReader* reader;
    ped_t ped;
    string ofName;
    TFile *fout = NULL;

    vector<int> nEvents;
    time_t st;
    double TS;
    vector<double> preTS;
    float rate;

    map<int, vector<boardReadout*>> boards;
    map<int, pair<int, int>> rawADC;

    vector<TTree *> traw;
};
#endif

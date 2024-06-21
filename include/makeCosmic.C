#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <ctime>
#include "TFile.h"
#include "TTree.h"
#include "cali.h"
#include "utilities.h"
#include "db.h"
#include "analysis.h"
#include "makeCosmic.h"

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

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
	cerr << FATAL << "no run number provided" << endl;
	exit(4);
    }

    const int run = atoi(argv[1]);
    cali::setRun(run);
    caliDB db;
    string runType = db.getRunType(run);
    if (runType != "cosmic")
    {
	cout << WARNING << "not a data/cosmic run: " << run << endl;
	exit(1);
    }
    string outName = cali::CALIROOT;
    outName += "/data/Run" + to_string(run) + ".root"; 

    const int pedRun = db.getPedRun(run);
    pedestal ped;
    if (!getPedestal(pedRun, ped))
    {
	cerr << FATAL << "unable to read pedestal file" << endl;
	exit(2);
    }

    cout << INFO << "processing run: " << run << endl;

    string listFile = Form("%s/data/Run%d_list.txt", cali::CALIROOT, run);
    if (!fileExists(listFile.c_str()))
    {
	cerr << FATAL << "no list file for run " << run << endl;
	exit(4);
    }

    ifstream fin(listFile);
    string line;
    vector<string> fields;
    int li = 0;
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
    time_t startTime = mktime(&t) - 5*3600; // FiXME why it is 5 hours here?

    while (li<9)
    {
	li++;
	getline(fin, line);
    }

    int bid;
    double ts;
    int ch, LG, HG;
    boardReadout *board = NULL;
    makeCosmic *maker = new makeCosmic();
    maker->setStartTime(startTime);
    maker->setPed(ped);
    maker->setOfName(outName);
    maker->init();

    while (getline(fin, line))
    {
	li++;
	fields.clear();
	fields = split(line);
	const int n = fields.size();
	if (6 == n)
	{
	    if (board)
		maker->addBoard(board);

	    ts  = stod(fields[0]);
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
	ch += cali::nCAENChannels*bid;
	board->addChannel(ch, LG, HG);
    }

    // the last board record
    maker->addBoard(board);
    maker->fill();
    maker->write();

    fin.close();
    delete maker;
}

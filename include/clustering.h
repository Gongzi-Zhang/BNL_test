#ifndef __CLUSTERING__
#define __CLUSTERING__

/* showershape analysis */
#include <iostream>

#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include "utilities.h"
#include "caliType.h"

using namespace std;

class clustering {
  public:
    void setInput(string f)  { inFileName = f; }
    void setOutput(string f) { outFileName = f; }
    void setNeighborX(double x) { neighbor_x = x; }
    void setNeighborY(double y) { neighbor_y = y; }
    void setNeighborZ(double z) { neighbor_z = z; }
    void setMinClusterCenterE(double e) { minClusterCenterE = e; }
    void setMinClusterHitE(double e) { minClusterHitE = e; }
    void setMinClusterE(double e) { minClusterE = e; }
    void setMinClusterNhits(size_t n) { minClusterNhits = n; }
    bool isNeighbor(caliHit &h1, caliHit &h2) { return (abs(h1.x - h2.x) <= neighbor_x) && (abs(h1.y - h2.y) <= neighbor_y) && (abs(h1.z - h2.z) <= neighbor_z); }

    void init();
    void process();
    void bfsGroup(vector<size_t> &group, const size_t idx);
    void findClusters();
	
  private:
    std::string inFileName;
    std::string outFileName;

    double neighbor_x, neighbor_y, neighbor_z;
    TClonesArray *hits = new TClonesArray("caliHit");
    vector<bool> visits;

    vector<vector<size_t>> groups;

    // cluster configurations
    double minClusterCenterE;
    double minClusterHitE;
    double minClusterE;
    size_t minClusterNhits;
};

void clustering::bfsGroup(vector<size_t> &group, const size_t idx)
{
    visits[idx] = true;

    group.push_back(idx);
    size_t count = 0;
    while (count < group.size())
    {
	size_t idx1 = group[count];
	caliHit *h1 = (caliHit*) hits->At(idx1);
	for (size_t idx2 = 0; idx2 < hits->GetEntries(); idx2++)
	{
	    caliHit *h2 = (caliHit*) hits->At(idx2);
	    if (!visits[idx2] && isNeighbor(*h1, *h2))
	    {
		visits[idx2] = true;
		if (h1->e < minClusterHitE)
		    continue;
		group.push_back(idx2);
	    }
	}
	count++;
    }
}

void clustering::findClusters()
{
    for (size_t i=0; i<hits->GetEntries(); i++)
    {
	caliHit *hit = (caliHit*) hits->At(i);
	if (visits[i] || hit->e < minClusterCenterE)
	    continue;

	groups.emplace_back();
	bfsGroup(groups.back(), i);
    }
}

void clustering::init()
{
    if (minClusterCenterE < minClusterHitE)
    {
	cerr << ERROR << "minClusterCenterE must be larger than minClusterHitE: " << minClusterCenterE << " vs " << minClusterHitE << endl;
	return;
    }
    if (minClusterE < minClusterCenterE)
    {
	cerr << ERROR << "minClusterE must be larger than minClusterCenterE: " << minClusterE << " vs " << minClusterCenterE << endl;
	return;
    }
}

void clustering::process()
{
    // input
    TFile *fio = new TFile(inFileName.c_str(), "update");
    TTree *tio = (TTree*) fio->Get("events");
    tio->SetBranchAddress("CALIHits", &hits);

    TClonesArray *clus = new TClonesArray("caliCluster");
    tio->Branch("CALIClusters", &clus);

    const size_t N = tio->GetEntries();
    for (size_t ei=0; ei<N; ei++)
    {
	if (ei % 10000 == 0)
	    cout << INFO << "reading event " << ei << endl;
	hits->Clear();
	tio->GetEntry(ei);
	visits.resize(hits->GetEntries(), false);

	groups.clear();
	findClusters();

	// form clusters
	clus->Clear();
	size_t nc = 0;
	for (const auto &group : groups)
	{
	    if (group.size() < minClusterNhits)
		continue;
	    float energy = 0;
	    for (size_t idx : group)
	    {
		energy += ((caliHit*)hits->At(idx))->e;
	    }
	    if (energy < minClusterE)
		continue;

	    new((*clus)[nc]) caliCluster(group.size(), energy);
	    nc++;
	}
	tio->Fill();
    }

    fio->Write();
    fio->Close();
}

#endif

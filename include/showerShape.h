#ifndef _SHOWERSHAPE_
#define _SHOWERSHAPE_

/* showershape analysis */
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include "caliType.h"

using namespace std;

class showerShape {
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

void showerShape::bfsGroup(vector<size_t> &group, const size_t idx)
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

void showerShape::findClusters()
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

void showerShape::init()
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

void showerShape::process()
{
    // input
    TFile *fin = new TFile(inFileName.c_str(), "update");
    TTree *tin = (TTree*) fin->Get("hit");
    tin->SetBranchAddress("CALIHits", &hits);

    const char* toutName = "cluster";
    if (fin->Get(toutName))
	fin->Delete(toutName);
    TTree *tout = new TTree(toutName, "reconstructed clusters");

    TClonesArray *clus = new TClonesArray("caliCluster");
    tout->Branch("CALIClusters", &clus);

    const size_t N = tin->GetEntries();
    for (size_t ei=0; ei<N; ei++)
    {
	if (ei % 10000 == 0)
	    cout << INFO << "reading event " << ei << endl;
	hits->Clear();
	tin->GetEntry(ei);
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
	tout->Fill();
    }

    fin->Write();
    fin->Close();
}

#endif

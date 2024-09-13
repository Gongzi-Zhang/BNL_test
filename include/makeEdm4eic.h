#ifndef __MAKEEDM$EIC__
#define __MAKEEDM$EIC__

#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TLeaf.h"

#include "edm4eic/CalorimeterHit.h"
#include "edm4eic/CalorimeterHitCollection.h"
#include "podio/ROOTWriter.h"
#include "podio/Frame.h"

#include "utilities.h"
#include "cali.h"
#include "makeEdm4eic.h"

using namespace std;

class makeEdm4eic {
  public:
    makeEdm4eic(string in, string out) { inFileName = in; outFileName = out; }
    ~makeEdm4eic() {}
    void make();

  private:
    std::string inFileName;
    std::string outFileName;
};

void makeEdm4eic::make()
{
    TFile *fin = new TFile(inFileName.c_str(), "read");
    if (!fin->IsOpen())
    {
	cerr << ERROR << "Can't open root file: " << inFileName << endl;
	exit(1);
    }
    TTree *tin = (TTree *) fin->Get("mip");
    if (!tin)
    {
	cerr << ERROR << "Can't read the mip tree" << endl;
	fin->Close();
	exit(2);
    }

    cali::sipmXY pos[cali::channelMax];
    int layerNumber[cali::channelMax];
    float mip[cali::channelMax];
    for (int ch=0; ch<calo::nChannels; ch++)
    {
	layerNumber[ch] = cali::getSipm(ch).layer;
        pos[ch] = cali::getSipmXY(ch);
	tin->GetBranch(Form("ch_%d", ch))->GetLeaf("LG")->SetAddress(&mip[ch]);
    }

    int cellID;
    float energy;
    edm4hep::Vector3f position(0, 0, 0);
    edm4hep::Vector3f dimension(0, 0, 0);
    edm4hep::Vector3f local_position(0, 0, 0);
    int sid = 0;
    int lid = 0;
    int slice = 0;

    podio::ROOTWriter writer(outFileName.c_str());
    for (int ei=0; ei<tin->GetEntries(); ei++)
    {
	if (ei % 100000 == 0)
	    cout << INFO << "reading " << ei << " events" << endl;
	auto hits = std::make_unique<edm4eic::CalorimeterHitCollection>();

	tin->GetEntry(ei);
	for (int ch=0; ch<calo::nChannels; ch++)
	{
	    if (0 == mip[ch])
		continue;
	    sid = (layerNumber[ch] < 4) ? 0 : 1;
	    if (0 == sid)
		lid = layerNumber[ch];
	    else
		lid = layerNumber[ch] - 4*4;
	    cellID = (lid << 8) + sid;
	    energy = mip[ch];
	    local_position = (pos[ch].x, pos[ch].y, layerNumber[ch]);
	    position = (pos[ch].x, pos[ch].y, layerNumber[ch]);
	    hits->create(
		cellID,
		energy,
		0,	    // 
		0,          // time
		0,
		position,
		dimension,
		sid,
		lid,
		local_position);
	}
	podio::Frame frame;
	frame.put(std::move(hits), "CALIHits");
	writer.writeFrame(frame, "events");
    }
    cout << INFO << tin->GetEntries() << " events processed." << endl;
    delete tin;
    fin->Close();

    writer.finish();
}

#endif

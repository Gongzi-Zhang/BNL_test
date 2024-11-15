#ifndef __CALI__
#define __CALI__

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <cassert>
#include "calo.h"
#include "utilities.h"

using namespace std;
namespace cali {
    const char* CALIROOT = (assert(getenv("CALIROOT") != NULL), true)
	? getenv("CALIROOT")
	: ".";
    const char* backupDir = "/media/arratialab/CALI/BNL_test/";

    /* CAEN:	  CAEN unit
     * channel:   channel count: 0-192
     * layer:	  sampling layers
     * board:	  PCB count: 0-44
     * quadrant:  quadrant count in a layer: 0-3
     * sipm:      SiPM count in a PCB: 0-6
     */

	  int run = -1;
    	  int nCAENs = 3;
    const int CAENMax = 5;
    const int nCAENChannels = 64;
	  int nChannels = nCAENs*nCAENChannels;
          int nLayers = 11;
    const int layerMax = 20;
	  int nHexLayers = 4;
	  int nSqaLayers = 9;
    const int nLayerBoards = 4;
          int nHexBoards = nHexLayers*nLayerBoards;
          int nSqaBoards = nSqaLayers*nLayerBoards;
	  int nBoards = nHexBoards + nSqaBoards;
    const int boardMax = 80;
    const int nHexBoardChannels = 7;
    const int nSqaBoardChannels = 4;
          int nHexChannels = nHexBoards*nHexBoardChannels;
          int nSqaChannels = nSqaBoards*nSqaBoardChannels;

    const int nEightLayerBoards = 8*nLayerBoards;
    const int nNineLayerBoards  = 9*nLayerBoards;
    const int channelMax = 300;

    const double gapX = 0*mm;
    const double gapY = 2.54*mm;	// 0.1 in
    const double pcbX = 131.92*mm;	// 177.64 - 45.72
    const double pcbY = 97.99*mm;	// 130.91 - 32.92
    const double layerZ = 27.1526*mm;
    const double X = 70*cm;
    const double Y = 0;
    const double Z = 7*m;


    void setRun(const int r)
    {
	run = r;
	if (run < 3)
	{
	    nCAENs = 1;
	    nChannels = 56;
	    nHexLayers = 2;
	    nHexBoards = 8;
	    nHexChannels = 56;
	    nSqaLayers = 0;
	    nSqaBoards = 0;
	    nSqaChannels = 0;
	    calo::setnCAENChannels({56});
	}
	else if (run < 4)
	{
	    nCAENs = 3;
	    nChannels = 176;
	    nHexLayers = 4;
	    nHexBoards = 16;
	    nHexChannels = 112;
	    nSqaLayers = 4;
	    nSqaBoards = 16;
	    nSqaChannels = 64;
	    calo::setnCAENChannels({64, 64, 48});
	}
	else
	{
            nCAENs = 3;
	    nChannels = 192;
	    nHexLayers = 4;
	    nHexBoards = 16;
	    nHexChannels = 112;
	    nSqaLayers = 6;
	    nSqaBoards = 20;
	    nSqaChannels = 80;
	    calo::setnCAENChannels({64, 64, 64});
	}
    }

    typedef struct {
	int layer;	
	int quadrant;	
	int sipm;
    } SiPM;

    struct sipmXY{
	double x, y;
	
	inline void   operator=(sipmXY a)   { x=a.x; y=a.y; }
	inline sipmXY operator-()	    { return {-x, -y}; }
	inline sipmXY operator-(sipmXY a)   { return {x-a.x, y-a.y}; }
	inline sipmXY operator+(sipmXY a)   { return {x+a.x, y+a.y}; }
	inline bool   operator==(sipmXY a)  { return (x == a.x) && (y == a.y); }
    };

    int boardLabel[][nLayerBoards] = {
    // top right, top left, bottom left, bottom right
	{ 1,  2,  3,  4},   // hexagon
	{ 5,  6,  7,  8},   // hexagon
	{10,  9, 11, 12},   // hexagon
	{26, 13, 25, 28},   // hexagon
	{30, 41, 14, 37},   // transition: top 2 square, bottom 2 hexagon
	{38, 54, 21, 18},   // square
	{44, 22, 58, 47},   // square
	{50, 57, 33, 31},   // square
	{ 0, 15, 46, 24},   // square, the bottom right board number is unknown, guess it to be 24
	{ 0, 20, 45, 36},   // square
	{42, 49, 23, 16},   // square
	{17, 52, 19, 51},   // square
	{55, 43, 32, 39},   // square
	{34, 56, 35, 59},   // sauqre
	{ 0,  0,  0, 29},   // sauqre
    };

    sipmXY pcbAnchor[] = {
	{-gapX/2, gapY/2 + pcbY},
	{ gapX/2, gapY/2},
	{ gapX/2, -(gapY/2 + pcbY)},
	{-gapX/2, -gapY/2},
    };
					    
    sipmXY hexBoardSipmXY[nHexBoardChannels] = {
	{50.01*mm, 80.90*mm},
	{77.64*mm, 64.95*mm},
	{22.37*mm, 64.95*mm},
	{50.01*mm, 48.99*mm},
	{77.64*mm, 33.04*mm},
	{22.37*mm, 33.04*mm},
	{50.01*mm, 17.08*mm},
    };
    sipmXY sqaBoardSipmXY[nSqaBoardChannels] = {
	{73.9*mm, 72.89*mm},
	{26.1*mm, 72.89*mm},
	{73.9*mm, 25.09*mm},
	{26.1*mm, 25.09*mm},
    };
    sipmXY hexBoardSipmXY_topdown[nHexBoardChannels] = {
	{50.01*mm, 80.90*mm},
	{22.37*mm, 64.95*mm},
	{77.64*mm, 64.95*mm},
	{50.01*mm, 48.99*mm},
	{77.64*mm, 33.04*mm},
	{22.37*mm, 33.04*mm},
	{50.01*mm, 17.08*mm},
    };
    sipmXY sqaBoardSipmXY_topdown[nSqaBoardChannels] = {
	{26.1*mm, 72.89*mm},
	{73.9*mm, 72.89*mm},
	{26.1*mm, 25.09*mm},
	{73.9*mm, 25.09*mm},
    };

    SiPM getSipm(const int ch)
    {
	if (ch < 0 || ch >= nChannels)
	{
	    cerr << ERROR << "Invalid channel number:" << ch << endl;
	    cout << INFO << "Allowed channel range: 0 - " << nChannels - 1 << endl;
	    return {-1, -1, -1};
	}

	SiPM re;
	int restCh = 0;
	int board = -1;
	int sipm = -1;
	if (ch < nHexChannels)
	{
	    board = ch / nHexBoardChannels;
	    sipm = ch % nHexBoardChannels;
	}
	else 
	{
	    restCh = ch - nHexChannels;
	    board = nHexBoards + restCh / nSqaBoardChannels;
	    sipm = restCh % nSqaBoardChannels;

	    if (board >= nEightLayerBoards)
		board += 1;
	    if (board >= nNineLayerBoards)
		board += 1;

            // special cases
	    if (run < 265) 
	    {	
		if (board == 37)
		    board = 40;
	    }
	}

	re.layer = board / nLayerBoards;
	re.quadrant = board % nLayerBoards;
	re.sipm = sipm;

	return re;
    }

    sipmXY getSipmXY(const int ch)
    {
	SiPM sp = getSipm(ch);
	if (sp.layer < 0)
	    return {0, 0};

	sipmXY pos;
	int bl = boardLabel[sp.layer][sp.quadrant];
	if (25 == bl)   // left side hexagonal
	    pos = hexBoardSipmXY_topdown[sp.sipm];
	else if (28 == bl || 37 == bl)	// right side hexagonal, flip the index
	    pos = hexBoardSipmXY_topdown[nHexBoardChannels - 1 - sp.sipm];
	else if (  41 == bl || 21 == bl || 15 == bl || 46 == bl || 20 == bl 
		|| 45 == bl || 49 == bl || 23 == bl || 52 == bl || 19 == bl
		|| 43 == bl || 32 == bl // left side square
		|| 30 == bl || 44 == bl || 47 == bl || 31 == bl || 36 == bl
		|| 42 == bl || 39 == bl	// right side square
		)    
	    pos = sqaBoardSipmXY_topdown[sp.sipm];
	else if (18 == bl || 38 == bl || 24 == bl || 16 == bl || 17 == bl)	// right side square, flip the index
	    pos = sqaBoardSipmXY_topdown[nSqaBoardChannels - 1 - sp.sipm];
	else if (sp.layer < nHexLayers) // general hex tile
	    pos = hexBoardSipmXY[sp.sipm];
	else    // general square tile
	    pos = sqaBoardSipmXY[sp.sipm];

	if (0 == sp.quadrant || 3 == sp.quadrant)
	    pos = -pos;

	return pos + pcbAnchor[sp.quadrant];
    }

    string getListFile(const int run)
    {
	char listFile[1024];
	sprintf(listFile, "%s/data/Run%d_list.txt", CALIROOT, run);
	if (!fileExists(listFile))
	{
	    sprintf(listFile, "%s/data/Run%d_list.txt", backupDir, run);
	    if (!fileExists(listFile))
	    {
		cerr << FATAL << "can't find the list file for run: " << run << endl;
		return NULL;
	    }
	}
	return listFile;
    }

    string getRootFile(const int run)
    {
	char rootFile[1024];
	sprintf(rootFile, "%s/data/Run%d.root", CALIROOT, run);
	if (!fileExists(rootFile))
	{
	    sprintf(rootFile, "%s/data/Run%d.root", backupDir, run);
	    if (!fileExists(rootFile))
	    {
		cerr << FATAL << "can't find the root file for run: " << run << endl;
		return NULL;
	    }
	}
	return rootFile;
    }

    string getRootFile(const char* fname)
    {
	char rootFile[1024];
	char dirs[3][1024];
	sprintf(dirs[0], ".");
	sprintf(dirs[1], "%s/data", CALIROOT);
	sprintf(dirs[2], "%s/data", backupDir);
	for (char* dir : dirs)
	{
	    sprintf(rootFile, "%s/%s", dir, fname);
	    if (fileExists(rootFile))
		return rootFile;
	}

	cerr << FATAL << "can't find the root file: " << fname << endl;
	return NULL;
    }

    string getPedFile(const int run)
    {
	char file[1024];
	sprintf(file, "%s/data/Run%d_ped.json", CALIROOT, run);
	if (!fileExists(file))
	{
	    sprintf(file, "%s/data/Run%d_ped.json", backupDir, run);
	    if (!fileExists(file))
	    {
		cerr << FATAL << "can't find the ped file for run: " << run << endl;
		return NULL;
	    }
	}
	return file;
    }

    string getMipFile(const int run)
    {
	char file[1024];
	sprintf(file, "%s/data/Run%d_MIP.json", CALIROOT, run);
	if (!fileExists(file))
	{
	    sprintf(file, "%s/data/Run%d_MIP.json", backupDir, run);
	    if (!fileExists(file))
	    {
		cerr << FATAL << "can't find the MIP file for run: " << run << endl;
		return NULL;
	    }
	}
	return file;
    }

    void printSipmInfo(const int ch = 0)
    {
	assert(ch <= nChannels);
	SiPM sp = getSipm(ch);
	printf("Channel: %d\n", ch);
	printf("Board: %d\n", sp.layer*nLayerBoards + sp.quadrant);
	printf("Board Label: %d\n", boardLabel[sp.layer][sp.quadrant]);
	printf("Layer: %d\n", sp.layer);
	printf("Quadrant in Layer: %d\n", sp.quadrant);
	printf("SiPM in Board: %d\n", sp.sipm);
	sipmXY pos = getSipmXY(ch);
	printf("X:\t%.2f cm\tY:\t%.2f cm\n", pos.x/cm, pos.y/cm);
    }
}

#endif

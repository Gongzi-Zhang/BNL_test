#ifndef __CALI__
#define __CALI__

#include <stdlib.h>
#include <cassert>
#include "utilities.h"

// units
int mm = 1;
int cm = 10*mm;

namespace cali {
    const char* CALIROOT = (assert(getenv("CALIROOT") != NULL), true)
	? getenv("CALIROOT")
	: ".";

    /* CEAN:	  CAEN unit
     * channel:   channel count: 0-192
     * layer:	  sampling layers
     * board:	  PCB count: 0-44
     * quadrant: local quadrant count: 0-3
     * sipm:    local quadrant SiPM count: 0-6
     */

	  int run = -1;
    	  int nCAENs = 3;
    const int nCAENChannels = 64;
	  int nChannels = nCAENs*nCAENChannels;
          int nLayers = 11;
    const int layerMax = 20;
    const int nHexLayers = 4;
	  int nSqaLayers = 9;
    const int nLayerBoards = 4;
    const int nHexBoards = nHexLayers*nLayerBoards;
          int nSqaBoards = nSqaLayers*nLayerBoards;
    const int nHexBoardChannels = 7;
    const int nSqaBoardChannels = 4;
    const int nHexChannels = nHexBoards*nHexBoardChannels;
    const int nSqaChannels = nSqaBoards*nSqaBoardChannels;

    const int nEightLayerBoards = 8*nLayerBoards;
    const int nNineLayerBoards  = 9*nLayerBoards;
    const int channelMax = 300;

    const double gapX = 0*mm;
    const double gapY = 2.54*mm;	// 0.1 in
    const double pcbX = 131.92*mm;	// 177.64 - 45.72
    const double pcbY = 97.99*mm;	// 130.91 - 32.92
    const double layerZ = 27.1526*mm;

    const char* gains[] = {"LG", "HG"};

    struct ADC_L {
	long long LG;
	long long HG;
    };
    struct ADC {
	float LG;
	float HG;
    };

    void setRun(const int r)
    {
	run = r;
	// if (run > 500)
	// {
	//     nCAENs = 4;
	//     nChannels = nCAENs*nCAENChannels;
	//     nSqaLayers = 
	//     nSqaBoards = nSqaLayers*nLayerBoards;
	//     nSqaChannels = nSqaBoards*nSqaBoardChannels;
	// }
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
	{ 0, 15, 46, 35},   // square
	{ 0, 20, 45, 36},   // square
	{42, 49, 23, 16},   // square
	{17, 52, 19, 51},   // square
	{55, 43, 32, 39},   // square
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
	    std::cerr << ERROR << "Invalid channel number:" << ch << std::endl;
	    std::cout << INFO << "Allowed channel range: 0 - " << nChannels - 1 << std::endl;
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
	else if (18 == bl || 38 == bl || 35 == bl || 16 == bl || 17 == bl)	// right side square, flip the index
	    pos = sqaBoardSipmXY_topdown[nSqaBoardChannels - 1 - sp.sipm];
	else if (sp.layer < nHexLayers) // general hex tile
	    pos = hexBoardSipmXY[sp.sipm];
	else    // general square tile
	    pos = sqaBoardSipmXY[sp.sipm];

	if (0 == sp.quadrant || 3 == sp.quadrant)
	    pos = -pos;

	return pos + pcbAnchor[sp.quadrant];
    }

    void printSipmXY(const int chMin = 0, const int chMax = nChannels - 1)
    {
	assert(chMin <= chMax);
	for (int ch=chMin; ch<=chMax; ch++) 
	{
	    sipmXY pos = getSipmXY(ch);
	    printf("%3d\t%.2f\t%.2f\n", ch, pos.x, pos.y);
	}
    }
}

#endif

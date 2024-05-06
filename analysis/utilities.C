/* channel: global channel count: 0-192
 * quadrant:   global quadrant count: 0-44
 * layer:   global layer count: 0-11
 *
 * quadrant: local quadrant count: 0-3
 * sipm:    local quadrant SiPM count: 0-6
 *
 * -- Weibin 20240430
 */

#include "spdlog/spdlog.h"

// auto logger = spdlog::stdout_logger_mt("cali_utitlities");
// logger->set_pattern("[%^%l%$] %v");
// logger->set_level(logger->level::debug);

const int nLayers = 11;
const int nLayerBoards = 4;
const int nHexBoardChannels = 7;
const int nSqaBoardChannels = 4;
const int nHexLayers = 4;
const int nHexBoards = 4*nHexLayers;
const int nHexChannels = 7*nHexBoards;
const int nEightLayerChannels = 4*nLayerBoards*nHexBoardChannels + 4*nLayerBoards*nSqaBoardChannels;
const int nNineLayerChannels = nEightLayerChannels + 3*4;
const int nTenLayerChannels = nEightLayerChannels + 2*3*4;
const int maxChannel = 192 - 1;

// units
int mm = 1;
int cm = 10*mm;

const double gapX = 0*mm;
const double gapY = 2.54*mm;	// 0.1 in
const double pcbX = 131.92*mm;	// 177.64 - 45.72
const double pcbY = 97.99*mm;	// 130.91 - 32.92
const double layerZ = 27.1526*mm;

typedef struct {
    int layer;	
    int quadrant;	
    int sipm;
} SiPM;

struct Pos{
    double x, y;
    
    inline void	operator=(Pos a)    { x=a.x; y=a.y; }
    inline Pos  operator-()	    { return {-x, -y}; }
    inline Pos  operator-(Pos a)    { return {x-a.x, y-a.y}; }
    inline Pos  operator+(Pos a)    { return {x+a.x, y+a.y}; }
    inline bool	operator==(Pos a)   { return (x == a.x) && (y == a.y); }
};

int boardLabel[][4] = {
// top right, top left, bottom left, bottom right
    { 1,  2,  3,  4},	// hexagon
    { 5,  6,  7,  8},	// hexagon
    {10,  9, 11, 12},	// hexagon
    {26, 13, 25, 28},	// hexagon
    {30, 41, 14, 37},	// transition: top 2 square, bottom 2 hexagon
    {38, 54, 21, 18},	// square
    {44, 22, 58, 47},	// square
    {50, 57, 33, 31},	// square
    { 0, 15, 46, 35},	// square
    { 0, 20, 45, 36},   // square
    {42, 49, 23, 16},   // square
    {17, 52, 19, 51},   // square
    {55, 43, 32, 39},   // square
};

Pos pcbAnchor[] = {
    {-gapX/2, gapY/2 + pcbY},
    { gapX/2, gapY/2},
    { gapX/2, -(gapY/2 + pcbY)},
    {-gapX/2, -gapY/2},
};
                                        
Pos hexBoardSipmPos[nHexBoardChannels] = {
    {50.01*mm, 80.90*mm},
    {77.64*mm, 64.95*mm},
    {22.37*mm, 64.95*mm},
    {50.01*mm, 48.99*mm},
    {77.64*mm, 33.04*mm},
    {22.37*mm, 33.04*mm},
    {50.01*mm, 17.08*mm},
};
Pos sqaBoardSipmPos[nSqaBoardChannels] = {
    {73.9*mm, 72.89*mm},
    {26.1*mm, 72.89*mm},
    {73.9*mm, 25.09*mm},
    {26.1*mm, 25.09*mm},
};
Pos otherHexBoardSipmPos[nHexBoardChannels] = {
    {50.01*mm, 80.90*mm},
    {22.37*mm, 64.95*mm},
    {77.64*mm, 64.95*mm},
    {50.01*mm, 48.99*mm},
    {77.64*mm, 33.04*mm},
    {22.37*mm, 33.04*mm},
    {50.01*mm, 17.08*mm},
};
Pos otherSqaBoardSipmPos[nSqaBoardChannels] = {
    {26.1*mm, 72.89*mm},
    {73.9*mm, 72.89*mm},
    {26.1*mm, 25.09*mm},
    {73.9*mm, 25.09*mm},
};

bool getSipm(const int ch, SiPM& re)
{
    if (ch < 0 || ch > maxChannel)
    {
	spdlog::error("Invalid channel number {}", ch);
	spdlog::info("Allowed channel range: 0 - {}", maxChannel);
	return false;
    }

    int restCh = 0;
    int board = -1;
    int sipm = -1;
    if (ch < nHexChannels)
    {
	board = ch / nHexBoardChannels;
	sipm = ch % nHexBoardChannels;
    }
    else if (ch < nEightLayerChannels)
    {
	restCh = ch - nHexChannels;
	board = nHexBoards + restCh / nSqaBoardChannels;
	sipm = restCh % nSqaBoardChannels;
    }
    else if (ch < nNineLayerChannels)
    {
	restCh = ch - nEightLayerChannels;
	board = 8*nLayerBoards + 1 + restCh / nSqaBoardChannels;
	sipm = restCh % nSqaBoardChannels;
    }
    else
    {	// no 10th layer right now
	restCh = ch - nNineLayerChannels;
	board = 10*nLayerBoards + restCh / nSqaBoardChannels;
	sipm = restCh % nSqaBoardChannels;
    }

    re.layer = board / nLayerBoards;
    re.quadrant = board % nLayerBoards;
    re.sipm = sipm;

    return true;
}

bool getSipmPos(const int ch, Pos& pos)
{
    SiPM sp;
    if (! getSipm(ch, sp))
	return false;

    int bl = boardLabel[sp.layer][sp.quadrant];
    if (25 == bl)   // left side hexagonal
	pos = otherHexBoardSipmPos[sp.sipm];
    else if (28 == bl || 37 == bl)	// right side hexagonal, flip the index
	pos = otherHexBoardSipmPos[nHexBoardChannels - 1 - sp.sipm];
    else if (21 == bl || 15 == bl || 46 == bl)	// left side square
	pos = otherSqaBoardSipmPos[sp.sipm];
    else if (18 == bl || 35 == bl || 38 == bl)	// right side square, flip the index
	pos = otherSqaBoardSipmPos[nSqaBoardChannels - 1 - sp.sipm];
    else if (sp.layer < nHexLayers) // general hex tile
	pos = hexBoardSipmPos[sp.sipm];
    else    // general square tile
	pos = sqaBoardSipmPos[sp.sipm];

    if (0 == sp.quadrant || 3 == sp.quadrant)
	pos = -pos;

    pos = pos + pcbAnchor[sp.quadrant];
    return true;
}

void printSipmPos(const int chMin = 0, const int chMax = maxChannel)
{
    assert(chMin <= chMax);
    Pos pos;
    for (int ch=chMin; ch<=chMax; ch++) 
    {
	getSipmPos(ch, pos);
	printf("%3d\t%.2f\t%.2f\n", ch, pos.x, pos.y);
    }
}

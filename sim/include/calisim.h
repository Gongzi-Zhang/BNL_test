#ifndef __CALISIM__
#define __CALISIM__

#include "cali.h"

namespace calisim 
{

    int getChId(const int cellID)
    {   // 0-191
	// for 20241002
	int orderedRightHex[]    = {3, 1, 0, 2, 4, 6, 5};
	int orderedLeftHex[]	 = {3, 2, 0, 1, 5, 6, 4};   
	int disorderedRightHex[] = {3, 5, 6, 4, 1, 0, 2};   
	int disorderedLeftHex[]	 = {3, 1, 0, 2, 5, 6, 4};   
	int orderedRightSqa[]    = {1, 0, 2, 3};
	int orderedLeftSqa[]	 = {1, 0, 2, 3};	// 41
	int disorderedRightSqa1[]= {2, 3, 1, 0};	// 30, 44, 47, 31
	int disorderedRightSqa2[]= {3, 2, 0, 1};	// 50
	int disorderedLeftSqa[]	 = {0, 1, 3, 2};

	int system =  cellID & 0x000000FF;		// start from 1
	int layer  = (cellID & 0x0000FF00) >> 8;	// start from 1
	int slice  = (cellID & 0x00FF0000) >> 16;	// start from 1
	layer -= 2;	// dis-count the first 2 trigger layer

	if (layer < 1)  // the front 2 trigger tiles
	    return layer-1;

	if ((layer == 9 || layer == 10) && slice == 1)	// the showermax trigger tiles
	    return -layer;

	if (layer <= 3) // 0 - 83
	{
	    int quadrant = (slice - 1) / cali::nHexBoardChannels;
	    int sipm = (slice - 1) % cali::nHexBoardChannels;

	    int layerOffset = (layer-1)*cali::nLayerBoards*cali::nHexBoardChannels;
	    int quadOffset = quadrant*cali::nHexBoardChannels;
	    int offset = layerOffset + quadOffset;

	    if (quadrant == 0 || quadrant == 3) // right
		return disorderedRightHex[sipm] + offset;
	    else if (quadrant == 1 || quadrant == 2)    // left
		return disorderedLeftHex[sipm] + offset;
	}
	else if (layer == 4)    // 84 - 111
	{
	    int quadrant = (slice - 1) / cali::nHexBoardChannels;
	    int sipm = (slice - 1) % cali::nHexBoardChannels;

	    int layerOffset = (layer-1)*cali::nLayerBoards*cali::nHexBoardChannels;
	    int quadOffset = quadrant*cali::nHexBoardChannels;
	    int offset = layerOffset + quadOffset;

	    if (quadrant == 0)
		return disorderedRightHex[sipm] + offset;
	    else if (quadrant == 1)
		return disorderedLeftHex[sipm] + offset;
	    else if (quadrant == 2)
		return orderedLeftHex[sipm] + offset;
	    else if (quadrant == 3)
		return orderedRightHex[sipm] + offset;
	}
	else if (layer == 5)    // 112 - 127
	{
	    int quadrant = (slice <= 8) ? (slice-1)/cali::nSqaBoardChannels : 2 + (slice-8-1) / cali::nHexBoardChannels;
	    int sipm = (quadrant < 2) ? (slice-1) % cali::nSqaBoardChannels : (slice-8-1) % cali::nHexBoardChannels;
	    int offset = 112 + 4*quadrant;

	    if (quadrant == 0)  // bd 30
		return disorderedRightSqa1[sipm] + offset;
	    else if (quadrant == 1)	// bd 41
		return orderedLeftSqa[sipm] + offset;
	    else if (quadrant == 2)	// bd 14
	    {
		if (sipm < 4)
		    return disorderedLeftHex[sipm] + offset;
		else 
		    return -1024;	// unused channels
	    }
	    else if (quadrant == 3)	// bd 37
	    {
		if (sipm < 4)
		    return orderedRightHex[sipm] + offset;
		else
		    return -1024;	// unused channels
	    }
	}
	else if (6 <= layer && layer <= 8)	// 128 - 175
	{
	    int quadrant = (slice-1) / cali::nSqaBoardChannels;
	    int sipm = (slice-1) % cali::nSqaBoardChannels;
	    int offset = 112 + 16*(layer-5) + 4*quadrant;
	    if ((layer == 6 && quadrant == 0)   // bd 38
	     || (layer == 6 && quadrant == 3)   // bd 18
	       )
		return orderedRightSqa[sipm] + offset;
	    else if ((layer == 6 && quadrant == 1)	// bd 54
		  || (layer == 7 && quadrant == 1)	// bd 22
		  || (layer == 7 && quadrant == 2)	// bd 58
		  || (layer == 8 && quadrant == 1)	// bd 57
		  || (layer == 8 && quadrant == 2)	// bd 33
		    )
		return disorderedLeftSqa[sipm] + offset;
	    else if ((layer == 6 && quadrant == 2)	// bd 21
		    )	
		return orderedLeftSqa[sipm] + offset;
	    else if ((layer == 7 && quadrant == 0)	// bd 44
		  || (layer == 7 && quadrant == 3)	// bd 47
		  || (layer == 8 && quadrant == 3)	// bd 31
		    )
		return disorderedRightSqa1[sipm] + offset;
	    else if ((layer == 8 && quadrant == 0)	// bd 50
		    )
		  return disorderedRightSqa2[sipm] + offset;
	}
	else if (layer == 9)    // 176 - 187
	{
	    int quadrant = 1 + (slice-1-1) / cali::nSqaBoardChannels;
	    int sipm = (slice-1-1) % cali::nSqaBoardChannels;
	    int offset = 112 + 16*(9-5) + 4*(quadrant-1);
	    if (quadrant == 1   // bd 15
	     || quadrant == 2   // bd 46
	       )
		return orderedLeftSqa[sipm] + offset;
	    else if (quadrant == 3)	// bd 24
		return orderedRightSqa[sipm] + offset;
	}
	else if (layer == 10)   // 188 - 191
	{
	    int quadrant = 1 + (slice-1-1) / cali::nSqaBoardChannels;
	    int sipm = (slice-1-1) % cali::nSqaBoardChannels;
	    int offset = 112 + 16*(9-5) + 12 + 4*(quadrant-1);
	    if (quadrant == 1)  // bd 20
		return orderedLeftSqa[sipm] + offset;
	    else
		return -1024;   // unused channels
	}

	return -1024;
    }

    bool isBadChannel(const int ch)
    {
	if ( (128 <= ch && ch <= 135)   // channel 128 - 135
	   || 5 == ch || 27 == ch || 32 == ch || 50 == ch
	   )
	    return true;
	else
	    return false;

    }

    bool isTriggerChannel(const int ch)
    {
	if (-1 == ch || -2 == ch || -9 == ch || -10 == ch)
	    return true;
	else
	    return false;
    }

    void test() {
	int system = 1;
	int layer = 1;
	int slice = 1;
	for (layer=1; layer<=2; layer++)
	{
	    slice = 1;
	    int id = (slice << 16) + (layer << 8) + system;
	    printf("layer--%d\tslice--%d\tchannel--%d\n", layer, slice, getChId(id));
	}
	for (layer=3; layer<=6; layer++)
	{
	    for (slice=1; slice<=28; slice++)
	    {
		int id = (slice << 16) + (layer << 8) + system;
		printf("layer--%d\tslice--%d\tchannel--%d\n", layer, slice, getChId(id));
	    }
	}
	{
	    layer=7;
	    for (slice=1; slice<=22; slice++)
	    {
		int id = (slice << 16) + (layer << 8) + system;
		printf("layer--%d\tslice--%d\tchannel--%d\n", layer, slice, getChId(id));
	    }
	}
	for (layer=8; layer<=10; layer++)
	{
	    for (slice=1; slice<=16; slice++)
	    {
		int id = (slice << 16) + (layer << 8) + system;
		printf("layer--%d\tslice--%d\tchannel--%d\n", layer, slice, getChId(id));
	    }
	}
	for (layer=11; layer<=12; layer++)
	{
	    for (slice=1; slice<=13; slice++)
	    {
		int id = (slice << 16) + (layer << 8) + system;
		printf("layer--%d\tslice--%d\tchannel--%d\n", layer, slice, getChId(id));
	    }
	}
    }
}

#endif

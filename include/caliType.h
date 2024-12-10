#ifndef __CALITYPE__
#define __CALITYPE__

#include "TObject.h"

class caliHit : public TObject
{
  public:
    float x, y;
    int z;
    float e;

    caliHit() : x(0), y(0), z(0), e(0) {}
    caliHit(float _x, float _y, int _z, float _e) : x(_x), y(_y), z(_z), e(_e) {}

    ClassDef(caliHit, 1)
};

class caliCluster : public TObject
{
  public:
    size_t nhits;
    float e;

    caliCluster() : nhits(0), e(0) {}
    caliCluster(size_t n, float _e) : nhits(n), e(_e) {}

    ClassDef(caliCluster, 1)
};

ClassImp(caliCluster)
ClassImp(caliHit)

#endif

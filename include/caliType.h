#ifndef CALITYPE_H
#define CALITYPE_H

#include <vector>

class caliHit : public TObject
{
  public:
    float x, y, z;
    float e;

    caliHit() : x(0), y(0), z(0), e(0) {}
    caliHit(float _x, float _y, float _z, float _e) : x(_x), y(_y), z(_z), e(_e) {}

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

/*
class caliHitCollection : public TObject
{
  private:
    std::vector<caliHit> hits; 

  public:
    size_t getHitNumber() { return hits.size(); }
    std::vector<caliHit> getHits() { return hits; }
    caliHit operator[](const size_t i) { assert(i < hits.size()); return hits[i]; }
    void addHit(caliHit hit) { hits.push_back(hit); }
    void clear() { hits.clear(); }
    ClassDef(caliHitCollection, 1)
};

class caliClusterCollection : public TObject 
{
  private:
    std::vector<caliCluster> clusters; 
				
  public:
    size_t getClusterNumber() { return clusters.size(); }
    std::vector<caliCluster> getClusters() { return clusters; }
    caliCluster operator[](const size_t i) { assert(i < clusters.size()); return clusters[i]; }
    void addCluster(caliCluster clu) { clusters.push_back(clu); }
    void clear() { clusters.clear(); }
    ClassDef(caliClusterCollection, 1)
};
*/

ClassImp(caliCluster)
ClassImp(caliHit)

#endif

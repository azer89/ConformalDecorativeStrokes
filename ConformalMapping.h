#ifndef CONFORMALMAPPING_H
#define CONFORMALMAPPING_H

#include "QuadMesh.h"


enum NeighborDirection
{
    ND_LEFT  = 0,
    ND_RIGHT = 1,
    ND_UP    = 2,
    ND_DOWN  = 3,
};


/**
 * Note that the iteration uses QTimer rather than a loop because of a "interactive animation" decision.
 * So you need to see mainwindow.cpp to see my timer details.
 */
class ConformalMapping
{
public:
    ConformalMapping();

    /**
     * Simple averaging, even for vertices on boundaries.
     * This function needs to be called repeatedly until convergence.
     */
    void ConformalMappingOneStepSimple(std::vector<QuadMesh>& quadMeshes);

    /**
     * Fancier calculation for vertices on boundaries.
     * see "Warping Pictures Nicely" paper.
     * This function needs to be called repeatedly until convergence.
     */
    void ConformalMappingOneStep(std::vector<QuadMesh>& quadMeshes);

    void MappingInterpolation(std::vector<QuadMesh>& quadMeshes);

    float GetIterDist() { return _iterDist; }

    void SetSlidingConstraint(std::vector<std::vector<AVector>> sConstraints) { _slidingConstraints = sConstraints; }

public:
    // debugging (delete after use)
    std::vector<AVector>  _debugPoints;
    std::vector<ALine>    _debugLines;

    std::vector<std::vector<AVector>> _slidingConstraints;

private:
    void ConformalMappingOneStepSimple(QuadMesh *qMesh);

    void ConformalMappingOneStep(QuadMesh* prevQMesh, QuadMesh* curQMesh, QuadMesh* nextQMesh);

    PlusSignVertex GetNeighbor(int x, int y, NeighborDirection dir,
                               QuadMesh* prevQMesh, QuadMesh* curQMesh, QuadMesh* nextQMesh,
                               std::vector<std::vector<PlusSignVertex>> pTempVertices,
                               std::vector<std::vector<PlusSignVertex>> cTempVertices,
                               std::vector<std::vector<PlusSignVertex>> nTempVertices);

    void UpdateNeighbor(int x, int y,
                                  QuadMesh* prevQMesh, QuadMesh* curQMesh, QuadMesh* nextQMesh,
                                  std::vector<std::vector<PlusSignVertex> > &pTempVertices,
                                  std::vector<std::vector<PlusSignVertex> > &cTempVertices,
                                  std::vector<std::vector<PlusSignVertex> > &nTempVertices);

    void MappingInterpolation(QuadMesh *qMesh);

    AVector GetClosestPointFromBorders(int x, int y, AVector pt, QuadMesh* prevQMesh, QuadMesh* curQMesh, QuadMesh* nextQMesh);

    AVector GetClosestPointFromRibs(int x, int y, AVector pt, QuadMesh* curQMesh);

    void GetClosestIndicesAndRatios(std::vector<AVector> boundary1,
                                    std::vector<AVector> boundary2,
                                    std::vector<std::pair<int, int>>& pairIndices,
                                    std::vector<float>& ratios);


private:
    /**
     * This value determines when the conformal mapping converges.
     * Currently, the iteration stops if _iterDist equals to zero
     */
    float _iterDist;
};

#endif // CONFORMALMAPPING_H

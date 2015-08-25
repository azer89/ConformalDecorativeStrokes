#ifndef CONFORMALMAPPING_H
#define CONFORMALMAPPING_H

#include "QuadMesh.h"

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

    float GetIterDist() { return _iterDist; }


private:
    void ConformalMappingOneStepSimple(QuadMesh *qMesh);
    void ConformalMappingOneStep(QuadMesh *qMesh);

    void MappingInterpolation(QuadMesh oriQMesh, QuadMesh *qMesh);

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

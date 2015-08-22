#ifndef CONFORMALMAPPING_H
#define CONFORMALMAPPING_H

#include "QuadMesh.h"

class ConformalMapping
{
public:
    ConformalMapping();

    void ConformalMappingOneStepSimple(std::vector<QuadMesh>& quadMeshes);
    void ConformalMappingOneStep(std::vector<QuadMesh>& quadMeshes);

    float GetIterDist() { return _iterDist; };

private:
    void ConformalMappingOneStepSimple(QuadMesh *qMesh);
    void ConformalMappingOneStep(QuadMesh *qMesh);

    AVector GetClosestPointFromBorders(QuadMesh qMesh, AVector pt);

private:
    float _iterDist;
};

#endif // CONFORMALMAPPING_H

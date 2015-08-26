#ifndef QUADMESH_H
#define QUADMESH_H

#include "AVector.h"
#include "ALine.h"
#include "PlusSignVertex.h"



enum QuadMeshType
{
    MESH_RECTANGLE = 0,
    MESH_KITE = 1,
};

struct QuadMesh
{
public:
    QuadMesh();
    ~QuadMesh();

    int GetWidth();
    int GetHeight();

    std::vector<AVector> GetABoundary(int index, bool isXUnchanged, bool isOri);

    AVector GetClosestPointFromBorders(AVector pt);

public:

    AVector _leftStartPt;
    AVector _leftEndPt;
    AVector _rightStartPt;
    AVector _rightEndPt;

    QuadMeshType _quadMeshType;
    AVector _sharpPt; // for kite only
    bool _isRightKite; // a stroke which turns right
    // Vertices
    std::vector<std::vector<PlusSignVertex>> _psVertices;
    std::vector<std::vector<PlusSignVertex>> _opsVertices;
};

#endif // QUADMESH_H

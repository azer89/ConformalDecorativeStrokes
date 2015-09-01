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

    QuadMesh(AVector      leftStartPt,
             AVector      leftEndPt,
             AVector      rightStartPt,
             AVector      rightEndPt,
             AVector      sharpPt,
             bool         isRightKite,
             QuadMeshType quadMeshType);

    QuadMesh(AVector      leftStartPt,
             AVector      leftEndPt,
             AVector      rightStartPt,
             AVector      rightEndPt,
             QuadMeshType quadMeshType);

    int GetWidth();

    int GetHeight();

    std::vector<AVector> GetABoundary(int index, bool isXUnchanged, bool isOri);

    AVector GetClosestPointFromBorders(AVector pt);

public:

    // the shape of the mesh is quadrilateral,
    // so we can define it with four corner positions
    AVector _leftStartPt;
    AVector _leftEndPt;
    AVector _rightStartPt;
    AVector _rightEndPt;

    QuadMeshType _quadMeshType;
    AVector _sharpPt; // for kite only
    bool _isRightKite; // a stroke which turns right

    // movable vertices
    std::vector<std::vector<PlusSignVertex>> _psVertices;

    // original vertices (we do not apply any conformal mapping)
    std::vector<std::vector<PlusSignVertex>> _opsVertices;
};

#endif // QUADMESH_H

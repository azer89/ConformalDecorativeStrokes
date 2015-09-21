#ifndef QUADMESH_H
#define QUADMESH_H

#include "AVector.h"
#include "ALine.h"
#include "PlusSignVertex.h"



enum QuadMeshType
{    
    MESH_KITE = 0,
    MESH_RECTANGLE   = 1, // obsolete
    MESH_LEG         = 2,
    MESH_RECTILINEAR = 3,
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

    std::vector<AVector> GetSideBoundary(int index);

    AVector GetClosestPointFromBorders(AVector pt);

    ALine GetTopLine()    { return ALine(_leftStartPt,  _leftEndPt); }
    ALine GetBottomLine() { return ALine(_rightStartPt, _rightEndPt); }
    ALine GetLeftLine()   { return ALine(_leftStartPt,  _rightStartPt); }
    ALine GetRightLine()  { return ALine(_leftEndPt,    _rightEndPt); }

public:

    float _textureLength;

    // to do: should add the entire borders (take from left and right lines...)

    // the shape of the mesh is quadrilateral,
    // so we can define it with four corner positions
    AVector _leftStartPt;
    AVector _leftEndPt;
    AVector _rightStartPt;
    AVector _rightEndPt;

    QuadMeshType _quadMeshType;
    AVector _innerConcavePt; // for kite only
    bool _isRightKite; // a stroke which turns right

    // movable vertices
    std::vector<std::vector<PlusSignVertex>> _psVertices;

    // original vertices (we do not apply any conformal mapping)
    std::vector<std::vector<PlusSignVertex>> _opsVertices;
};

#endif // QUADMESH_H

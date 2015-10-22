#ifndef QUADMESH_H
#define QUADMESH_H

#include "AVector.h"
#include "ALine.h"
#include "PlusSignVertex.h"

/**
  * Reza Adhitya Saputra
  * radhitya@uwaterloo.ca
  */


/*
enum QuadMeshType
{
    MESH_KITE = 0,
    //MESH_RECTANGLE   = 1, // obsolete
    MESH_LEFT_LEG    = 1,
    MESH_RIGHT_LEG   = 2,
    MESH_RECTILINEAR = 3,
};
 */

enum QuadMeshType
{    
    MESH_KITE = 0,
    MESH_LEFT_LEG    = 1,
    MESH_RIGHT_LEG   = 2,
    MESH_RECTILINEAR = 3,
    TYPE_NUM = 4,};

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
    void SetSlidingConstraintFlag(int index, bool boolValue, std::vector<AVector> &debugPoints);

    AVector GetClosestPointFromBorders(AVector pt);

    void ResetVertices()
    {
        uint w = GetWidth();
        uint h = GetHeight();

        for(uint a = 0; a < w; a++)
        {
            for(uint b = 0; b < h; b++)
            {
                _psVertices[a][b].position = _opsVertices[a][b].position;
            }
        }
    }

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

#ifndef QUADMESH_H
#define QUADMESH_H

#include "AVector.h"
#include "ALine.h"
#include "PlusSignVertex.h"

#include "UtilityFunctions.h"

#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

enum QuadMeshType
{
    MESH_RECTANGLE = 0,
    MESH_KITE = 1,
};

struct QuadMesh
{
public:
    QuadMesh()
    {        
    }

    ~QuadMesh()
    {
    }

    int GetWidth() { return this->_plusSignVertices.size(); }
    int GetHeight() { return this->_plusSignVertices[0].size(); }

    std::vector<AVector> GetABoundary(int index, bool isXAxis)
    {
        std::vector<AVector> vertices;
        if(isXAxis) // unchanged x index
        {
            int meshHeight = GetHeight();
            for(int yIter = 0; yIter < meshHeight; yIter++)
                { vertices.push_back(_plusSignVertices[index][yIter].position); }
        }
        else // unchanged y index
        {
            int meshWidth = GetWidth();
            for(int xIter = 0; xIter < meshWidth; xIter++)
                { vertices.push_back(_plusSignVertices[xIter][index].position); }
        }
        return vertices;
    }

    AVector GetClosestPointFromBorders(AVector pt)
    {
        AVector closestPt = pt;
        float dist = std::numeric_limits<float>::max();
        std::vector<ALine> borderLines;
        borderLines.push_back(ALine(_leftStartPt,  _rightStartPt));
        borderLines.push_back(ALine(_leftEndPt,    _rightEndPt));
        borderLines.push_back(ALine(_leftStartPt,  _leftEndPt));
        borderLines.push_back(ALine(_rightStartPt, _rightEndPt));
        for(uint a = 0; a < borderLines.size(); a++)
        {
            AVector cPt = UtilityFunctions::GetClosestPoint(borderLines[a].GetPointA(), borderLines[a].GetPointB(), pt);
            if(pt.Distance(cPt) < dist)
            {
                dist = pt.Distance(cPt);
                closestPt = cPt;
            }
        }
        return closestPt;
    }

public:

    AVector _leftStartPt;
    AVector _leftEndPt;
    AVector _rightStartPt;
    AVector _rightEndPt;

    QuadMeshType _quadMeshType;
    AVector _sharpPt; // for kite only
    bool _isRightKite; // a stroke which turns right
    // Vertices
    std::vector<std::vector<PlusSignVertex>> _plusSignVertices;
};

#endif // QUADMESH_H

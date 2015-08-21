#ifndef QUADMESH_H
#define QUADMESH_H

#include "AVector.h"
#include "ALine.h"
#include "PlusSignVertex.h"

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

    //void AddBorders(std::vector<AVector> lines)
    //{
    //    for(uint a = 0; a < lines.size() - 1; a++)
    //    {
    //        _borderLines.push_back(ALine(lines[a], lines[a+1]));
    //    }
    //}

    //void AddBorder(ALine aline)
    //{
    //    _borderLines.push_back(aline);
    //}


public:
    int _mesh_width;
    int _mesh_height;

    AVector _leftStartPt;
    AVector _leftEndPt;
    AVector _rightStartPt;
    AVector _rightEndPt;

    QuadMeshType _quadMeshType;
    AVector _sharpPt; // for kite only
    bool _isRightKite; // a stroke which turns right

    //std::vector<ALine>        _borderLines;

    // Vertices
    std::vector<std::vector<PlusSignVertex>> _plusSignVertices;
};

#endif // QUADMESH_H

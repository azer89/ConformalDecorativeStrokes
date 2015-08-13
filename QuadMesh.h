#ifndef QUADMESH_H
#define QUADMESH_H

#include "AVector.h"
#include "ALine.h"
#include "PlusSignVertex.h"

#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

struct QuadMesh
{
public:
    QuadMesh()
    {        
    }

    void AddBorders(std::vector<AVector> lines)
    {
        for(int a = 0; a < lines.size() - 1; a++)
        {
            _borderLines.push_back(ALine(lines[a], lines[a+1]));
        }
    }

    void AddBorder(ALine aline)
    {
        _borderLines.push_back(aline);
    }


public:
    int _mesh_width;
    int _mesh_height;

    // Vertices
    std::vector<std::vector<PlusSignVertex>> _plusSignVertices;
    QOpenGLBuffer               _plusSignVerticesVbo;
    QOpenGLVertexArrayObject    _plusSignVerticesVao;

    // Mesh
    std::vector<ALine>        _borderLines;
    //QOpenGLBuffer             _borderLinesVbo; // do I need this ?
    //QOpenGLVertexArrayObject  _borderLinesVao; // do I need this ?

    // Texture
    // Every mesh has its own texture
    QImage _img;
    QOpenGLTexture* _imgTexture;
    QOpenGLBuffer               _texturedStrokeVbo;
    QOpenGLVertexArrayObject    _texturedStrokeVao;
};

#endif // QUADMESH_H

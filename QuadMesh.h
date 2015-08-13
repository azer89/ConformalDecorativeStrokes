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

public:
    // Vertices
    std::vector<std::vector<PlusSignVertex>> _plusSignVertices;
    QOpenGLBuffer               _plusSignVerticesVbo;
    QOpenGLVertexArrayObject    _plusSignVerticesVao;

    // Mesh
    std::vector<ALine>        _borderLines;
    QOpenGLBuffer             _borderLinesVbo;
    QOpenGLVertexArrayObject  _borderLinesVao;

    // Texture
    // Every mesh has its own texture
    QImage _img;
    QOpenGLTexture* _imgTexture;
    QOpenGLBuffer               _texturedStrokeVbo;
    QOpenGLVertexArrayObject    _texturedStrokeVao;
};

#endif // QUADMESH_H

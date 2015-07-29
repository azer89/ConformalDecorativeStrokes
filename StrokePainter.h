#ifndef STROKEPAINTER_H
#define STROKEPAINTER_H

#include "ALine.h"
#include "AVector.h"
#include "VertexData.h"
#include "PlusSignVertex.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class StrokePainter
{
public:
    StrokePainter();
    ~StrokePainter();

    // mouse press
    void mousePressEvent(float x, float y);
    // mouse move
    void mouseMoveEvent(float x, float y);
    // mouse release
    void mouseReleaseEvent(float x, float y);

    void Draw();

public:
    QOpenGLShaderProgram* _shaderProgram;
    int         _colorLocation;
    int         _vertexLocation;
    int         _use_color_location;

private:

    int _mesh_width;
    int _mesh_height;

    // strokes
    std::vector<AVector>        _oriStrokeLines;    // not resampled
    std::vector<AVector>        _strokeLines;       // resampled
    QOpenGLBuffer               _strokeLinesVbo;
    QOpenGLVertexArrayObject    _strokeLinesVao;

    std::vector<ALine>          _ribLines;
    QOpenGLBuffer               _ribLinesVbo;
    QOpenGLVertexArrayObject    _ribLinesVao;

    std::vector<ALine>          _gridLines;
    QOpenGLBuffer               _gridLinesVbo;
    QOpenGLVertexArrayObject    _gridLinesVao;

    std::vector<AVector>        _lLines;
    QOpenGLBuffer               _lLinesVbo;
    QOpenGLVertexArrayObject    _lLinesVao;

    std::vector<AVector>        _rLines;
    QOpenGLBuffer               _rLinesVbo;
    QOpenGLVertexArrayObject    _rLinesVao;

    std::vector<AVector>        _points;
    QOpenGLBuffer               _pointsVbo;
    QOpenGLVertexArrayObject    _pointsVao;

    std::vector<AVector>        _vertices;
    QOpenGLBuffer               _verticesVbo;
    QOpenGLVertexArrayObject    _verticesVao;

    //int _plusSignDataSize;
    std::vector<std::vector<PlusSignVertex>> _plusSignVertices;
    QOpenGLBuffer               _plusSignVerticesVbo;
    QOpenGLVertexArrayObject    _plusSignVerticesVao;

private:
    void CalculateInitialRibbon();
    void CalculateVertices();

    void BuildLinesVertexData(std::vector<AVector> points, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol);
    void BuildLinesVertexData(std::vector<ALine> lines, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol);
    void BuildLinesVertexData(std::vector<std::vector<PlusSignVertex>> plusSignVertices, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol);
    void BuildPointsVertexData(std::vector<AVector> points, QOpenGLBuffer* ptsVbo, QOpenGLVertexArrayObject* ptsVao, QVector3D vecCol);
    void BuildVboWithColor(QVector<VertexData> data, QOpenGLBuffer* vbo);
};

#endif // STROKEPAINTER_H

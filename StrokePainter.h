#ifndef STROKEPAINTER_H
#define STROKEPAINTER_H

#include "ALine.h"
#include "AVector.h"

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

private:
    //void CalculateDecorativeStroke1();
    void CalculateInitialRibbon();

    void BuildLinesVertexData(std::vector<AVector> points, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol);
    void BuildLinesVertexData(std::vector<ALine> lines, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol);
    void BuildPointsVertexData(std::vector<AVector> points, QOpenGLBuffer* ptsVbo, QOpenGLVertexArrayObject* ptsVao, QVector3D vecCol);
};

#endif // STROKEPAINTER_H

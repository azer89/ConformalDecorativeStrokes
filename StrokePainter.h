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

private:
    void CalculateDecorativeStroke();

    void PrepareLinesVAO(std::vector<AVector> points, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol);

};

#endif // STROKEPAINTER_H

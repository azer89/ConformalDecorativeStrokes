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

    QSize MeshSize(){ return QSize(_mesh_width, _mesh_height); }

    void Draw();

    void ConformalMappingOneStep1();
    void ConformalMappingOneStep2();
    bool ShouldStop() { return _iterDist < std::numeric_limits<float>::epsilon(); }

public:
    QOpenGLShaderProgram* _shaderProgram;
    int         _colorLocation;
    int         _vertexLocation;
    int         _use_color_location;

private:

    int _mesh_width;
    int _mesh_height;

    float _iterDist;

    // strokes
    std::vector<AVector>        _oriStrokeLines;    // original
    std::vector<AVector>        _strokeLines;       // resampled and simplified
    QOpenGLBuffer               _strokeLinesVbo;
    QOpenGLVertexArrayObject    _strokeLinesVao;

    //std::vector<ALine>          _borderLines;
    //QOpenGLBuffer               _borderLinesVbo;
    //QOpenGLVertexArrayObject    _borderLinesVao;

    // debugging visualization
    //std::vector<ALine>          _ribLines;
    //QOpenGLBuffer               _ribLinesVbo;
    //QOpenGLVertexArrayObject    _ribLinesVao;

    // debugging visualization
    //std::vector<ALine>          _gridLines;
    //QOpenGLBuffer               _gridLinesVbo;
    //QOpenGLVertexArrayObject    _gridLinesVao;

    // left lines of the strokes
    std::vector<AVector>        _lLines;
    QOpenGLBuffer               _lLinesVbo;
    QOpenGLVertexArrayObject    _lLinesVao;

    // right lines of the stroke
    std::vector<AVector>        _rLines;
    QOpenGLBuffer               _rLinesVbo;
    QOpenGLVertexArrayObject    _rLinesVao;

    // debugging visualization
    //std::vector<AVector>        _points;
    //QOpenGLBuffer               _pointsVbo;
    //QOpenGLVertexArrayObject    _pointsVao;

    // debugging visualization
    //std::vector<AVector>        _debugPoints;
    //QOpenGLBuffer               _debugPointsVbo;
    //QOpenGLVertexArrayObject    _debugPointsVao;

    // debugging visualization
    //std::vector<ALine>          _debugLines;
    //QOpenGLBuffer               _debugLinesVbo;
    //QOpenGLVertexArrayObject    _debugLinesVao;

    // vertices of the mesh as a 1D list (debugging visualization)
    //std::vector<AVector>        _vertices;
    //QOpenGLBuffer               _verticesVbo;
    //QOpenGLVertexArrayObject    _verticesVao;

    // vertices of the mesh as 2D list
    std::vector<std::vector<PlusSignVertex>> _plusSignVertices;
    QOpenGLBuffer               _plusSignVerticesVbo;
    QOpenGLVertexArrayObject    _plusSignVerticesVao;

private:
    AVector GetClosestPointFromBorders(AVector pt);

    void CalculateInitialRibbon();
    void CalculateVertices();

    void BuildLinesVertexData(std::vector<AVector> points, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol);
    void BuildLinesVertexData(std::vector<ALine> lines, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol);
    void BuildLinesVertexData(std::vector<std::vector<PlusSignVertex>> plusSignVertices, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol);
    void BuildPointsVertexData(std::vector<AVector> points, QOpenGLBuffer* ptsVbo, QOpenGLVertexArrayObject* ptsVao, QVector3D vecCol);
    void BuildVboWithColor(QVector<VertexData> data, QOpenGLBuffer* vbo);
};

#endif // STROKEPAINTER_H

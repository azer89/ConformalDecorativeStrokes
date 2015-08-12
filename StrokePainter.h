#ifndef STROKEPAINTER_H
#define STROKEPAINTER_H

#include "ALine.h"
#include "AVector.h"
#include "VertexData.h"
#include "PlusSignVertex.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

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

    void SetImage(QString img);

public:
    QOpenGLShaderProgram* _shaderProgram;
    int         _colorLocation;
    int         _vertexLocation;
    int         _use_color_location;

private:

    bool _isMouseDown;

    // texture
    QImage _img;
    QOpenGLTexture* _imgTexture;
    QOpenGLBuffer               _texturedStrokeVbo;
    QOpenGLVertexArrayObject    _texturedStrokeVao;

    int _mesh_width;
    int _mesh_height;

    float _iterDist;

    // strokes
    std::vector<AVector>        _oriStrokeLines;    // original

    std::vector<AVector>        _spineLines;       // resampled and simplified from strokeLines
    QOpenGLBuffer               _spineLinesVbo;
    QOpenGLVertexArrayObject    _spineLinesVao;

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

    std::vector<ALine>          _junctionRibLines;
    QOpenGLBuffer               _junctionRibLinesVbo;
    QOpenGLVertexArrayObject    _junctionRibLinesVao;

    // left lines of the strokes
    // I need these lines to calculate closest points on the borders
    std::vector<AVector>        _leftLines;
    QOpenGLBuffer               _leftLinesVbo;
    QOpenGLVertexArrayObject    _leftLinesVao;

    // right lines of the stroke
    // I need these lines to calculate closest points on the borders
    std::vector<AVector>        _rightLines;
    QOpenGLBuffer               _rightLinesVbo;
    QOpenGLVertexArrayObject    _rightLinesVao;

    // debugging visualization
    //std::vector<AVector>        _points;
    //QOpenGLBuffer               _pointsVbo;
    //QOpenGLVertexArrayObject    _pointsVao;

    // debugging visualization
    //std::vector<AVector>        _debugPoints;
    //QOpenGLBuffer               _debugPointsVbo;
    //QOpenGLVertexArrayObject    _debugPointsVao;
    int _numConstrainedPoints;
    QOpenGLBuffer               _constrainedPointsVbo;
    QOpenGLVertexArrayObject    _constrainedPointsVao;

    // debugging visualization
    std::vector<ALine>          _debugLines;
    QOpenGLBuffer               _debugLinesVbo;
    QOpenGLVertexArrayObject    _debugLinesVao;

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
    AVector GetClosestPointFromMiddleVerticalLines(AVector pt);
    AVector GetClosestPointFromStrokeLines(AVector pt);
    AVector GetClosestPointFromStrokePoints(AVector pt);

    void CalculateInitialRibbon();
    //void CalculateInitialRibbon2();

    void CalculateVertices1();
    void CalculateVertices2();

    void BuildLinesVertexData(std::vector<AVector> points, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol);
    void BuildLinesVertexData(std::vector<ALine> lines, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol);

    void BuildLinesVertexData(std::vector<std::vector<PlusSignVertex>> plusSignVertices, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol);
    void BuildTexturedStrokeVertexData(std::vector<std::vector<PlusSignVertex>> plusSignVertices, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao);
    void BuildConstrainedPointsVertexData(std::vector<std::vector<PlusSignVertex>> plusSignVertices, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao, QVector3D vecCol);

    void BuildPointsVertexData(std::vector<AVector> points, QOpenGLBuffer* ptsVbo, QOpenGLVertexArrayObject* ptsVao, QVector3D vecCol);
    void BuildVboWithColor(QVector<VertexData> data, QOpenGLBuffer* vbo);
};

#endif // STROKEPAINTER_H

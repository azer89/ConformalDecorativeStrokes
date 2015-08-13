#ifndef STROKEPAINTER_H
#define STROKEPAINTER_H

#include "ALine.h"
#include "AVector.h"
#include "VertexData.h"
#include "PlusSignVertex.h"
#include "VertexDataHelper.h"
#include "QuadMesh.h"

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
    void SetVertexDataHelper(QOpenGLShaderProgram* shaderProgram);

public:
    /*
    // VertexDataHelper
    QOpenGLShaderProgram* _shaderProgram;
    int         _colorLocation;
    int         _vertexLocation;
    int         _use_color_location;
    */

private:

    bool _isMouseDown;

    VertexDataHelper* _vDataHelper;

    int _mesh_width;
    int _mesh_height;

    float _iterDist;

    // modification
    QuadMesh _aQuadMesh;

    // Modification
    // texture
    //QImage _img;
    //QOpenGLTexture* _imgTexture;
    //QOpenGLBuffer               _texturedStrokeVbo;
    //QOpenGLVertexArrayObject    _texturedStrokeVao;

    // strokes
    std::vector<AVector>        _oriStrokeLines;    // original

    // spines
    std::vector<AVector>        _spineLines;       // resampled and simplified from strokeLines
    QOpenGLBuffer               _spineLinesVbo;
    QOpenGLVertexArrayObject    _spineLinesVao;    

    //
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

    // points that cannot move at all
    int _numConstrainedPoints;
    QOpenGLBuffer               _constrainedPointsVbo;
    QOpenGLVertexArrayObject    _constrainedPointsVao;

    // debugging visualization
    std::vector<ALine>          _debugLines;
    QOpenGLBuffer               _debugLinesVbo;
    QOpenGLVertexArrayObject    _debugLinesVao;

    // modification
    // vertices of the mesh as 2D list
    //std::vector<std::vector<PlusSignVertex>> _plusSignVertices;
    //QOpenGLBuffer               _plusSignVerticesVbo;
    //QOpenGLVertexArrayObject    _plusSignVerticesVao;

    // debugging visualization
    //std::vector<AVector>        _points;
    //QOpenGLBuffer               _pointsVbo;
    //QOpenGLVertexArrayObject    _pointsVao;

    // debugging visualization
    //std::vector<AVector>        _debugPoints;
    //QOpenGLBuffer               _debugPointsVbo;
    //QOpenGLVertexArrayObject    _debugPointsVao;

    // vertices of the mesh as a 1D list (debugging visualization)
    //std::vector<AVector>        _vertices;
    //QOpenGLBuffer               _verticesVbo;
    //QOpenGLVertexArrayObject    _verticesVao;

    // delete (?)
    //std::vector<ALine>          _borderLines;
    //QOpenGLBuffer               _borderLinesVbo;
    //QOpenGLVertexArrayObject    _borderLinesVao;

    // debugging visualization delete (?)
    //std::vector<ALine>          _ribLines;
    //QOpenGLBuffer               _ribLinesVbo;
    //QOpenGLVertexArrayObject    _ribLinesVao;

    // debugging visualization delete (?)
    //std::vector<ALine>          _gridLines;
    //QOpenGLBuffer               _gridLinesVbo;
    //QOpenGLVertexArrayObject    _gridLinesVao;


private:
    AVector GetClosestPointFromBorders(AVector pt);
    AVector GetClosestPointFromMiddleVerticalLines(AVector pt);
    AVector GetClosestPointFromStrokeLines(AVector pt);
    AVector GetClosestPointFromStrokePoints(AVector pt);

    void CalculateInitialRibbon();
    //void CalculateInitialRibbon2();

    void CalculateVertices1();
    void CalculateVertices2();
};

#endif // STROKEPAINTER_H

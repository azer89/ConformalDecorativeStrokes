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

    void mousePressEvent(float x, float y);
    void mouseMoveEvent(float x, float y);
    void mouseReleaseEvent(float x, float y);

    //QSize MeshSize(){ return QSize(_aQuadMesh._mesh_width, _aQuadMesh._mesh_height); }

    void Draw();

    void ConformalMappingOneStep1();
    void ConformalMappingOneStep2();
    bool ShouldStop() { return _iterDist < std::numeric_limits<float>::epsilon(); }

    void SetImage(QString img);
    void SetVertexDataHelper(QOpenGLShaderProgram* shaderProgram);

public:

private:

    bool _isMouseDown;

    VertexDataHelper* _vDataHelper;

    float _iterDist;

    // modification
    //QuadMesh _aQuadMesh;

    std::vector<QuadMesh> _quadMeshes;
    int _qMeshNumData;
    QOpenGLBuffer               _quadMeshesVbo;
    QOpenGLVertexArrayObject    _quadMeshesVao;

    // texture
    //QImage _masterImg;
    //QOpenGLTexture* _masterImgTexture;

    // strokes
    std::vector<AVector>        _oriStrokeLines;    // original

    // spines
    std::vector<AVector>        _spineLines;       // resampled and simplified from strokeLines
    QOpenGLBuffer               _spineLinesVbo;
    QOpenGLVertexArrayObject    _spineLinesVao;    

    //
    //std::vector<ALine>          _junctionRibLines;
    //QOpenGLBuffer               _junctionRibLinesVbo;
    //QOpenGLVertexArrayObject    _junctionRibLinesVao;

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
    //int _numConstrainedPoints;
    //QOpenGLBuffer               _constrainedPointsVbo;
    //QOpenGLVertexArrayObject    _constrainedPointsVao;

    // debugging visualization
    //std::vector<ALine>          _debugLines;
    //QOpenGLBuffer               _debugLinesVbo;
    //QOpenGLVertexArrayObject    _debugLinesVao;

    // debugging visualization
    //std::vector<AVector>        _debugPoints;
    //QOpenGLBuffer               _debugPointsVbo;
    //QOpenGLVertexArrayObject    _debugPointsVao;


private:
    AVector GetClosestPointFromBorders(AVector pt);
    AVector GetClosestPointFromMiddleVerticalLines(AVector pt);
    AVector GetClosestPointFromStrokeLines(AVector pt);
    AVector GetClosestPointFromStrokePoints(AVector pt);

    void CalculateInitialRibbon();
    void CalculateLeftRightLines();
    void CalculateKitesAndRectangles();
    void CalculateSpines();
    //void CalculateInitialRibbon2();

    void CalculateVertices1();
    void CalculateVertices2(QuadMesh *qMesh);
};

#endif // STROKEPAINTER_H

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

    int QuadMeshSize() { return _quadMeshes.size(); }
    float IterationDelta() { return _iterDist; }
    //QSize MeshSize(){ return QSize(_aQuadMesh._mesh_width, _aQuadMesh._mesh_height); }

    void Draw();

    void ConformalMappingOneStep1();
    void ConformalMappingOneStep2();

    void ConformalMappingOneStep3();
    void ConformalMappingOneStep3(QuadMesh *qMesh);

    void CalculateVertices1();

    void CalculateVertices2();
    void CalculateVertices2(QuadMesh *qMesh);

    bool ShouldStop() { return _iterDist < std::numeric_limits<float>::epsilon(); }

    void SetStrokeTexture(QString img);
    void SetCornerTexture(QString img);

    void SetVertexDataHelper(QOpenGLShaderProgram* shaderProgram);

public:

private:

    bool _isMouseDown;

    VertexDataHelper* _vDataHelper;

    float _iterDist;

    std::vector<QuadMesh>       _quadMeshes;
    int                         _qMeshNumData;
    QOpenGLBuffer               _quadMeshesVbo;
    QOpenGLVertexArrayObject    _quadMeshesVao;

    //int                         _qMeshTexNumData;
    //QOpenGLBuffer               _quadMeshesTexVbo;
    //QOpenGLVertexArrayObject    _quadMeshesTexVao;

    // texture
    std::vector<QImage>          _masterImages;
    std::vector<QOpenGLTexture*> _masterTextures;
    std::vector<int>             _qmTexNumbers;
    std::vector<QOpenGLBuffer>             _qmTexVbos;
    std::vector<QOpenGLVertexArrayObject>  _qmTexVaos;
    //QImage _masterImg;
    //QOpenGLTexture* _masterImgTexture;

    // strokes
    std::vector<AVector>        _oriStrokeLines;    // original

    // spines
    std::vector<AVector>        _spineLines;       // resampled and simplified from strokeLines
    QOpenGLBuffer               _spineLinesVbo;
    QOpenGLVertexArrayObject    _spineLinesVao;    

    // left lines of the strokes. I need these lines to calculate closest points on the borders
    std::vector<AVector>        _leftLines;
    QOpenGLBuffer               _leftLinesVbo;
    QOpenGLVertexArrayObject    _leftLinesVao;

    // right lines of the stroke. I need these lines to calculate closest points on the borders
    std::vector<AVector>        _rightLines;
    QOpenGLBuffer               _rightLinesVbo;
    QOpenGLVertexArrayObject    _rightLinesVao;

    // debugging
    std::vector<AVector>        _debugPoints;
    QOpenGLBuffer               _debugPointsVbo;
    QOpenGLVertexArrayObject    _debugPointsVao;

    // debugging
    /*std::vector<ALine>          _debugLines;
    QOpenGLBuffer               _debugLinesVbo;
    QOpenGLVertexArrayObject    _debugLinesVao;*/

    //
    /*std::vector<ALine>          _junctionRibLines;
    QOpenGLBuffer               _junctionRibLinesVbo;
    QOpenGLVertexArrayObject    _junctionRibLinesVao;*/

    // points that cannot move at all
    /*int _numConstrainedPoints;
    QOpenGLBuffer               _constrainedPointsVbo;
    QOpenGLVertexArrayObject    _constrainedPointsVao;*/


private:
    AVector GetClosestPointFromBorders(AVector pt);
    AVector GetClosestPointFromBorders(QuadMesh qMesh, AVector pt);
    //AVector GetClosestPointFromMiddleVerticalLines(AVector pt);
    AVector GetClosestPointFromStrokeLines(AVector pt);
    AVector GetClosestPointFromStrokePoints(AVector pt);

    void CalculateInitialRibbon();
    void CalculateLeftRightLines();
    void CalculateKitesAndRectangles();
    void CalculateSpines();
    //void CalculateInitialRibbon2();


};

#endif // STROKEPAINTER_H

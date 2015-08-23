#ifndef STROKEPAINTER_H
#define STROKEPAINTER_H

#include "ALine.h"
#include "AVector.h"
#include "VertexData.h"
#include "PlusSignVertex.h"
#include "QuadMesh.h"

#include "VertexDataHelper.h"
#include "ConformalMapping.h"

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

    void Draw();

    void ConformalMappingOneStepSimple();
    void ConformalMappingOneStep();

    void CalculateVertices();
    void CalculateVertices(QuadMesh *qMesh);

    int QuadMeshSize() { return _quadMeshes.size(); }
    float IterationDelta() { return _cMapping->GetIterDist(); }
    bool ShouldStop() { return _cMapping->GetIterDist() < std::numeric_limits<float>::epsilon(); }

    void SetStrokeTexture(QString img);
    void SetCornerTexture(QString img);

    void SetVertexDataHelper(QOpenGLShaderProgram* shaderProgram);

private:

    bool _isMouseDown;

    VertexDataHelper* _vDataHelper;
    ConformalMapping* _cMapping;

    // Meshes
    std::vector<QuadMesh>       _quadMeshes;
    int                         _qMeshNumData;
    QOpenGLBuffer               _quadMeshesVbo;
    QOpenGLVertexArrayObject    _quadMeshesVao;

    // texture
    std::vector<QImage>          _masterImages;
    std::vector<QOpenGLTexture*> _masterTextures;
    std::vector<int>             _qmTexNumbers;
    std::vector<QOpenGLBuffer>             _qmTexVbos;
    std::vector<QOpenGLVertexArrayObject>  _qmTexVaos;

    // interactive editing
    int _selectedIndex;
    float _maxDist;
    QOpenGLBuffer               _selectedPointVbo;
    QOpenGLVertexArrayObject    _selectedPointVao;

    // strokes
    std::vector<AVector>        _oriStrokeLines;
    QOpenGLBuffer               _oriStrokeLinesVbo;
    QOpenGLVertexArrayObject    _oriStrokeLinesVao;

    // middle spines
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

    // constrained points
    std::vector<AVector>        _constrainedPoints;
    QOpenGLBuffer               _constrainedPointsVbo;
    QOpenGLVertexArrayObject    _constrinedPointsVao;


private:
    void CalculateInitialRibbon();
    void CalculateLeftRightLines();
    void CalculateKitesAndRectangles();
    void CalculateSpines();

    AVector GetClosestPointFromLeftRightLines(AVector pt);
    AVector GetClosestPointFromSpineLines(AVector pt);
    AVector GetClosestPointFromSpinePoints(AVector pt);
    int GetClosestIndexFromSpinePoints(AVector pt, float maxDist);


};

#endif // STROKEPAINTER_H

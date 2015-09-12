#ifndef STROKEPAINTER_H
#define STROKEPAINTER_H

#include "ALine.h"
#include "AVector.h"
#include "VertexData.h"
#include "PlusSignVertex.h"
#include "QuadMesh.h"

#include "SystemParams.h"

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

    void CalculateInitialRibbon();          // shouln't be public
    void CalculateInitialLeftRightLines();  // shouln't be public
    void CalculateInitialSegments();        // shouln't be public
    void DecomposeSegments();
    void CalculateSpines();                 // shouln't be public

    void ConformalMappingOneStepSimple();
    void ConformalMappingOneStep();
    void MappingInterpolation();

    void CalculateVertices();

    // illustrator style
    void CalculateLinearVertices(QuadMesh *qMesh);

    // meshes are separated to each other    
    void CalculateVertices1(QuadMesh *qMesh);

    // meshes are connected during a conformal mapping computation
    void CalculateVertices2(QuadMesh* prevQMesh, QuadMesh* curQMesh, QuadMesh* nextQMesh);

    int   QuadMeshSize()   { return _quadMeshes.size(); }
    float IterationDelta() { return _cMapping->GetIterDist(); }
    bool  ShouldStop()     { return _cMapping->GetIterDist() < SystemParams::iter_threshold /*std::numeric_limits<float>::epsilon()*/; }

    void SetLegTexture(QString img);
    void SetKiteTexture(QString img);
    void SetRectilinearTexture(QString img);

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
    QVector3D                   _rectMeshesColor;
    QVector3D                   _legMeshesColor;
    QVector3D                   _kiteMeshesColor;

    // textures
    std::vector<QImage>                    _images;          // texture images
    std::vector<QOpenGLTexture*>           _oglTextures;     // OpenGL textures
    std::vector<int>                       _vertexNumbers;   // number of vertices which use a certain texture
    std::vector<QSizeF>                    _textureSizes;   // (width = length, height = width) should be scaled to SystemParams::stroke_width
    std::vector<QOpenGLBuffer>             _texVbos;         // VBOs of textures
    std::vector<QOpenGLVertexArrayObject>  _texVaos;         // VAOs of textures

    // interactive editing
    int _selectedIndex;
    float _maxDist;
    QOpenGLBuffer               _selectedPointVbo;
    QOpenGLVertexArrayObject    _selectedPointVao;
    QVector3D                   _selectedPointColor;
    QVector3D                   _unselectedPointColor;

    // strokes
    std::vector<AVector>        _oriStrokeLines;
    QOpenGLBuffer               _oriStrokeLinesVbo;
    QOpenGLVertexArrayObject    _oriStrokeLinesVao;
    QVector3D                   _oriStrokeColor;

    // middle spines
    std::vector<AVector>        _spineLines;       // resampled and simplified from strokeLines
    QOpenGLBuffer               _spineLinesVbo;
    QOpenGLVertexArrayObject    _spineLinesVao;
    QVector3D                   _spineLinesColor;

    // left lines of the strokes. I need these lines to calculate closest points on the borders
    std::vector<AVector>        _leftLines;
    QOpenGLBuffer             _leftLinesVbo;
    QOpenGLVertexArrayObject  _leftLinesVao;

    // right lines of the stroke. I need these lines to calculate closest points on the borders
    std::vector<AVector>        _rightLines;
    QOpenGLBuffer             _rightLinesVbo;
    QOpenGLVertexArrayObject  _rightLinesVao;

    // constrained points
    std::vector<AVector>        _constrainedPoints;
    QOpenGLBuffer               _constrainedPointsVbo;
    QOpenGLVertexArrayObject    _constrainedPointsVao;
    QVector3D                   _constrainedPointColor;

    // debug points
    std::vector<AVector>        _debugPoints;
    QOpenGLBuffer               _debugPointsVbo;
    QOpenGLVertexArrayObject    _debugPointsVao;

    // debug lines
    std::vector<ALine>          _debugLines;
    QOpenGLBuffer               _debugLinesVbo;
    QOpenGLVertexArrayObject    _debugLinesVao;

private:
    AVector GetClosestPointFromLeftRightLines(AVector pt);
    AVector GetClosestPointFromSpineLines(AVector pt);
    AVector GetClosestPointFromSpinePoints(AVector pt);
    int     GetClosestIndexFromSpinePoints(AVector pt, float maxDist);
};

#endif // STROKEPAINTER_H

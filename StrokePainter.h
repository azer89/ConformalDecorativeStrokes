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

/**
  * Reza Adhitya Saputra
  * radhitya@uwaterloo.ca
  */

class StrokePainter
{
public:
    StrokePainter();
    ~StrokePainter();

    /**
     * Mouse events
     */
    void mousePressEvent(float x, float y);
    void mouseMoveEvent(float x, float y);
    void mouseReleaseEvent(float x, float y);

    void Draw();

    void CalculateInitialRibbon();          // shouldn't be public
    void CalculateInitialLeftRightLines();  // shouldn't be public
    void CalculateInitialSegments();        // shouldn't be public
    void DecomposeSegments();
    void CalculateSpines();                 // shouldn't be public

    /**
     * Conformal mapping iteration
     */
    void ConformalMappingOneStepSimple();
    void ConformalMappingOneStep();
    void MappingInterpolation();

    /**
     * Calculate initial vertices before warping
     */
    void CalculateVertices();

    /**
     * Illustrator's linear warping
     */
    void CalculateLinearVertices(QuadMesh *qMesh);

    // Meshes are separated to each other
    //void CalculateVertices1(QuadMesh *qMesh);

    /**
     * Meshes are connected during a conformal mapping computation
     */
    void CalculateVertices(QuadMesh* prevQMesh, QuadMesh* curQMesh, QuadMesh* nextQMesh);

    /**
     * Info
     */
    int   QuadMeshSize()   { return _quadMeshes.size(); }
    float IterationDelta() { return _cMapping->GetIterDist(); }
    bool  ShouldStop()     { return _cMapping->GetIterDist() < SystemParams::iter_threshold /*std::numeric_limits<float>::epsilon()*/; }

    /**
     * Textures
     */
    void SetLeftLegTexture(QString img);
    void SetRightLegTexture(QString img);
    void SetKiteTexture(QString img);
    void SetRectilinearTexture(QString img);

    /**
     * VBO and VAO
     */
    void SetVertexDataHelper(QOpenGLShaderProgram* shaderProgram);

    /**
     * Sliding Constraints near the boundaries
     */
    void SelectSlidingConstraints(float x, float y);

private:
    /**
     * Sliding Constraints
     */
    void GenerateSlidingConstraintCandidates();

    std::vector<std::vector<AVector>> GetFilteredList(std::vector<std::vector<AVector>> candidates, std::vector<bool> mask);

    AVector GetClosestPointFromLeftRightLines(AVector pt);
    AVector GetClosestPointFromSpineLines(AVector pt);
    AVector GetClosestPointFromSpinePoints(AVector pt);
    int     GetClosestIndexFromSpinePoints(AVector pt, float maxDist);


private:
    bool _isMouseDown;

    VertexDataHelper* _vDataHelper;
    ConformalMapping* _cMapping;

    // Meshes
    std::vector<QuadMesh>       _quadMeshes;
    int                         _qMeshNumData;
    QOpenGLBuffer               _quadMeshVbo;
    QOpenGLVertexArrayObject    _quadMeshVao;
    QVector3D                   _rectMeshColor;
    QVector3D                   _lLegMeshColor;
    QVector3D                   _rLegMeshColor;
    QVector3D                   _kiteMeshColor;

    // textures
    std::vector<QImage>                    _images;          // texture images
    std::vector<QOpenGLTexture*>           _oglTextures;     // OpenGL textures
    std::vector<int>                       _vertexNumbers;   // number of vertices which use a certain texture
    std::vector<QSizeF>                    _textureSizes;    // (width = length, height = width) should be scaled to SystemParams::stroke_width
    std::vector<QOpenGLBuffer>             _texVbos;         // VBOs of textures
    std::vector<QOpenGLVertexArrayObject>  _texVaos;         // VAOs of textures

    // sliding constraints
    std::vector<std::vector<AVector>> _sConstraintCandidates;
    QOpenGLBuffer                     _sConstraintCandVbo;
    std::vector<bool>                 _sConstraintMask;
    QOpenGLBuffer                     _sConstraintVbo;
    QOpenGLVertexArrayObject          _sConstraintVao;
    int                               _sConstraintNumData;

    // interactive editing
    int _selectedIndex;
    float _maxDist;
    QOpenGLBuffer               _selectedPointVbo;
    QOpenGLVertexArrayObject    _selectedPointVao;
    QVector3D                   _selectedPointColor;
    QVector3D                   _unselectedPointColor;

    // strokes
    std::vector<AVector>        _oriStrokeLines;
    QOpenGLBuffer               _oriStrokeLineVbo;
    QOpenGLVertexArrayObject    _oriStrokeLineVao;
    QVector3D                   _oriStrokeColor;

    // middle spines
    std::vector<AVector>        _spineLines;       // resampled and simplified from strokeLines
    QOpenGLBuffer               _spineLineVbo;
    QOpenGLVertexArrayObject    _spineLineVao;
    QVector3D                   _spineLineColor;

    // left lines of the strokes. I need these lines to calculate closest points on the borders
    std::vector<AVector>        _leftLines;
    QOpenGLBuffer             _leftLineVbo;
    QOpenGLVertexArrayObject  _leftLineVao;

    // right lines of the stroke. I need these lines to calculate closest points on the borders
    std::vector<AVector>        _rightLines;
    QOpenGLBuffer               _rightLineVbo;
    QOpenGLVertexArrayObject    _rightLineVao;

    // constrained points
    std::vector<AVector>        _constrainedPoints;
    QOpenGLBuffer               _constrainedPointVbo;
    QOpenGLVertexArrayObject    _constrainedPointVao;
    QVector3D                   _constrainedPointColor;

    // debug points
    std::vector<AVector>        _debugPoints;
    QOpenGLBuffer               _debugPointVbo;
    QOpenGLVertexArrayObject    _debugPointVao;
    QVector3D                   _debugPointColor;

    // debug lines
    std::vector<ALine>          _debugLines;
    QOpenGLBuffer               _debugLineVbo;
    QOpenGLVertexArrayObject    _debugLineVao;

};

#endif // STROKEPAINTER_H

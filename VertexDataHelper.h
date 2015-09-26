#ifndef VERTEXDATAHELPER_H
#define VERTEXDATAHELPER_H

#include "AVector.h"
#include "ALine.h"
#include "VertexData.h"
#include "PlusSignVertex.h"
#include "QuadMesh.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

class VertexDataHelper
{
public:
    VertexDataHelper(QOpenGLShaderProgram* shaderProgram);
    ~VertexDataHelper();

    // this function is awkward, should not be put in this class...
    void NeedToDrawWithColor(GLfloat num) { _shaderProgram->setUniformValue(_useColorLocation, num); }

    // these functions below are pretty standard
    void BuildLinesVertexData(std::vector<AVector> points, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol);
    void BuildLinesVertexData(std::vector<std::vector<AVector>> lines, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao, int &numData);
    void BuildLinesVertexData(std::vector<std::vector<AVector>> lines, std::vector<bool> constraintMask, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao, int &numData);
    void BuildLinesVertexData(std::vector<ALine> lines, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol);
    void BuildLinesVertexData(std::vector<ALine> lines, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol1, QVector3D vecCol2);
    void BuildPointsVertexData(std::vector<AVector> points, QOpenGLBuffer* ptsVbo, QOpenGLVertexArrayObject* ptsVao, QVector3D vecCol);
    void BuildVboWithColor(QVector<VertexData> data, QOpenGLBuffer* vbo);

    // selected point of the spine line
    void BuildPointsVertexData(std::vector<AVector> points, QOpenGLBuffer* ptsVbo, QOpenGLVertexArrayObject* ptsVao, int selectedIndex, QVector3D selectedCol, QVector3D unselectedCol);

    // three functions below needs mesh width and mesh height information    
    void BuildLinesVertexData(std::vector<std::vector<PlusSignVertex>> plusSignVertices, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, int mesh_width, int mesh_height, QVector3D vecCol);
    void BuildTexturedStrokeVertexData(std::vector<std::vector<PlusSignVertex>> plusSignVertices, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao, int mesh_width, int mesh_height);
    void BuildConstrainedPointsVertexData(std::vector<std::vector<PlusSignVertex>> plusSignVertices, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao, int *numConstrainedPoints, int mesh_width, int mesh_height, QVector3D vecCol);

    // Quad Mesh
    void BuildLinesVertexData(std::vector<QuadMesh> quadMeshes,
                              QOpenGLBuffer* linesVbo,
                              QOpenGLVertexArrayObject* linesVao,
                              int& qMeshNumData,
                              QVector3D vecCol1,
                              QVector3D vecCol2,
                              QVector3D vecCol3);
    void BuildTexturedStrokeVertexData(std::vector<QuadMesh> quadMeshes, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao, int& qMeshTexNumData, QuadMeshType qmType);

private:
    // to do: the function name is ugly
    QVector<VertexData> SpecificTypeOfMesh(std::vector<QuadMesh> quadMeshes, QuadMeshType qType, QVector3D vecCol);
    void ConcatQVector(QVector<VertexData>& destData, QVector<VertexData>& sourceData);

private:
    QOpenGLShaderProgram* _shaderProgram;
    int                   _colorLocation;
    int                   _vertexLocation;
    int                   _useColorLocation;
    int                   _texCoordLocation;
};

#endif // VERTEXDATAHELPER_H

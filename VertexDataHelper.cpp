
#include "VertexDataHelper.h"

#include "UtilityFunctions.h"
#include "SystemParams.h"
//#include <utility>

// Clamping
// shamelessly taken from http://stackoverflow.com/questions/9323903/most-efficient-elegant-way-to-clip-a-number
template <typename T>
T clip(const T& n, const T& lower, const T& upper)
{
    return std::max(lower, std::min(n, upper));
}

VertexDataHelper::VertexDataHelper(QOpenGLShaderProgram* shaderProgram)
{
    this->_shaderProgram    = shaderProgram;
    this->_colorLocation    = _shaderProgram->attributeLocation("vertexColor");
    this->_vertexLocation   = _shaderProgram->attributeLocation("vert");
    this->_useColorLocation = _shaderProgram->uniformLocation("use_color");
    this->_texCoordLocation = _shaderProgram->attributeLocation("uv");
}

VertexDataHelper::~VertexDataHelper()
{
}

void VertexDataHelper::BuildLinesVertexData(std::vector<std::vector<AVector>> lines, std::vector<bool> constraintMask, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao, int &numData)
{
    if(lines.size() == 0) return;

    bool isInit = false;
    if(!vao->isCreated())
    {
        vao->create();
        vao->bind();
        isInit = true;
    }

    QVector<VertexData> data;
    for(uint a = 0; a < lines.size(); a++)
    {
        if(!constraintMask[a]) { continue; }

        QVector3D rCol((float)(rand() % 255) / 255.0,
                       (float)(rand() % 255) / 255.0,
                       (float)(rand() % 255) / 255.0);
        std::vector<AVector> line = lines[a];
        for(uint b = 0; b < line.size() - 1; b++)
        {
            data.append(VertexData(QVector3D(line[b].x, line[b].y,  0),       QVector2D(), rCol));
            data.append(VertexData(QVector3D(line[b+1].x, line[b + 1].y,  0), QVector2D(), rCol));
        }

    }
    numData = data.size();

    BuildVboWithColor(data, vbo);

    if(isInit) { vao->release(); }
}

void VertexDataHelper::BuildLinesVertexData(std::vector<std::vector<AVector>> lines, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao, int& numData)
{
    if(lines.size() == 0) return;

    bool isInit = false;
    if(!vao->isCreated())
    {
        vao->create();
        vao->bind();
        isInit = true;
    }

    QVector<VertexData> data;
    for(uint a = 0; a < lines.size(); a++)
    {
        QVector3D rCol((float)(rand() % 255) / 255.0,
                       (float)(rand() % 255) / 255.0,
                       (float)(rand() % 255) / 255.0);
        std::vector<AVector> line = lines[a];
        for(uint b = 0; b < line.size() - 1; b++)
        {
            data.append(VertexData(QVector3D(line[b].x, line[b].y,  0),       QVector2D(), rCol));
            data.append(VertexData(QVector3D(line[b+1].x, line[b + 1].y,  0), QVector2D(), rCol));
        }
    }
    numData = data.size();

    BuildVboWithColor(data, vbo);

    if(isInit) { vao->release(); }
}

void VertexDataHelper::BuildLinesVertexData(std::vector<AVector> points, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol)
{
    if(points.size() == 0) return;

    bool isInit = false;
    if(!linesVao->isCreated())
    {
        linesVao->create();
        linesVao->bind();
        isInit = true;
    }

    QVector<VertexData> data;
    for(uint a = 0; a < points.size() - 1; a ++)
    {
        data.append(VertexData(QVector3D(points[a].x, points[a].y,  0), QVector2D(), vecCol));
        data.append(VertexData(QVector3D(points[a+1].x, points[a+1].y,  0), QVector2D(), vecCol));
    }

    BuildVboWithColor(data, linesVbo);

    if(isInit) { linesVao->release(); }
}

// is this function wrong on pass by reference ?
void VertexDataHelper::BuildConstrainedPointsVertexData(std::vector<std::vector<PlusSignVertex>> plusSignVertices, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao, int *numConstrainedPoints, int mesh_width, int mesh_height, QVector3D vecCol)
{
    if(plusSignVertices.size() == 0) return;

    bool isInit = false;
    if(!vao->isCreated())
    {
        vao->create();
        vao->bind();
        isInit = true;
    }

    numConstrainedPoints = 0;
    QVector<VertexData> data;
    for(int a = 0; a < mesh_width; a++)
    {
        for(int b = 0; b < mesh_height; b++)
        {
            PlusSignVertex psVertex = plusSignVertices[a][b];
            if(!psVertex.shouldMove)
            {
                data.append(VertexData(QVector3D(psVertex.position.x, psVertex.position.y,  0), QVector2D(), vecCol));
                numConstrainedPoints++;
            }
        }
    }

    BuildVboWithColor(data, vbo);

    if(isInit) { vao->release(); }

}

void VertexDataHelper::BuildTexturedStrokeVertexData(std::vector<std::vector<PlusSignVertex>> plusSignVertices, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao, int mesh_width, int mesh_height)
{
    if(plusSignVertices.size() == 0) return;

    bool isInit = false;
    if(!vao->isCreated())
    {
        vao->create();
        vao->bind();
        isInit = true;
    }

    int heightMinOne = mesh_height - 1;

    QVector<VertexData> data;
    for(int a = 0; a < mesh_width - 1; a++)
    {
        float aNumerator = a % heightMinOne;

        float xCoord1 = (aNumerator) / (float)heightMinOne;
        float xCoord2 = (aNumerator + 1.0f) / (float)heightMinOne;

        for(int b = 0; b < mesh_height - 1; b++)
        {
            float yCoord1 = (float)b / (float)heightMinOne;
            float yCoord2 = (float)(b+1) / (float)heightMinOne;

            AVector aVec = plusSignVertices[a][b].position;
            AVector bVec = plusSignVertices[a+1][b].position;
            AVector cVec = plusSignVertices[a+1][b+1].position;
            AVector dVec = plusSignVertices[a][b+1].position;

            data.append(VertexData(QVector3D(aVec.x, aVec.y,  0), QVector2D(xCoord1, yCoord1)));
            data.append(VertexData(QVector3D(bVec.x, bVec.y,  0), QVector2D(xCoord2, yCoord1)));
            data.append(VertexData(QVector3D(cVec.x, cVec.y,  0), QVector2D(xCoord2, yCoord2)));
            data.append(VertexData(QVector3D(dVec.x, dVec.y,  0), QVector2D(xCoord1, yCoord2)));
        }
    }

    if(!vbo->isCreated()) { vbo->create(); }
    vbo->bind();
    vbo->allocate(data.data(), data.size() * sizeof(VertexData));

    quintptr offset = 0;

    // vertex
    _shaderProgram->enableAttributeArray(_vertexLocation);
    _shaderProgram->setAttributeBuffer(_vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    offset += sizeof(QVector3D);

    // uv
    _shaderProgram->enableAttributeArray(_texCoordLocation);
    _shaderProgram->setAttributeBuffer(_texCoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

    if(isInit) { vao->release(); }
}


void VertexDataHelper::BuildLinesVertexData(std::vector<ALine> lines, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol1, QVector3D vecCol2)
{
    bool isInit = false;
    if(!linesVao->isCreated())
    {
        linesVao->create();
        linesVao->bind();
        isInit = true;
    }

    QVector<VertexData> data;
    for(uint a = 0; a < lines.size(); a++)
    {
        data.append(VertexData(QVector3D(lines[a].XA, lines[a].YA,  0), QVector2D(), vecCol1));
        data.append(VertexData(QVector3D(lines[a].XB, lines[a].YB,  0), QVector2D(), vecCol2));
    }

    BuildVboWithColor(data, linesVbo);

    if(isInit) { linesVao->release(); }
}

void VertexDataHelper::BuildLinesVertexData(std::vector<ALine> lines, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol)
{
    bool isInit = false;
    if(!linesVao->isCreated())
    {
        linesVao->create();
        linesVao->bind();
        isInit = true;
    }

    QVector<VertexData> data;
    for(uint a = 0; a < lines.size(); a++)
    {
        data.append(VertexData(QVector3D(lines[a].XA, lines[a].YA,  0), QVector2D(), vecCol));
        data.append(VertexData(QVector3D(lines[a].XB, lines[a].YB,  0), QVector2D(), vecCol));
    }

    BuildVboWithColor(data, linesVbo);

    if(isInit) { linesVao->release(); }
}

void VertexDataHelper::BuildTexturedStrokeVertexData(std::vector<QuadMesh> quadMeshes,
                                                     QOpenGLBuffer* vbo,
                                                     QOpenGLVertexArrayObject* vao,
                                                     int& qMeshTexNumData,
                                                     QuadMeshType qmType)
{
    bool isInit = false;
    if(!vao->isCreated())
    {
        vao->create();
        vao->bind();
        isInit = true;
    }

    // do what you want
    QVector<VertexData> data;
    for(uint iter = 0; iter < quadMeshes.size(); iter++)
    {
        QuadMesh* prevQMesh = 0;
        QuadMesh* curQMesh = &quadMeshes[iter];
        QuadMesh* nextQMesh = 0;

        if(iter > 0) { prevQMesh = &quadMeshes[iter - 1]; }
        if(iter < quadMeshes.size() - 1) { nextQMesh = &quadMeshes[iter + 1]; }

        if(curQMesh->_quadMeshType != qmType){ continue; }

        std::vector<std::vector<PlusSignVertex>> plusSignVertices = curQMesh->_psVertices;
        int mesh_width = plusSignVertices.size();
        int mesh_height = plusSignVertices[0].size();

        int textureHeight = mesh_height - 1;
        int textureWidth = (int)(curQMesh->_textureLength / SystemParams::grid_cell_size);

        for(int a = 0; a < mesh_width - 1; a++)
        {
            float aNumerator = a % textureWidth;

            float xCoord1 = clip((aNumerator) / (float)textureWidth, 0.0f, 1.0f);
            float xCoord2 = clip((aNumerator + 1.0f) / (float)textureWidth, 0.0f, 1.0f);

            for(int b = 0; b < mesh_height - 1; b++)
            {
                float yCoord1 = clip((float)b / (float)textureHeight, 0.0f, 1.0f);
                float yCoord2 = clip((float)(b + 1) / (float)textureHeight, 0.0f, 1.0f);

                QVector2D uv1;
                QVector2D uv2;
                QVector2D uv3;
                QVector2D uv4;

                // to do: clipping doesn't work !

                if(curQMesh->_quadMeshType == QuadMeshType::MESH_KITE && !curQMesh->_isRightKite)
                {
                    // a left kite
                    uv1 = QVector2D(xCoord1, clip(1.0f - yCoord1, 0.0f, 1.0f));
                    uv2 = QVector2D(xCoord2, clip(1.0f - yCoord1, 0.0f, 1.0f));
                    uv3 = QVector2D(xCoord2, clip(1.0f - yCoord2, 0.0f, 1.0f));
                    uv4 = QVector2D(xCoord1, clip(1.0f - yCoord2, 0.0f, 1.0f));
                }
                else if(curQMesh->_quadMeshType == QuadMeshType::MESH_KITE && curQMesh->_isRightKite)
                {
                    // a right kite
                    uv1 = QVector2D(clip(1.0f - xCoord1, 0.0f, 1.0f), yCoord1);
                    uv2 = QVector2D(clip(1.0f - xCoord2, 0.0f, 1.0f), yCoord1);
                    uv3 = QVector2D(clip(1.0f - xCoord2, 0.0f, 1.0f), yCoord2);
                    uv4 = QVector2D(clip(1.0f - xCoord1, 0.0f, 1.0f), yCoord2);
                }
                else if(curQMesh->_quadMeshType == QuadMeshType::MESH_LEG &&
                        prevQMesh && prevQMesh->_quadMeshType == QuadMeshType::MESH_KITE &&
                        prevQMesh->_isRightKite)
                {
                    // previous neighbour is a right kite
                    uv1 = QVector2D(xCoord1, yCoord1);
                    uv2 = QVector2D(xCoord2, yCoord1);
                    uv3 = QVector2D(xCoord2, yCoord2);
                    uv4 = QVector2D(xCoord1, yCoord2);
                }
                else if(curQMesh->_quadMeshType == QuadMeshType::MESH_LEG &&
                        prevQMesh && prevQMesh->_quadMeshType == QuadMeshType::MESH_KITE &&
                        !prevQMesh->_isRightKite)
                {
                    // previous neighbor is a left kite
                    uv1 = QVector2D(xCoord1, clip(1.0f - yCoord1, 0.0f, 1.0f));
                    uv2 = QVector2D(xCoord2, clip(1.0f - yCoord1, 0.0f, 1.0f));
                    uv3 = QVector2D(xCoord2, clip(1.0f - yCoord2, 0.0f, 1.0f));
                    uv4 = QVector2D(xCoord1, clip(1.0f - yCoord2, 0.0f, 1.0f));
                }
                else if(curQMesh->_quadMeshType == QuadMeshType::MESH_LEG &&
                        nextQMesh && nextQMesh->_quadMeshType == QuadMeshType::MESH_KITE &&
                        nextQMesh->_isRightKite)
                {
                    // next neighbor is a right kite
                    uv1 = QVector2D(clip(1.0f - xCoord1, 0.0f, 1.0f), yCoord1);
                    uv2 = QVector2D(clip(1.0f - xCoord2, 0.0f, 1.0f), yCoord1);
                    uv3 = QVector2D(clip(1.0f - xCoord2, 0.0f, 1.0f), yCoord2);
                    uv4 = QVector2D(clip(1.0f - xCoord1, 0.0f, 1.0f), yCoord2);
                }
                else if(curQMesh->_quadMeshType == QuadMeshType::MESH_LEG &&
                        nextQMesh && nextQMesh->_quadMeshType == QuadMeshType::MESH_KITE &&
                        !nextQMesh->_isRightKite)
                {
                    // next neightbor is a left kite
                    uv1 = QVector2D(clip(1.0f - xCoord1, 0.0f, 1.0f), clip(1.0f - yCoord1, 0.0f, 1.0f));
                    uv2 = QVector2D(clip(1.0f - xCoord2, 0.0f, 1.0f), clip(1.0f - yCoord1, 0.0f, 1.0f));
                    uv3 = QVector2D(clip(1.0f - xCoord2, 0.0f, 1.0f), clip(1.0f - yCoord2, 0.0f, 1.0f));
                    uv4 = QVector2D(clip(1.0f - xCoord1, 0.0f, 1.0f), clip(1.0f - yCoord2, 0.0f, 1.0f));
                }
                else if(curQMesh->_quadMeshType == QuadMeshType::MESH_RECTILINEAR)
                {
                    uv1 = QVector2D(xCoord1, yCoord1);
                    uv2 = QVector2D(xCoord2, yCoord1);
                    uv3 = QVector2D(xCoord2, yCoord2);
                    uv4 = QVector2D(xCoord1, yCoord2);
                }

                AVector aVec = plusSignVertices[a][b].position;
                AVector bVec = plusSignVertices[a+1][b].position;
                AVector cVec = plusSignVertices[a+1][b+1].position;
                AVector dVec = plusSignVertices[a][b+1].position;

                data.append(VertexData(QVector3D(aVec.x, aVec.y,  0), uv1));
                data.append(VertexData(QVector3D(bVec.x, bVec.y,  0), uv2));
                data.append(VertexData(QVector3D(cVec.x, cVec.y,  0), uv3));
                data.append(VertexData(QVector3D(dVec.x, dVec.y,  0), uv4));
            }
        }
    }

    qMeshTexNumData = data.size();

    if(!vbo->isCreated()) { vbo->create(); }
    vbo->bind();
    vbo->allocate(data.data(), data.size() * sizeof(VertexData));

    quintptr offset = 0;
    // vertex
    _shaderProgram->enableAttributeArray(_vertexLocation);
    _shaderProgram->setAttributeBuffer(_vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    offset += sizeof(QVector3D);

    // uv
    _shaderProgram->enableAttributeArray(_texCoordLocation);
    _shaderProgram->setAttributeBuffer(_texCoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

    if(isInit) { vao->release(); }
}

void VertexDataHelper::BuildLinesVertexData(std::vector<QuadMesh> quadMeshes,
                                            QOpenGLBuffer* linesVbo,
                                            QOpenGLVertexArrayObject* linesVao,
                                            int &qMeshNumData,
                                            QVector3D vecCol1,
                                            QVector3D vecCol2,
                                            QVector3D vecCol3)
{
    qMeshNumData = 0;
    if(quadMeshes.size() == 0) { return; }

    bool isInit = false;
    if(!linesVao->isCreated())
    {
        linesVao->create();
        linesVao->bind();
        isInit = true;
    }

    QVector<VertexData> data;

    QVector<VertexData> kiteData = SpecificTypeOfMesh(quadMeshes, QuadMeshType::MESH_KITE,        vecCol2);
    QVector<VertexData> legData =  SpecificTypeOfMesh(quadMeshes, QuadMeshType::MESH_LEG,         vecCol3);
    QVector<VertexData> rectData = SpecificTypeOfMesh(quadMeshes, QuadMeshType::MESH_RECTILINEAR, vecCol1);

    ConcatQVector(data, kiteData);
    ConcatQVector(data, legData);
    ConcatQVector(data, rectData);

    qMeshNumData = data.size();
    BuildVboWithColor(data, linesVbo);
    if(isInit) { linesVao->release(); }
}

void VertexDataHelper::ConcatQVector(QVector<VertexData>& destData, QVector<VertexData>& sourceData)
{
    for(uint a = 0; a < sourceData.size(); a++)
    {
        destData.append(sourceData[a]);
    }
}

QVector<VertexData> VertexDataHelper::SpecificTypeOfMesh(std::vector<QuadMesh> quadMeshes, QuadMeshType qType, QVector3D vecCol)
{
     QVector<VertexData> data;

     for(uint iter = 0; iter < quadMeshes.size(); iter++)
     {
         QuadMesh qMesh = quadMeshes[iter];

         if(qMesh._quadMeshType != qType)
             { continue; }

         int mesh_width = qMesh._psVertices.size();
         int mesh_height = qMesh._psVertices[0].size();
         std::vector<std::vector<PlusSignVertex>> plusSignVertices = qMesh._psVertices;

         for(int a = 0; a < mesh_width - 1; a++)
         {
             for(int b = 0; b < mesh_height - 1; b++)
             {
                 AVector aVec = plusSignVertices[a][b].position;
                 AVector bVec = plusSignVertices[a+1][b].position;
                 AVector cVec = plusSignVertices[a+1][b+1].position;
                 AVector dVec = plusSignVertices[a][b+1].position;

                 data.append(VertexData(QVector3D(aVec.x, aVec.y,  0), QVector2D(), vecCol));
                 data.append(VertexData(QVector3D(bVec.x, bVec.y,  0), QVector2D(), vecCol));

                 data.append(VertexData(QVector3D(dVec.x, dVec.y,  0), QVector2D(), vecCol));
                 data.append(VertexData(QVector3D(aVec.x, aVec.y,  0), QVector2D(), vecCol));

                 if(a == mesh_width - 2)
                 {
                     data.append(VertexData(QVector3D(bVec.x, bVec.y,  0), QVector2D(), vecCol));
                     data.append(VertexData(QVector3D(cVec.x, cVec.y,  0), QVector2D(), vecCol));
                 }

                 if(b == mesh_height - 2)
                 {
                     data.append(VertexData(QVector3D(cVec.x, cVec.y,  0), QVector2D(), vecCol));
                     data.append(VertexData(QVector3D(dVec.x, dVec.y,  0), QVector2D(), vecCol));
                 }
             }
         }
     }
     return data;
}

void VertexDataHelper::BuildLinesVertexData(std::vector<std::vector<PlusSignVertex>> plusSignVertices, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, int mesh_width, int mesh_height, QVector3D vecCol)
{
    if(plusSignVertices.size() == 0) return;

    bool isInit = false;
    if(!linesVao->isCreated())
    {
        linesVao->create();
        linesVao->bind();
        isInit = true;
    }

    QVector<VertexData> data;
    for(int a = 0; a < mesh_width - 1; a++)
    {
        for(int b = 0; b < mesh_height - 1; b++)
        {
            AVector aVec = plusSignVertices[a][b].position;
            AVector bVec = plusSignVertices[a+1][b].position;
            AVector cVec = plusSignVertices[a+1][b+1].position;
            AVector dVec = plusSignVertices[a][b+1].position;

            data.append(VertexData(QVector3D(aVec.x, aVec.y,  0), QVector2D(), vecCol));
            data.append(VertexData(QVector3D(bVec.x, bVec.y,  0), QVector2D(), vecCol));

            data.append(VertexData(QVector3D(dVec.x, dVec.y,  0), QVector2D(), vecCol));
            data.append(VertexData(QVector3D(aVec.x, aVec.y,  0), QVector2D(), vecCol));

            if(a == mesh_width - 2)
            {
                data.append(VertexData(QVector3D(bVec.x, bVec.y,  0), QVector2D(), vecCol));
                data.append(VertexData(QVector3D(cVec.x, cVec.y,  0), QVector2D(), vecCol));
            }

            if(b == mesh_height - 2)
            {
                data.append(VertexData(QVector3D(cVec.x, cVec.y,  0), QVector2D(), vecCol));
                data.append(VertexData(QVector3D(dVec.x, dVec.y,  0), QVector2D(), vecCol));
            }
        }
    }

    BuildVboWithColor(data, linesVbo);

    if(isInit) { linesVao->release(); }
}

void VertexDataHelper::BuildPointsVertexData(std::vector<AVector> points, QOpenGLBuffer* ptsVbo, QOpenGLVertexArrayObject* ptsVao, int selectedIndex, QVector3D selectedCol, QVector3D unselectedCol)
{
    bool isInit = false;
    if(!ptsVao->isCreated())
    {
        ptsVao->create();
        ptsVao->bind();
        isInit = true;
    }

    QVector<VertexData> data;
    for(uint a = 0; a < points.size(); a++)
    {
        QVector3D vecCol = unselectedCol;
        if(a == selectedIndex)
        {
            vecCol = selectedCol;
        }
        data.append(VertexData(QVector3D(points[a].x, points[a].y,  0), QVector2D(), vecCol));
    }

    BuildVboWithColor(data, ptsVbo);

    if(isInit) { ptsVao->release(); }
}

void VertexDataHelper::BuildPointsVertexData(std::vector<AVector> points, QOpenGLBuffer* ptsVbo, QOpenGLVertexArrayObject* ptsVao, QVector3D vecCol)
{
    bool isInit = false;
    if(!ptsVao->isCreated())
    {
        ptsVao->create();
        ptsVao->bind();
        isInit = true;
    }

    QVector<VertexData> data;
    for(uint a = 0; a < points.size(); a++)
        { data.append(VertexData(QVector3D(points[a].x, points[a].y,  0), QVector2D(), vecCol)); }

    BuildVboWithColor(data, ptsVbo);

    if(isInit) { ptsVao->release(); }
}

void VertexDataHelper::BuildVboWithColor(QVector<VertexData> data, QOpenGLBuffer* vbo)
{
    if(!vbo->isCreated()) { vbo->create(); }
    vbo->bind();
    vbo->allocate(data.data(), data.size() * sizeof(VertexData));

    quintptr offset = 0;

    _shaderProgram->enableAttributeArray(_vertexLocation);
    _shaderProgram->setAttributeBuffer(_vertexLocation, GL_FLOAT, 0, 3, sizeof(VertexData));

    offset += sizeof(QVector3D);
    offset += sizeof(QVector2D);

    _shaderProgram->enableAttributeArray(_colorLocation);
    _shaderProgram->setAttributeBuffer(_colorLocation, GL_FLOAT, offset, 3, sizeof(VertexData));
}



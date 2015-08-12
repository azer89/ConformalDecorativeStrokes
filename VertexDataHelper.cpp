
#include "VertexDataHelper.h"

VertexDataHelper::VertexDataHelper(QOpenGLShaderProgram* shaderProgram)
{
    this->_shaderProgram = shaderProgram;
    this->_colorLocation = _shaderProgram->attributeLocation("vertexColor");
    this->_vertexLocation = _shaderProgram->attributeLocation("vert");
    this->_use_color_location = _shaderProgram->uniformLocation("use_color");
}

VertexDataHelper::~VertexDataHelper()
{
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
        float aNominator = a % heightMinOne;

        float xCoord1 = (aNominator) / (float)heightMinOne;
        float xCoord2 = (aNominator + 1.0f) / (float)heightMinOne;

        for(int b = 0; b < mesh_height - 1; b++)
        {
            float yCoord1 = (float)b / (float)heightMinOne;
            float yCoord2 = (float)(b+1) / (float)heightMinOne;

            //std::cout << a << " " << b << " " << xCoord1 << " " << xCoord2 << " "<< yCoord1 << " " << yCoord2 << "\n";

            AVector aVec = plusSignVertices[a][b].position;
            AVector bVec = plusSignVertices[a+1][b].position;
            AVector cVec = plusSignVertices[a+1][b+1].position;
            AVector dVec = plusSignVertices[a][b+1].position;

            QVector2D aTexCoord(xCoord1, yCoord1);
            QVector2D bTexCoord(xCoord2, yCoord1);
            QVector2D cTexCoord(xCoord2, yCoord2);
            QVector2D dTexCoord(xCoord1, yCoord2);

            data.append(VertexData(QVector3D(aVec.x, aVec.y,  0), aTexCoord));
            data.append(VertexData(QVector3D(bVec.x, bVec.y,  0), bTexCoord));
            data.append(VertexData(QVector3D(cVec.x, cVec.y,  0), cTexCoord));
            data.append(VertexData(QVector3D(dVec.x, dVec.y,  0), dTexCoord));
        }
    }

    if(!vbo->isCreated()) { vbo->create(); }
    vbo->bind();
    vbo->allocate(data.data(), data.size() * sizeof(VertexData));

    //BuildVboWithColor(data, vbo);
    quintptr offset = 0;
    // vertex
    int vertexLocation = _shaderProgram->attributeLocation("vert");
    _shaderProgram->enableAttributeArray(vertexLocation);
    _shaderProgram->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    offset += sizeof(QVector3D);

    // uv
    int texcoordLocation = _shaderProgram->attributeLocation("uv");
    _shaderProgram->enableAttributeArray(texcoordLocation);
    _shaderProgram->setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

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
    {
        data.append(VertexData(QVector3D(points[a].x, points[a].y,  0), QVector2D(), vecCol));
    }

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


#include "StrokePainter.h"
#include "VertexData.h"
#include "CurveRDP.h"
#include "UtilityFunctions.h"
#include "SystemParams.h"

#include <stdlib.h>
#include <iostream>

StrokePainter::StrokePainter() :
    _mesh_width(0),
    _mesh_height(0)
{
}

StrokePainter::~StrokePainter()
{
}


void StrokePainter::CalculateInitialRibbon()
{
    //_points.clear();

    _lLines.clear();
    _rLines.clear();
    //_ribLines.clear();
    //_gridLines.clear();

    _strokeLines.clear();

    std::vector<AVector> tempLine;
    CurveRDP::SimplifyRDP(_oriStrokeLines, tempLine, SystemParams::rdp_epsilon);
    _strokeLines = std::vector<AVector>(tempLine);

    for(uint a = 0; a < _strokeLines.size(); a++)
    {
        if(a == 0)
        {
            AVector pt1 = _strokeLines[0];
            AVector pt2 = _strokeLines[1];
            AVector dirVec = (pt2 - pt1).Norm() * (SystemParams::stroke_width / 2.0f);

            AVector rightVec (-dirVec.y,  dirVec.x);
            AVector leftVec( dirVec.y, -dirVec.x);

            AVector lPoint = pt1 + leftVec;
            AVector rPoint = pt1 + rightVec;

            _lLines.push_back(lPoint);
            _rLines.push_back(rPoint);

            //_ribLines.push_back(ALine(lPoint, rPoint));
        }
        else if(_strokeLines.size() >= 3 && a <= _strokeLines.size() - 2)
        {
            ALine prevLine(_strokeLines[a-1], _strokeLines[a]);
            ALine curLine(_strokeLines[a], _strokeLines[a+1]);

            AVector lPoint, rPoint;

            UtilityFunctions::GetMiterJoints(prevLine, curLine,
                                             SystemParams::stroke_width / 2.0f, SystemParams::stroke_width / 2.0f,
                                             &lPoint, &rPoint);

            _lLines.push_back(lPoint);
            _rLines.push_back(rPoint);

            //_ribLines.push_back(ALine(lPoint, rPoint));
        }

        // add an end
        if(a == _strokeLines.size() - 2)
        {
            AVector pt1 = _strokeLines[a];
            AVector pt2 = _strokeLines[a + 1];
            AVector dirVec = (pt2 - pt1).Norm() * (SystemParams::stroke_width / 2.0f);

            AVector rightVec (-dirVec.y,  dirVec.x);
            AVector leftVec( dirVec.y, -dirVec.x);

            AVector lPoint = pt2 + leftVec;
            AVector rPoint = pt2 + rightVec;

            _lLines.push_back(lPoint);
            _rLines.push_back(rPoint);

            //_ribLines.push_back(ALine(lPoint, rPoint));
        }
    }

    //BuildLinesVertexData(_ribLines, &_ribLinesVbo, &_ribLinesVao, QVector3D(1, 0, 0));

    BuildLinesVertexData(_strokeLines, &_strokeLinesVbo, &_strokeLinesVao, QVector3D(0, 0, 0));
    BuildLinesVertexData(_lLines, &_lLinesVbo, &_lLinesVao, QVector3D(0, 0, 1));
    BuildLinesVertexData(_rLines, &_rLinesVbo, &_rLinesVao, QVector3D(1, 0, 0));

    //_points = std::vector<AVector>(_strokeLines);
    //BuildPointsVertexData(_points, &_pointsVbo, &_pointsVao, QVector3D(1, 0, 0));
}

void StrokePainter::CalculateVertices()
{
    _vertices.clear();
    _plusSignVertices.clear();

    for(int a = 0; a < _strokeLines.size() - 1; a++)
    {
        AVector mStartPt = _strokeLines[a];
        AVector mEndPt   = _strokeLines[a + 1];
        int intMeshWidth = mStartPt.Distance(mEndPt) / SystemParams::mesh_size;
        int intMeshHeight = SystemParams::stroke_width / SystemParams::mesh_size;

        AVector lStartPt = _lLines[a];
        AVector lEndPt   = _lLines[a + 1];

        AVector rStartPt = _rLines[a];
        AVector rEndPt   = _rLines[a + 1];

        AVector vVec = rStartPt - lStartPt;

        AVector uHVec = (lEndPt - lStartPt);
        AVector bHVec = (rEndPt - rStartPt);

        int xLoop = intMeshWidth;
        int yLoop = intMeshHeight + 1;

        if(a == _strokeLines.size() - 2)
        {
            xLoop++;
        }

        //int widthSum = 0;

        for(int xIter = 0; xIter < xLoop; xIter++)
        {
            std::vector<PlusSignVertex> columnVertices;

            for(int yIter = 0; yIter < yLoop;  yIter++)
            {
                float xFactor = (float)xIter / (float)intMeshWidth;
                float yFactor = (float)yIter / (float)intMeshHeight;

                AVector hVec = uHVec * (1.0f - yFactor) + bHVec * yFactor;

                AVector pt = lStartPt + vVec * yFactor;
                pt = pt + hVec * xFactor;

                _vertices.push_back(pt);
                columnVertices.push_back(PlusSignVertex(pt));
            }

            _plusSignVertices.push_back(columnVertices);
        }


        //std::cout << "_vertices.size() " << _vertices.size() << "\n";
    }

    _mesh_width = _plusSignVertices.size();
    _mesh_height = _plusSignVertices[0].size();

    //std::cout << _mesh_width << " " << _mesh_height << "\n";

    //BuildPointsVertexData(_vertices, &_verticesVbo, &_verticesVao, QVector3D(0, 0, 1));
    BuildLinesVertexData(_plusSignVertices, &_plusSignVerticesVbo, &_plusSignVerticesVao, QVector3D(1, 0, 0));

}

AVector StrokePainter::GetClosestPointFromBorders(AVector pt)
{
    AVector closestPt = pt;
    float dist = std::numeric_limits<float>::max();

    // left
    for(int a = 0; a < _lLines.size() - 1; a++)
    {
        AVector pt1 = _lLines[a];
        AVector pt2 = _lLines[a+1];
        AVector cPt = UtilityFunctions::GetClosestPoint(pt1, pt2, pt);
        if(pt.Distance(cPt) < dist)
        {
            dist = pt.Distance(cPt);
            closestPt = cPt;
        }
    }

    // right
    for(int a = 0; a < _rLines.size() - 1; a++)
    {
        AVector pt1 = _rLines[a];
        AVector pt2 = _rLines[a+1];
        AVector cPt = UtilityFunctions::GetClosestPoint(pt1, pt2, pt);
        if(pt.Distance(cPt) < dist)
        {
            dist = pt.Distance(cPt);
            closestPt = cPt;
        }
    }

    // start
    AVector s1 = _lLines[0];
    AVector s2 = _rLines[0];
    AVector scPt = UtilityFunctions::GetClosestPoint(s1, s2, pt);
    if(pt.Distance(scPt) < dist)
    {
        dist = pt.Distance(scPt);
        closestPt = scPt;
    }

    // end
    AVector e1 = _lLines[_lLines.size() - 1];
    AVector e2 = _rLines[_rLines.size() - 1];
    AVector ecPt = UtilityFunctions::GetClosestPoint(e1, e2, pt);
    if(pt.Distance(ecPt) < dist)
    {
        dist = pt.Distance(ecPt);
        closestPt = ecPt;
    }

    return closestPt;
}

void StrokePainter::ConformalMappingOneStep()
{
    //std::cout << "ConformalMappingIteration\n";
    std::vector<std::vector<PlusSignVertex>> tempVertices = _plusSignVertices;

    for(int a = 0; a < _mesh_width; a++)
    {
        for(int b = 0; b < _mesh_height; b++)
        {
            AVector avgPositions(0, 0);
            int numNeighbor = 0;

            // left
            if(a > 0)
            {
                PlusSignVertex lVertex = tempVertices[a - 1][b];
                avgPositions = avgPositions + lVertex.position;
                numNeighbor++;
            }

            // right
            if(a < _mesh_width - 1)
            {
                PlusSignVertex rVertex = tempVertices[a + 1][b];
                avgPositions = avgPositions + rVertex.position;
                numNeighbor++;
            }

            // up
            if(b > 0)
            {
                PlusSignVertex uVertex = tempVertices[a][b - 1];
                avgPositions = avgPositions + uVertex.position;
                numNeighbor++;
            }

            // bottom
            if(b < _mesh_height - 1)
            {
                PlusSignVertex bVertex = tempVertices[a][b + 1];
                avgPositions = avgPositions + bVertex.position;
                numNeighbor++;
            }

            avgPositions = avgPositions / (float)numNeighbor;

            if(numNeighbor < 4)
            {
                AVector closestPt = GetClosestPointFromBorders(avgPositions);
                tempVertices[a][b].position = closestPt;
            }
            else
            {
                tempVertices[a][b].position = avgPositions;
            }
        }
    }

    float sumDist = 0;
    for(int a = 0; a < _mesh_width; a++)
    {
        for(int b = 0; b < _mesh_height; b++)
        {
            AVector pt1 = _plusSignVertices[a][b].position;
            AVector pt2 = tempVertices[a][b].position;
            sumDist += pt1.Distance(pt2);
        }
    }
    _iterDist = sumDist;
    //std::cout << sumDist << "\n";


    _plusSignVertices = tempVertices;

    BuildLinesVertexData(_plusSignVertices, &_plusSignVerticesVbo, &_plusSignVerticesVao, QVector3D(1, 0, 0));

}

// mouse press
void StrokePainter::mousePressEvent(float x, float y)
{
    //_points.clear();
    _vertices.clear();
    _plusSignVertices.clear();

    _lLines.clear();
    _rLines.clear();

    //_ribLines.clear();
    //_gridLines.clear();

    _strokeLines.clear();
    _oriStrokeLines.clear();

    _oriStrokeLines.push_back(AVector(x, y));
}

// mouse move
void StrokePainter::mouseMoveEvent(float x, float y)
{
    _oriStrokeLines.push_back(AVector(x, y));
    _strokeLines = std::vector<AVector>(_oriStrokeLines);
    BuildLinesVertexData(_strokeLines, &_strokeLinesVbo, &_strokeLinesVao, QVector3D(0, 0, 0));
}

// mouse release
void StrokePainter::mouseReleaseEvent(float x, float y)
{
    //std::cout << "StrokePainter::mouseReleaseEvent\n";

    _oriStrokeLines.push_back(AVector(x, y));
    CalculateInitialRibbon();
    CalculateVertices();

    //std::cout << "_plusSignVertices.size() " << _plusSignVertices.size() << "\n";
}

void StrokePainter::Draw()
{
    _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);

    if(_plusSignVerticesVao.isCreated() && _plusSignVertices.size() > 0)
    {
        int wMin1 = _mesh_width - 1;
        int hMin1 = _mesh_height - 1;

        int meshSize = ((wMin1 * _mesh_width) + (hMin1 * _mesh_height)) * 2;

        glLineWidth(1.0f);
        _plusSignVerticesVao.bind();
        glDrawArrays(GL_LINES, 0, meshSize);
        _plusSignVerticesVao.release();
    }

    if(_strokeLinesVao.isCreated() && _plusSignVertices.size() == 0)
    {
        glLineWidth(2.0f);
        _strokeLinesVao.bind();
        glDrawArrays(GL_LINES, 0, (_strokeLines.size() - 1) * 2);
        _strokeLinesVao.release();
    }

    /*
    if(_pointsVao.isCreated())
    {
        glPointSize(10.0f);
        _pointsVao.bind();
        glDrawArrays(GL_POINTS, 0, _points.size());
        _pointsVao.release();
    }*/

    /*if(_verticesVao.isCreated())
    {
        glPointSize(10.0f);
        _verticesVao.bind();
        glDrawArrays(GL_POINTS, 0, _vertices.size());
        _verticesVao.release();
    }*/

    /*
    if(_lLinesVao.isCreated())
    {
        glLineWidth(2.0f);
        _lLinesVao.bind();
        glDrawArrays(GL_LINES, 0, (_lLines.size() - 1) * 2);
        _lLinesVao.release();
    }

    if(_rLinesVao.isCreated())
    {
        glLineWidth(2.0f);
        _rLinesVao.bind();
        glDrawArrays(GL_LINES, 0, (_rLines.size() - 1) * 2);
        _rLinesVao.release();
    }
    */


    /*
    if(_ribLinesVao.isCreated())
    {
        glLineWidth(2.0f);
        _ribLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _ribLines.size() * 2);
        _ribLinesVao.release();
    }

    if(_gridLinesVao.isCreated())
    {
        glLineWidth(2.0f);
        _gridLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _gridLines.size() * 2);
        _gridLinesVao.release();
    }
    */
}

void StrokePainter::BuildLinesVertexData(std::vector<AVector> points, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol)
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

    if(isInit)
    {
        linesVao->release();
    }
}

void StrokePainter::BuildLinesVertexData(std::vector<std::vector<PlusSignVertex>> plusSignVertices, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol)
{
    if(plusSignVertices.size() == 0) return;

    bool isInit = false;
    if(!linesVao->isCreated())
    {
        linesVao->create();
        linesVao->bind();
        isInit = true;
    }

    /*
    QVector<VertexData> data;
    for(int a = 0; a < _mesh_width; a++)
    {
        for(int b = 0; b < _mesh_height; b++)
        {
            AVector aVec = plusSignVertices[a][b].position;
            data.append(VertexData(QVector3D(aVec.x, aVec.y,  0), QVector2D(), vecCol));
        }
    }
    */
    QVector<VertexData> data;
    for(int a = 0; a < _mesh_width - 1; a++)
    {
        for(int b = 0; b < _mesh_height - 1; b++)
        {
            AVector aVec = plusSignVertices[a][b].position;
            AVector bVec = plusSignVertices[a+1][b].position;
            AVector cVec = plusSignVertices[a+1][b+1].position;
            AVector dVec = plusSignVertices[a][b+1].position;

            data.append(VertexData(QVector3D(aVec.x, aVec.y,  0), QVector2D(), vecCol));
            data.append(VertexData(QVector3D(bVec.x, bVec.y,  0), QVector2D(), vecCol));

            data.append(VertexData(QVector3D(dVec.x, dVec.y,  0), QVector2D(), vecCol));
            data.append(VertexData(QVector3D(aVec.x, aVec.y,  0), QVector2D(), vecCol));

            if(a == _mesh_width - 2)
            {
                data.append(VertexData(QVector3D(bVec.x, bVec.y,  0), QVector2D(), vecCol));
                data.append(VertexData(QVector3D(cVec.x, cVec.y,  0), QVector2D(), vecCol));
            }

            if(b == _mesh_height - 2)
            {
                data.append(VertexData(QVector3D(cVec.x, cVec.y,  0), QVector2D(), vecCol));
                data.append(VertexData(QVector3D(dVec.x, dVec.y,  0), QVector2D(), vecCol));
            }
        }
    }

    BuildVboWithColor(data, linesVbo);

    if(isInit)
    {
        linesVao->release();
    }
}

void StrokePainter::BuildLinesVertexData(std::vector<ALine> lines, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol)
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

    if(isInit)
    {
        linesVao->release();
    }
}

void StrokePainter::BuildPointsVertexData(std::vector<AVector> points, QOpenGLBuffer* ptsVbo, QOpenGLVertexArrayObject* ptsVao, QVector3D vecCol)
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

    if(isInit)
    {
        ptsVao->release();
    }
}

void StrokePainter::BuildVboWithColor(QVector<VertexData> data, QOpenGLBuffer* vbo)
{
    if(!vbo->isCreated())
    {
        vbo->create();
    }
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

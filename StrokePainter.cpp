
#include "StrokePainter.h"
#include "VertexData.h"
#include "UtilityFunctions.h"
#include "SystemParams.h"

#include <stdlib.h>
#include <iostream>

StrokePainter::StrokePainter()
{
}

StrokePainter::~StrokePainter()
{
}

void StrokePainter::CalculateDecorativeStroke()
{
    _gridLines.clear();

    UtilityFunctions::UniformResample(_oriStrokeLines, _strokeLines, SystemParams::stroke_resample_length);

    for(uint a = 0; a < _strokeLines.size(); a++)
    {
        if(a == 0)
        {
        }
        else if(a == _strokeLines.size() - 1)
        {
        }
        else if(_strokeLines.size() >= 4 && a < _strokeLines.size() - 2)
        {
            ALine prevLine(_strokeLines[a-1], _strokeLines[a]);
            ALine curLine(_strokeLines[a], _strokeLines[a+1]);
            ALine nextLine(_strokeLines[a+1], _strokeLines[a+2]);

            AVector d0Left, d0Right, d1Left, d1Right;
            UtilityFunctions::GetSegmentPoints(curLine, prevLine, nextLine,
                                               SystemParams::stroke_width / 2.0f, SystemParams::stroke_width / 2.0f,
                                               &d0Left, &d0Right, &d1Left, &d1Right);
            _gridLines.push_back(ALine(d0Left, d1Left));
            _gridLines.push_back(ALine(d0Right, d1Right));
            _gridLines.push_back(ALine(d0Left, d0Right));
            _gridLines.push_back(ALine(d1Left, d1Right));
        }
    }

    PrepareLinesVAO(_gridLines, &_gridLinesVbo, &_gridLinesVao, QVector3D(1, 0, 0));
    PrepareLinesVAO(_strokeLines, &_strokeLinesVbo, &_strokeLinesVao, QVector3D(1, 0, 0));
}

// mouse press
void StrokePainter::mousePressEvent(float x, float y)
{
    _oriStrokeLines.clear();
    _oriStrokeLines.push_back(AVector(x, y));
}

// mouse move
void StrokePainter::mouseMoveEvent(float x, float y)
{
    _oriStrokeLines.push_back(AVector(x, y));
    CalculateDecorativeStroke();
}

// mouse release
void StrokePainter::mouseReleaseEvent(float x, float y)
{
    _oriStrokeLines.push_back(AVector(x, y));
    CalculateDecorativeStroke();

    for(uint a = 0; a < _strokeLines.size() - 2; a++)
    {
        std::cout << _strokeLines[a].Distance(_strokeLines[a+1]) << " ";
    }
    std::cout << "\n";
}

void StrokePainter::Draw()
{
    _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);

    if(_strokeLinesVao.isCreated())
    {
        glLineWidth(2.0f);
        _strokeLinesVao.bind();
        glDrawArrays(GL_LINES, 0, (_strokeLines.size() - 1) * 2);
        _strokeLinesVao.release();
    }

    if(_gridLinesVao.isCreated())
    {
        glLineWidth(2.0f);
        _gridLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _gridLines.size() * 2);
        _gridLinesVao.release();
    }
}

void StrokePainter::PrepareLinesVAO(std::vector<AVector> points, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol)
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

    if(!linesVbo->isCreated())
    {
        linesVbo->create();
    }
    linesVbo->bind();

    linesVbo->allocate(data.size() * sizeof(VertexData));
    linesVbo->write(0, data.data(), data.size() * sizeof(VertexData));

    quintptr offset = 0;

    _shaderProgram->enableAttributeArray(_vertexLocation);
    _shaderProgram->setAttributeBuffer(_vertexLocation, GL_FLOAT, 0, 3, sizeof(VertexData));

    offset += sizeof(QVector3D);
    offset += sizeof(QVector2D);

    _shaderProgram->enableAttributeArray(_colorLocation);
    _shaderProgram->setAttributeBuffer(_colorLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    if(isInit)
    {
        linesVao->release();
    }
}

void StrokePainter::PrepareLinesVAO(std::vector<ALine> lines, QOpenGLBuffer* linesVbo, QOpenGLVertexArrayObject* linesVao, QVector3D vecCol)
{
    if(linesVao->isCreated())
    {
        linesVao->destroy();
    }

    linesVao->create();
    linesVao->bind();

    QVector<VertexData> data;
    for(uint a = 0; a < lines.size(); a++)
    {
        data.append(VertexData(QVector3D(lines[a].XA, lines[a].YA,  0), QVector2D(), vecCol));
        data.append(VertexData(QVector3D(lines[a].XB, lines[a].YB,  0), QVector2D(), vecCol));
    }

    linesVbo->create();
    linesVbo->bind();
    linesVbo->allocate(data.data(), data.size() * sizeof(VertexData));

    quintptr offset = 0;

    _shaderProgram->enableAttributeArray(_vertexLocation);
    _shaderProgram->setAttributeBuffer(_vertexLocation, GL_FLOAT, 0, 3, sizeof(VertexData));

    offset += sizeof(QVector3D);
    offset += sizeof(QVector2D);

    _shaderProgram->enableAttributeArray(_colorLocation);
    _shaderProgram->setAttributeBuffer(_colorLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    linesVao->release();
}

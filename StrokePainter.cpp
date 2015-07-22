
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
    UtilityFunctions::UniformResample(_oriStrokeLines, _strokeLines, SystemParams::stroke_resample_length);
}

// mouse press
void StrokePainter::mousePressEvent(float x, float y)
{
    _oriStrokeLines.clear();
    _oriStrokeLines.push_back(AVector(x, y));
    PrepareLinesVAO(_strokeLines, &_strokeLinesVbo, &_strokeLinesVao, QVector3D(1, 0, 0));
}

// mouse move
void StrokePainter::mouseMoveEvent(float x, float y)
{
    _oriStrokeLines.push_back(AVector(x, y));
    CalculateDecorativeStroke();
    PrepareLinesVAO(_strokeLines, &_strokeLinesVbo, &_strokeLinesVao, QVector3D(1, 0, 0));
}

// mouse release
void StrokePainter::mouseReleaseEvent(float x, float y)
{
    _oriStrokeLines.push_back(AVector(x, y));
    CalculateDecorativeStroke();
    PrepareLinesVAO(_strokeLines, &_strokeLinesVbo, &_strokeLinesVao, QVector3D(1, 0, 0));
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

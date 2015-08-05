
#include "StrokePainter.h"
#include "VertexData.h"
#include "CurveRDP.h"
#include "UtilityFunctions.h"
#include "SystemParams.h"

#include <stdlib.h>
#include <iostream>

StrokePainter::StrokePainter() :
    _isMouseDown(false),
    _mesh_width(0),
    _mesh_height(0),
    _imgTexture(0)
{
}

StrokePainter::~StrokePainter()
{
}

void StrokePainter::SetImage(QString img)
{
    /*if(_img.isNull())
    {
        std::cout << "image is empty\n";
    }
    else
    {
        std::cout << "image is legit\n";
    }
    std::cout << img.toStdString() << "\n";*/

    _img.load(img);
    _imgTexture = new QOpenGLTexture(_img);

    /*if(_img.isNull())
    {
        std::cout << "image is empty\n";
    }
    else
    {
        std::cout << "image is legit\n";
    }*/

}

void StrokePainter::CalculateInitialRibbon()
{
    _lLines.clear();
    _rLines.clear();
    _midVerticalLines.clear();

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

            _midVerticalLines.push_back(ALine(lPoint, rPoint));
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
        }
    }
    BuildLinesVertexData(_strokeLines, &_strokeLinesVbo, &_strokeLinesVao, QVector3D(0.5, 0.5, 1));
    BuildLinesVertexData(_midVerticalLines, &_midVerticalLinesVbo, &_midVerticalLinesVao, QVector3D(0.5, 0.5, 1));
    BuildLinesVertexData(_lLines, &_lLinesVbo, &_lLinesVao, QVector3D(0.5, 0.5, 1));
    BuildLinesVertexData(_rLines, &_rLinesVbo, &_rLinesVao, QVector3D(0.5, 0.5, 1));
}

void StrokePainter::CalculateVertices2()
{
    _plusSignVertices.clear();

    for(int a = 0; a < _strokeLines.size() - 1; a++)
    {
        AVector mStartPt = _strokeLines[a];
        AVector mEndPt   = _strokeLines[a + 1];

        int intMeshHeight = SystemParams::stroke_width / SystemParams::mesh_size;
        int intMeshWidth =  mStartPt.Distance(mEndPt) / SystemParams::stroke_width * SystemParams::mesh_size;
    }

    _mesh_width = _plusSignVertices.size();
    _mesh_height = _plusSignVertices[0].size();
}

void StrokePainter::CalculateVertices1()
{
    //_debugPoints.clear();

    //_vertices.clear();
    _plusSignVertices.clear();

    for(int a = 0; a < _strokeLines.size() - 1; a++)
    {
        AVector mStartPt = _strokeLines[a];
        AVector mEndPt   = _strokeLines[a + 1];

        // no corner avoidance
        //int intMeshHeight = SystemParams::stroke_width / SystemParams::mesh_size;
        //int intMeshWidth = mStartPt.Distance(mEndPt) / SystemParams::mesh_size;


        // with corner avoidance
        int intMeshHeight = SystemParams::stroke_width / SystemParams::mesh_size;
        int intMeshWidth =  (int)(mStartPt.Distance(mEndPt) / SystemParams::stroke_width) * intMeshHeight;


        AVector lStartPt = _lLines[a];
        AVector lEndPt   = _lLines[a + 1];

        AVector rStartPt = _rLines[a];
        AVector rEndPt   = _rLines[a + 1];

        AVector vVec = rStartPt - lStartPt;

        AVector uHVec = (lEndPt - lStartPt);
        AVector bHVec = (rEndPt - rStartPt);

        int xLoop = intMeshWidth;
        int yLoop = intMeshHeight + 1;

        bool isTheEnd = false;
        if(a == _strokeLines.size() - 2)
        {
            isTheEnd = true;
            xLoop++;
        }

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

                bool shouldMove = true;
                bool midVerticalConstrained = false;
                bool midHorizontalConstrained = false;
                if(SystemParams::enforce_miter_joint && xIter == 0 && yIter == 0)
                    { shouldMove = false; }
                else if(SystemParams::enforce_miter_joint && xIter == 0 && yIter == yLoop - 1 )
                    { shouldMove = false; }
                else if(SystemParams::enforce_miter_joint && isTheEnd && xIter == xLoop - 1 && yIter == 0)
                    { shouldMove = false; }
                else if(SystemParams::enforce_miter_joint && isTheEnd && xIter == xLoop - 1 && yIter == yLoop - 1 )
                    { shouldMove = false; }

                if(a > 0 && xIter == 0)
                {
                    midVerticalConstrained = true;
                }

                // odd only
                if(yLoop % 2 != 0)
                {
                    int yMid = yLoop / 2;
                    if(yIter == yMid)
                    {
                        midHorizontalConstrained = true;
                    }
                }


                /*
                if(yLoop % 2 == 0) // even
                {
                    int yMid1 = yLoop / 2;
                    int yMid2 = yMid1 - 1;

                    if(xIter == 0 && (yIter == yMid1 || yIter == yMid2) )
                        { shouldMove = false; }
                    else if(isTheEnd && xIter == xLoop - 1 && (yIter == yMid1 || yIter == yMid2))
                        { shouldMove = false; }
                    else if(isTheEnd && xIter == xLoop - 1 && (yIter == yMid1 || yIter == yMid2) )
                        { shouldMove = false; }
                }
                else // odd
                {

                    int yMid = yLoop / 2;
                    std::cout << yMid << "\n";

                    if(xIter == 0 && (yIter == yMid) )
                        { shouldMove = false; }
                    else if(isTheEnd && xIter == xLoop - 1 && (yIter == yMid))
                        { shouldMove = false; }
                    else if(isTheEnd && xIter == xLoop - 1 && (yIter == yMid) )
                        { shouldMove = false; }
                }*/

                columnVertices.push_back(PlusSignVertex(pt, shouldMove, midVerticalConstrained, midHorizontalConstrained));
            }

            _plusSignVertices.push_back(columnVertices);


        }

    }

    _mesh_width = _plusSignVertices.size();
    _mesh_height = _plusSignVertices[0].size();

    // don't create vertex data here
    //BuildPointsVertexData(_vertices, &_verticesVbo, &_verticesVao, QVector3D(0, 0, 1));
    //BuildLinesVertexData(_plusSignVertices, &_plusSignVerticesVbo, &_plusSignVerticesVao, QVector3D(1, 0, 0));
    //BuildPointsVertexData(_debugPoints, &_debugPointsVbo, &_debugPointsVao, QVector3D(0, 0.5, 0));
}

AVector StrokePainter::GetClosestPointFromMiddleVerticalLines(AVector pt)
{
    AVector closestPt = pt;
    float dist = std::numeric_limits<float>::max();
    for(int a = 0; a < _midVerticalLines.size(); a++)
    {
        AVector pt1 = _midVerticalLines[a].GetPointA();
        AVector pt2 = _midVerticalLines[a].GetPointB();
        AVector cPt = UtilityFunctions::GetClosestPoint(pt1, pt2, pt);
        if(pt.Distance(cPt) < dist)
        {
            dist = pt.Distance(cPt);
            closestPt = cPt;
        }
    }
    return closestPt;
}

AVector StrokePainter::GetClosestPointFromStrokePoints(AVector pt)
{
    AVector closestPt = pt;
    float dist = std::numeric_limits<float>::max();

    for(int a = 0; a < _strokeLines.size(); a++)
    {
        AVector cPt = _strokeLines[a];
        if(pt.Distance(cPt) < dist)
        {
            dist = pt.Distance(cPt);
            closestPt = cPt;
        }
    }

    return closestPt;
}

AVector StrokePainter::GetClosestPointFromStrokeLines(AVector pt)
{
    AVector closestPt = pt;
    float dist = std::numeric_limits<float>::max();

    for(int a = 0; a < _strokeLines.size() - 1; a++)
    {
        AVector pt1 = _strokeLines[a];
        AVector pt2 = _strokeLines[a+1];
        AVector cPt = UtilityFunctions::GetClosestPoint(pt1, pt2, pt);
        if(pt.Distance(cPt) < dist)
        {
            dist = pt.Distance(cPt);
            closestPt = cPt;
        }
    }

    return closestPt;
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

void StrokePainter::ConformalMappingOneStep2()
{
    std::vector<std::vector<PlusSignVertex>> tempVertices = _plusSignVertices;

    for(int a = 0; a < _mesh_width; a++)
    {
        for(int b = 0; b < _mesh_height; b++)
        {
            if(!tempVertices[a][b].shouldMove)
            {
                continue;
            }

            AVector curPos = tempVertices[a][b].position;
            AVector sumPositions(0, 0);
            float sumArmLengths = 0;
            float sumArmAngles = 0;
            int numNeighbor = 0;

            // have left and right
            if(a > 0 && a < _mesh_width - 1)
            {
                PlusSignVertex lVertex = tempVertices[a - 1][b];
                PlusSignVertex rVertex = tempVertices[a + 1][b];

                // left
                sumPositions += lVertex.position;
                sumArmLengths += curPos.Distance(lVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(-1, 0), (lVertex.position - curPos).Norm());


                // right
                sumPositions += rVertex.position;
                sumArmLengths += curPos.Distance(rVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(1, 0), (rVertex.position - curPos).Norm());

                numNeighbor += 2;
            }
            // have left only
            else if(a > 0)
            {
                PlusSignVertex lVertex = tempVertices[a - 1][b];
                AVector fakeNeighbor = lVertex.position + UtilityFunctions::Rotate( AVector(1, 0) * lVertex.armLength, lVertex.angle);
                sumPositions += fakeNeighbor;
                sumArmLengths += curPos.Distance(lVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(-1, 0), (lVertex.position - curPos).Norm());

                numNeighbor++;
            }
            // have right only
            else if(a < _mesh_width - 1)
            {
                PlusSignVertex rVertex = tempVertices[a + 1][b];
                AVector fakeNeighbor = rVertex.position + UtilityFunctions::Rotate( AVector(-1, 0) * rVertex.armLength, rVertex.angle);

                sumPositions += fakeNeighbor;
                sumArmLengths += curPos.Distance(rVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(1, 0), (rVertex.position - curPos).Norm());

                numNeighbor++;
            }

            // have up and down
            if(b > 0 && b < _mesh_height - 1)
            {
                PlusSignVertex uVertex = tempVertices[a][b - 1];
                PlusSignVertex bVertex = tempVertices[a][b + 1];

                // up
                sumPositions += uVertex.position;
                sumArmLengths += curPos.Distance(uVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(0, -1), (uVertex.position - curPos).Norm());

                // down
                sumPositions += bVertex.position;
                sumArmLengths += curPos.Distance(bVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(0, 1), (bVertex.position - curPos).Norm());

                numNeighbor += 2;
            }
            // have up only
            else if(b > 0)
            {
                PlusSignVertex uVertex = tempVertices[a][b - 1];
                AVector fakeNeighbor = uVertex.position + UtilityFunctions::Rotate( AVector(0, 1) * uVertex.armLength, uVertex.angle);
                sumPositions += fakeNeighbor;
                sumArmLengths += curPos.Distance(uVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(0, -1), (uVertex.position - curPos).Norm());

                numNeighbor++;
            }
            // have down only
            else if(b < _mesh_height - 1)
            {
                PlusSignVertex bVertex = tempVertices[a][b + 1];
                AVector fakeNeighbor = bVertex.position + UtilityFunctions::Rotate( AVector(0, -1) * bVertex.armLength, bVertex.angle);
                sumPositions += fakeNeighbor;
                sumArmLengths += curPos.Distance(bVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(0, 1), (bVertex.position - curPos).Norm());

                numNeighbor++;
            }

            sumPositions  = sumPositions / (float)numNeighbor;
            sumArmAngles  = sumArmAngles / (float)numNeighbor;
            sumArmLengths = sumArmLengths / (float)numNeighbor;

            tempVertices[a][b].armLength = sumArmLengths;
            tempVertices[a][b].angle = sumArmAngles;

            if(numNeighbor < 4)
            {
                AVector closestPt = GetClosestPointFromBorders(sumPositions);
                tempVertices[a][b].position = closestPt;
            }

            else if(tempVertices[a][b].midHorizontalConstrained && tempVertices[a][b].midVerticalConstrained)
            {

                AVector closestPt = GetClosestPointFromStrokePoints(sumPositions);
                tempVertices[a][b].position = closestPt;
            }

            else if(tempVertices[a][b].midHorizontalConstrained)
            {
                AVector closestPt = GetClosestPointFromStrokeLines(sumPositions);
                tempVertices[a][b].position = closestPt;
            }

            else if(tempVertices[a][b].midVerticalConstrained)
            {
                AVector closestPt = GetClosestPointFromMiddleVerticalLines(sumPositions);
                tempVertices[a][b].position = closestPt;
            }

            else
            {
                tempVertices[a][b].position = sumPositions;
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

    _plusSignVertices = tempVertices;
    BuildLinesVertexData(_plusSignVertices, &_plusSignVerticesVbo, &_plusSignVerticesVao, QVector3D(1, 0, 0));
    BuildTexturedStrokeVertexData(_plusSignVertices, &_texturedStrokeVbo, &_texturedStrokeVao);
    BuildConstrainedPointsVertexData(_plusSignVertices, &_constrainedPointsVbo, &_constrainedPointsVao, QVector3D(0.5, 0.5, 1));
    //BuildLinesVertexData(_debugLines, &_debugLinesVbo, &_debugLinesVao, QVector3D(0, 0, 1));
    //BuildPointsVertexData(_debugPoints, &_debugPointsVbo, &_debugPointsVao, QVector3D(0, 0.5, 0));
}

void StrokePainter::ConformalMappingOneStep1()
{
    std::vector<std::vector<PlusSignVertex>> tempVertices = _plusSignVertices;

    for(int a = 0; a < _mesh_width; a++)
    {
        for(int b = 0; b < _mesh_height; b++)
        {
            AVector sumPositions(0, 0);
            int numNeighbor = 0;

            // left
            if(a > 0)
            {
                PlusSignVertex lVertex = tempVertices[a - 1][b];
                sumPositions += lVertex.position;
                numNeighbor++;
            }

            // right
            if(a < _mesh_width - 1)
            {
                PlusSignVertex rVertex = tempVertices[a + 1][b];
                sumPositions = sumPositions + rVertex.position;
                numNeighbor++;
            }

            // up
            if(b > 0)
            {
                PlusSignVertex uVertex = tempVertices[a][b - 1];
                sumPositions += uVertex.position;
                numNeighbor++;
            }

            // bottom
            if(b < _mesh_height - 1)
            {
                PlusSignVertex bVertex = tempVertices[a][b + 1];
                sumPositions += bVertex.position;
                numNeighbor++;
            }

            sumPositions  = sumPositions / (float)numNeighbor;

            if(numNeighbor < 4)
            {
                AVector closestPt = GetClosestPointFromBorders(sumPositions);
                tempVertices[a][b].position = closestPt;
            }
            else
            {
                tempVertices[a][b].position = sumPositions;
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

    _plusSignVertices = tempVertices;

    BuildLinesVertexData(_plusSignVertices, &_plusSignVerticesVbo, &_plusSignVerticesVao, QVector3D(1, 0, 0));

}

// mouse press
void StrokePainter::mousePressEvent(float x, float y)
{
    _isMouseDown = true;

    //_points.clear();
    //_vertices.clear();
    _plusSignVertices.clear();

    _midVerticalLines.clear();
    _lLines.clear();
    _rLines.clear();

    //_ribLines.clear();
    //_gridLines.clear();
    _numConstrainedPoints = 0;

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
    _isMouseDown = false;

    _oriStrokeLines.push_back(AVector(x, y));
    CalculateInitialRibbon();
    CalculateVertices1();
}

void StrokePainter::Draw()
{


    /*if(_debugPointsVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);
        glPointSize(4.0f);
        _debugPointsVao.bind();
        glDrawArrays(GL_POINTS, 0, _debugPoints.size());
        _debugPointsVao.release();
    }*/

    /*if(_debugLinesVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);
        glLineWidth(2.0f);
        _debugLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _debugLines.size() * 2);
        _debugLinesVao.release();
    }*/

    if((_isMouseDown || SystemParams::show_mesh) &&_strokeLinesVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);

        glLineWidth(2.0f);
        _strokeLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _strokeLines.size() * 2);
        _strokeLinesVao.release();
    }

    if(SystemParams::show_mesh && _midVerticalLinesVao.isCreated())
        {
            _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);
            glLineWidth(2.0f);
            _midVerticalLinesVao.bind();
            glDrawArrays(GL_LINES, 0, _midVerticalLines.size() * 2);
            _midVerticalLinesVao.release();
        }

    if(SystemParams::show_mesh && SystemParams::enforce_miter_joint && _constrainedPointsVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);

        glPointSize(10.0f);
        _constrainedPointsVao.bind();
        glDrawArrays(GL_POINTS, 0, _numConstrainedPoints);
        _constrainedPointsVao.release();
    }


    if(SystemParams::show_mesh && _plusSignVerticesVao.isCreated() && _plusSignVertices.size() > 0)
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);

        int wMin1 = _mesh_width - 1;
        int hMin1 = _mesh_height - 1;

        int meshSize = ((wMin1 * _mesh_width) + (hMin1 * _mesh_height)) * 2;

        glLineWidth(1.0f);
        _plusSignVerticesVao.bind();
        glDrawArrays(GL_LINES, 0, meshSize);
        _plusSignVerticesVao.release();
    }




    //_texturedStrokeVao
    if(SystemParams::show_texture && _texturedStrokeVao.isCreated() && _plusSignVertices.size() > 0)
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)0.0);

        int wMin1 = _mesh_width - 1;
        int hMin1 = _mesh_height - 1;

        int meshSize = wMin1 * hMin1 * 4;

        glLineWidth(1.0f);
        if(_imgTexture)
            _imgTexture->bind();
        _texturedStrokeVao.bind();
        glDrawArrays(GL_QUADS, 0, meshSize);
        _texturedStrokeVao.release();
        if(_imgTexture)
            _imgTexture->release();
    }

    /*
    if(_strokeLinesVao.isCreated() && _plusSignVertices.size() == 0)
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);

        glLineWidth(2.0f);
        _strokeLinesVao.bind();
        glDrawArrays(GL_LINES, 0, (_strokeLines.size() - 1) * 2);
        _strokeLinesVao.release();
    }*/

    /*
    if(_pointsVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);
        glPointSize(10.0f);
        _pointsVao.bind();
        glDrawArrays(GL_POINTS, 0, _points.size());
        _pointsVao.release();
    }*/

    /*if(_verticesVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);
        glPointSize(10.0f);
        _verticesVao.bind();
        glDrawArrays(GL_POINTS, 0, _vertices.size());
        _verticesVao.release();
    }*/

    /*
    if(_lLinesVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);
        glLineWidth(2.0f);
        _lLinesVao.bind();
        glDrawArrays(GL_LINES, 0, (_lLines.size() - 1) * 2);
        _lLinesVao.release();
    }

    if(_rLinesVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);
        glLineWidth(2.0f);
        _rLinesVao.bind();
        glDrawArrays(GL_LINES, 0, (_rLines.size() - 1) * 2);
        _rLinesVao.release();
    }
    */

    /*
    if(_ribLinesVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);
        glLineWidth(2.0f);
        _ribLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _ribLines.size() * 2);
        _ribLinesVao.release();
    }

    if(_gridLinesVao.isCreated())
    {
        _shaderProgram->setUniformValue(_use_color_location, (GLfloat)1.0);
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

    if(isInit) { linesVao->release(); }
}

void StrokePainter::BuildConstrainedPointsVertexData(std::vector<std::vector<PlusSignVertex>> plusSignVertices, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao, QVector3D vecCol)
{
    if(plusSignVertices.size() == 0) return;

    bool isInit = false;
    if(!vao->isCreated())
    {
        vao->create();
        vao->bind();
        isInit = true;
    }

    _numConstrainedPoints = 0;
    QVector<VertexData> data;
    for(int a = 0; a < _mesh_width; a++)
    {
        for(int b = 0; b < _mesh_height; b++)
        {
            PlusSignVertex psVertex = plusSignVertices[a][b];

            if(!psVertex.shouldMove)
            {
                data.append(VertexData(QVector3D(psVertex.position.x, psVertex.position.y,  0), QVector2D(), vecCol));
                _numConstrainedPoints++;
            }
        }
    }

    BuildVboWithColor(data, vbo);

    if(isInit) { vao->release(); }

}

void StrokePainter::BuildTexturedStrokeVertexData(std::vector<std::vector<PlusSignVertex>> plusSignVertices, QOpenGLBuffer* vbo, QOpenGLVertexArrayObject* vao)
{
    if(plusSignVertices.size() == 0) return;

    bool isInit = false;
    if(!vao->isCreated())
    {
        vao->create();
        vao->bind();
        isInit = true;
    }


    int heightMinOne = _mesh_height - 1;

    QVector<VertexData> data;
    for(int a = 0; a < _mesh_width - 1; a++)
    {
        float aNominator = a % heightMinOne;

        float xCoord1 = (aNominator) / (float)heightMinOne;
        float xCoord2 = (aNominator + 1.0f) / (float)heightMinOne;

        for(int b = 0; b < _mesh_height - 1; b++)
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

    if(isInit) { linesVao->release(); }
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

    if(isInit) { linesVao->release(); }
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

    if(isInit) { ptsVao->release(); }
}



void StrokePainter::BuildVboWithColor(QVector<VertexData> data, QOpenGLBuffer* vbo)
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

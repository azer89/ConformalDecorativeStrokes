
#include "StrokePainter.h"
#include "VertexData.h"
#include "CurveRDP.h"
#include "UtilityFunctions.h"
#include "SystemParams.h"

#include <stdlib.h>
#include <iostream>

StrokePainter::StrokePainter() :
    _isMouseDown(false),
    _vDataHelper(0),
    _cMapping(new ConformalMapping()),
    _qMeshNumData(0),
    _masterImages(std::vector<QImage>(2)),              // current only support two textures
    _masterTextures(std::vector<QOpenGLTexture*>(2)),
    _qmTexNumbers(std::vector<int>(2)),
    _qmTexVbos(std::vector<QOpenGLBuffer>(2)),
    _qmTexVaos(std::vector<QOpenGLVertexArrayObject>(2)),
    _selectedIndex(-1),
    _maxDist(2.0f)
{
}

StrokePainter::~StrokePainter()
{
    if(_vDataHelper) delete _vDataHelper;
}

void StrokePainter::SetVertexDataHelper(QOpenGLShaderProgram* shaderProgram)
{
    _vDataHelper = new VertexDataHelper(shaderProgram);
}

void StrokePainter::SetStrokeTexture(QString img)
{
    _masterImages[0].load(img);
    _masterTextures[0] = new QOpenGLTexture(_masterImages[0]);
}

void StrokePainter::SetCornerTexture(QString img)
{
    _masterImages[1].load(img);
    _masterTextures[1] = new QOpenGLTexture(_masterImages[1]);
}

void StrokePainter::CalculateLeftRightLines()
{
    float halfStrokeWidth = SystemParams::stroke_width / 2.0f;

    // calculate left, right, and junction ribs;
    _leftLines.clear();
    _rightLines.clear();
    //_junctionRibLines.clear();
    for(uint a = 0; a < _spineLines.size(); a++)
    {
        if(a == 0)
        {
            AVector pt1 = _spineLines[0];
            AVector pt2 = _spineLines[1];
            AVector dirVec = (pt2 - pt1).Norm() * halfStrokeWidth;
            _leftLines.push_back(pt1 + AVector(  dirVec.y,  -dirVec.x));
            _rightLines.push_back(pt1 + AVector(-dirVec.y,   dirVec.x));
        }
        else if(_spineLines.size() >= 3 && a <= _spineLines.size() - 2)
        {
            ALine prevLine(_spineLines[a-1], _spineLines[a]);
            ALine curLine(_spineLines[a], _spineLines[a+1]);
            AVector lPoint, rPoint;
            UtilityFunctions::GetMiterJoints(prevLine, curLine, halfStrokeWidth, halfStrokeWidth, &lPoint, &rPoint);
            _leftLines.push_back(lPoint);
            _rightLines.push_back(rPoint);
            //_junctionRibLines.push_back(ALine(lPoint, rPoint));
        }

        // add an end
        if(a == _spineLines.size() - 2)
        {
            AVector pt1 = _spineLines[a];
            AVector pt2 = _spineLines[a + 1];
            AVector dirVec = (pt2 - pt1).Norm() * halfStrokeWidth;
            _leftLines.push_back(pt2 + AVector( dirVec.y,  -dirVec.x));
            _rightLines.push_back(pt2 + AVector(-dirVec.y,  dirVec.x));
        }
    }

}

void StrokePainter::CalculateInitialRibbon()
{
    //CalculateSpines(); // I removed this, but I'm not sure whether I should bring it back...
    CalculateLeftRightLines();
    CalculateKitesAndRectangles();

    _vDataHelper->BuildLinesVertexData(_spineLines, &_spineLinesVbo, &_spineLinesVao, QVector3D(0.5, 0.5, 1));
    _vDataHelper->BuildLinesVertexData(_leftLines, &_leftLinesVbo, &_leftLinesVao, QVector3D(0.5, 0.5, 1));
    _vDataHelper->BuildLinesVertexData(_rightLines, &_rightLinesVbo, &_rightLinesVao, QVector3D(0.5, 0.5, 1));
}

void StrokePainter::CalculateSpines()
{
    _spineLines.clear();
    std::vector<AVector> tempLine;
    CurveRDP::SimplifyRDP(_oriStrokeLines, tempLine, SystemParams::rdp_epsilon);
    _spineLines = std::vector<AVector>(tempLine);
}

void StrokePainter::CalculateKitesAndRectangles()
{
    _quadMeshes.clear();

    float strokeWidth = SystemParams::stroke_width;

    for(uint a = 0; a < _spineLines.size() - 1; a++)
    {
        // KITES
        if(a > 0 && _spineLines.size() >= 3)
        {
            ALine prevLine(_spineLines[a-1], _spineLines[a]);
            ALine curLine(_spineLines[a], _spineLines[a+1]);

            AVector dir1 = prevLine.Direction().Norm();
            AVector dir2 = curLine.Direction().Norm();

            float rot1 = UtilityFunctions::GetRotation(dir1, dir2);
            if(rot1 > 0)
            {
                // turn right: positive rotation
                AVector lMid = _leftLines[a];
                AVector rMid = _rightLines[a];
                AVector lStart = rMid + AVector(dir1.y, -dir1.x) * strokeWidth;
                AVector lEnd = rMid + AVector(dir2.y, -dir2.x) * strokeWidth;

                QuadMesh qMesh;
                qMesh._leftStartPt  = lStart;
                qMesh._leftEndPt    = lMid;
                qMesh._rightStartPt = rMid;
                qMesh._rightEndPt   = lEnd;
                qMesh._sharpPt = lMid;
                qMesh._isRightKite = true;
                qMesh._quadMeshType = QuadMeshType::MESH_KITE;
                _quadMeshes.push_back(qMesh);
            }
            else if(rot1 < 0)
            {
                // turn left: negative rotation
                AVector lMid = _leftLines[a];
                AVector rMid = _rightLines[a];
                AVector rStart = lMid + AVector(-dir1.y, dir1.x) * strokeWidth;
                AVector rEnd  = lMid + AVector(-dir2.y, dir2.x) * strokeWidth;

                QuadMesh qMesh;
                qMesh._leftStartPt  = rStart;
                qMesh._leftEndPt    = lMid;
                qMesh._rightStartPt = rMid;
                qMesh._rightEndPt   = rEnd;
                qMesh._sharpPt = rMid;
                qMesh._isRightKite = false;
                qMesh._quadMeshType = QuadMeshType::MESH_KITE;
                _quadMeshes.push_back(qMesh);
            }
        }

        // RECTANGLES
        if(a == 0 && _spineLines.size() > 2)    // START
        {
            ALine curLine(_spineLines[a], _spineLines[a+1]);
            ALine nextLine(_spineLines[a+1], _spineLines[a+2]);

            AVector dir1 = curLine.Direction().Norm();
            AVector dir2 = nextLine.Direction().Norm();
            float rot = UtilityFunctions::GetRotation(dir1, dir2);

            AVector leftEnd = _leftLines[a+1];
            AVector rightEnd = _rightLines[a+1];
            if(rot > 0)
            {
                // turn right: positive rotation
                AVector leftDir(dir1.y, -dir1.x);
                leftEnd = _rightLines[a+1] + leftDir * strokeWidth;
            }
            else if(rot < 0)
            {
                // turn left: negative rotation
                AVector rightDir(-dir1.y, dir1.x);
                rightEnd = _leftLines[a+1] + rightDir * strokeWidth;
            }

            QuadMesh qMesh;
            qMesh._leftStartPt  = _leftLines[a];
            qMesh._leftEndPt    = leftEnd;
            qMesh._rightStartPt = _rightLines[a];
            qMesh._rightEndPt   = rightEnd;
            qMesh._quadMeshType = QuadMeshType::MESH_RECTANGLE;
            _quadMeshes.push_back(qMesh);
        }
        else if(a == 0 && _spineLines.size() == 2)  // START
        {
            QuadMesh qMesh;
            qMesh._leftStartPt  = _leftLines[a];
            qMesh._leftEndPt    = _leftLines[a+1];
            qMesh._rightStartPt = _rightLines[a];
            qMesh._rightEndPt   = _rightLines[a+1];
            qMesh._quadMeshType = QuadMeshType::MESH_RECTANGLE;
            _quadMeshes.push_back(qMesh);
        }

        else if(a == _spineLines.size() - 2 && _spineLines.size() > 2) // END
        {
            ALine prevLine(_spineLines[a-1], _spineLines[a]);
            ALine curLine(_spineLines[a], _spineLines[a+1]);
            AVector dir1 = prevLine.Direction().Norm();
            AVector dir2 = curLine.Direction().Norm();
            float rot = UtilityFunctions::GetRotation(dir1, dir2);

            AVector leftStart = _leftLines[a];
            AVector rightStart = _rightLines[a];
            if(rot > 0)
            {
                // turn right: positive rotation
                AVector leftDir(dir2.y, -dir2.x);
                leftStart = _rightLines[a] + leftDir * strokeWidth;
            }
            else if(rot < 0)
            {
                // turn left: negative rotation
                AVector rightDir(-dir2.y, dir2.x);
                rightStart = _leftLines[a] + rightDir * strokeWidth;
            }

            QuadMesh qMesh;
            qMesh._leftStartPt  = leftStart;
            qMesh._leftEndPt    = _leftLines[a+1];
            qMesh._rightStartPt = rightStart;
            qMesh._rightEndPt   = _rightLines[a+1];
            qMesh._quadMeshType = QuadMeshType::MESH_RECTANGLE;
            _quadMeshes.push_back(qMesh);
        }

        else if(a > 0)  // MIDDLE
        {
            ALine prevLine(_spineLines[a-1], _spineLines[a]);
            ALine curLine(_spineLines[a], _spineLines[a+1]);
            ALine nextLine(_spineLines[a+1], _spineLines[a+2]);

            AVector dir1 = prevLine.Direction().Norm();
            AVector dir2 = curLine.Direction().Norm();
            AVector dir3 = nextLine.Direction().Norm();

            float rot1 = UtilityFunctions::GetRotation(dir1, dir2);
            float rot2 = UtilityFunctions::GetRotation(dir2, dir3);

            AVector leftStart = _leftLines[a];
            AVector rightStart = _rightLines[a];
            AVector leftEnd = _leftLines[a+1];
            AVector rightEnd = _rightLines[a+1];

            // start
            if(rot1 > 0)
            {
                AVector leftDir(dir2.y, -dir2.x);
                leftStart = _rightLines[a] + leftDir * strokeWidth;
            }
            else if(rot1 < 0)
            {
                AVector rightDir(-dir2.y, dir2.x);
                rightStart = _leftLines[a] + rightDir * strokeWidth;
            }

            // end
            if(rot2 > 0)
            {
                AVector leftDir(dir2.y, -dir2.x);
                leftEnd = _rightLines[a+1] + leftDir * strokeWidth;
            }
            else if(rot2 < 0)
            {
                AVector rightDir(-dir2.y, dir2.x);
                rightEnd = _leftLines[a+1] + rightDir * strokeWidth;
            }

            QuadMesh qMesh;
            qMesh._leftStartPt  = leftStart;
            qMesh._leftEndPt    = leftEnd;
            qMesh._rightStartPt = rightStart;
            qMesh._rightEndPt   = rightEnd;
            qMesh._quadMeshType = QuadMeshType::MESH_RECTANGLE;
            _quadMeshes.push_back(qMesh);
        }
    }
}

void StrokePainter::CalculateVertices()
{
    _constrainedPoints.clear();
    for(uint a = 0; a < _quadMeshes.size(); a++)
    {
        CalculateVertices(&_quadMeshes[a]);
    }
    _qMeshNumData = 0;
    _vDataHelper->BuildPointsVertexData(_constrainedPoints, &_constrainedPointsVbo, &_constrainedPointsVao, QVector3D(1, 0, 0));
    _vDataHelper->BuildLinesVertexData(_quadMeshes, &_quadMeshesVbo, &_quadMeshesVao, _qMeshNumData, QVector3D(0, 0, 0), QVector3D(0, 0, 1));
}

void StrokePainter::CalculateVertices(QuadMesh* qMesh)
{
    qMesh->_psVertices.clear();
    qMesh->_opsVertices.clear();

    AVector lStartPt = qMesh->_leftStartPt;
    AVector lEndPt   = qMesh->_leftEndPt ;

    AVector rStartPt = qMesh->_rightStartPt;
    AVector rEndPt   = qMesh->_rightEndPt;

    AVector vVec = rStartPt - lStartPt;

    AVector uHVec = (lEndPt - lStartPt);
    AVector bHVec = (rEndPt - rStartPt);

    AVector mStartPt = ALine(lStartPt, rStartPt).GetMiddlePoint();
    AVector mEndPt   = ALine(lEndPt, rEndPt).GetMiddlePoint();

    float meshSize = SystemParams::mesh_size;

    int intMeshHeight = SystemParams::stroke_width / meshSize;
    int intMeshWidth =  (int)(mStartPt.Distance(mEndPt) / SystemParams::stroke_width) * intMeshHeight;

    // to do: fix this bug
    if(intMeshWidth == 0) { intMeshWidth = intMeshHeight; }

    if(qMesh->_quadMeshType == QuadMeshType::MESH_KITE) { intMeshWidth = intMeshHeight; }

    int xLoop = intMeshWidth;
    int yLoop = intMeshHeight + 1;
    xLoop++;

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
            bool junctionRibsConstrained = false;
            bool spinesConstrained = false;

            if(!SystemParams::fixed_separation_constraint && qMesh->_quadMeshType == QuadMeshType::MESH_KITE)
            {
                if( (xIter == 0 && yIter == 0) ||
                    (xIter == xLoop - 1 && yIter == yLoop - 1) ||
                    (xIter == 0 && yIter == yLoop - 1) ||
                    (xIter == xLoop - 1 && yIter == 0))
                {
                    _constrainedPoints.push_back(pt);
                    shouldMove = false;
                }
            }

            if(SystemParams::fixed_separation_constraint)
            {
                if(qMesh->_quadMeshType == QuadMeshType::MESH_KITE)
                {
                    if(pt.Distance(qMesh->_sharpPt) < std::numeric_limits<float>::epsilon() * 100)
                    {
                        _constrainedPoints.push_back(pt);
                        shouldMove = false;
                    }
                    else if(qMesh->_isRightKite && (xIter == 0 || yIter == yLoop - 1))
                    {
                        _constrainedPoints.push_back(pt);
                        shouldMove = false;
                    }
                    else if(!qMesh->_isRightKite && (xIter == xLoop - 1 || yIter == 0))
                    {
                        _constrainedPoints.push_back(pt);
                        shouldMove = false;
                    }
                }
            }

            PlusSignVertex psVert = PlusSignVertex(pt, shouldMove, junctionRibsConstrained, spinesConstrained);
            columnVertices.push_back(psVert);
        }
        qMesh->_psVertices.push_back(columnVertices);
        qMesh->_opsVertices.push_back(columnVertices);
    }
}

int StrokePainter::GetClosestIndexFromSpinePoints(AVector pt, float maxDist)
{
    int closestIndex = -1;
    float dist = std::numeric_limits<float>::max();
    for(uint a = 0; a < _spineLines.size(); a++)
    {
        AVector cPt = _spineLines[a];
        float d = pt.Distance(cPt);
        if(d < dist && d < maxDist)
        {
            dist = pt.Distance(cPt);
            closestIndex = a;
        }
    }
    return closestIndex;
}


AVector StrokePainter::GetClosestPointFromSpinePoints(AVector pt)
{
    AVector closestPt = pt;
    float dist = std::numeric_limits<float>::max();
    for(uint a = 0; a < _spineLines.size(); a++)
    {
        AVector cPt = _spineLines[a];
        if(pt.Distance(cPt) < dist)
        {
            dist = pt.Distance(cPt);
            closestPt = cPt;
        }
    }
    return closestPt;
}

AVector StrokePainter::GetClosestPointFromSpineLines(AVector pt)
{
    AVector closestPt = pt;
    float dist = std::numeric_limits<float>::max();
    for(uint a = 0; a < _spineLines.size() - 1; a++)
    {
        AVector pt1 = _spineLines[a];
        AVector pt2 = _spineLines[a+1];
        AVector cPt = UtilityFunctions::GetClosestPoint(pt1, pt2, pt);
        if(pt.Distance(cPt) < dist)
        {
            dist = pt.Distance(cPt);
            closestPt = cPt;
        }
    }
    return closestPt;
}

void StrokePainter::ConformalMappingOneStepSimple()
{
    _cMapping->ConformalMappingOneStepSimple(_quadMeshes);

    _qMeshNumData = 0;
    _vDataHelper->BuildLinesVertexData(_quadMeshes, &_quadMeshesVbo, &_quadMeshesVao, _qMeshNumData, QVector3D(0, 0, 0), QVector3D(0, 0, 1));
    _vDataHelper->BuildTexturedStrokeVertexData(_quadMeshes, &_qmTexVbos[0], &_qmTexVaos[0], _qmTexNumbers[0], QuadMeshType::MESH_RECTANGLE);
    _vDataHelper->BuildTexturedStrokeVertexData(_quadMeshes, &_qmTexVbos[1], &_qmTexVaos[1], _qmTexNumbers[1], QuadMeshType::MESH_KITE);
}

void StrokePainter::ConformalMappingOneStep()
{
    _cMapping->ConformalMappingOneStep(_quadMeshes);

    // debug (delete after use)
    //_debugPoints = _cMapping->_debugPoints;
    //_vDataHelper->BuildPointsVertexData(_debugPoints, &_debugPointsVbo, &_debugPointsVao, QVector3D(0, 0.25, 0));

    //_debugLines = _cMapping->_debugLines;
    //_vDataHelper->BuildLinesVertexData(_debugLines, &_debugLinesVbo, &_debugLinesVao, QVector3D(0, 0.25, 0));

    _qMeshNumData = 0;
    _vDataHelper->BuildLinesVertexData(_quadMeshes, &_quadMeshesVbo, &_quadMeshesVao, _qMeshNumData, QVector3D(0, 0, 0), QVector3D(0, 0, 1));
    _vDataHelper->BuildTexturedStrokeVertexData(_quadMeshes, &_qmTexVbos[0], &_qmTexVaos[0], _qmTexNumbers[0], QuadMeshType::MESH_RECTANGLE);
    _vDataHelper->BuildTexturedStrokeVertexData(_quadMeshes, &_qmTexVbos[1], &_qmTexVaos[1], _qmTexNumbers[1], QuadMeshType::MESH_KITE);
}

void StrokePainter::MappingInterpolation()
{
    //std::cout << "mapping interpolation\n";

    _cMapping->MappingInterpolation(_quadMeshes);

    // debug (delete after use)
    //_debugPoints = _cMapping->_debugPoints;
    //_vDataHelper->BuildPointsVertexData(_debugPoints, &_debugPointsVbo, &_debugPointsVao, QVector3D(0, 0.25, 0));

    //_debugLines = _cMapping->_debugLines;
    //_vDataHelper->BuildLinesVertexData(_debugLines, &_debugLinesVbo, &_debugLinesVao, QVector3D(0, 0.25, 0));

    _qMeshNumData = 0;
    _vDataHelper->BuildLinesVertexData(_quadMeshes, &_quadMeshesVbo, &_quadMeshesVao, _qMeshNumData, QVector3D(0, 0, 0), QVector3D(0, 0, 1));
    _vDataHelper->BuildTexturedStrokeVertexData(_quadMeshes, &_qmTexVbos[0], &_qmTexVaos[0], _qmTexNumbers[0], QuadMeshType::MESH_RECTANGLE);
    _vDataHelper->BuildTexturedStrokeVertexData(_quadMeshes, &_qmTexVbos[1], &_qmTexVaos[1], _qmTexNumbers[1], QuadMeshType::MESH_KITE);
}

// mouse press
void StrokePainter::mousePressEvent(float x, float y)
{
    _isMouseDown = true;

    _oriStrokeLines.clear();
    _selectedPointVao.destroy();

    // Get the closest point within a distance _maxDist
    _selectedIndex = GetClosestIndexFromSpinePoints(AVector(x, y), _maxDist);

    if(_selectedIndex != -1)
    {
        // Selection mode: If we get a point
        _vDataHelper->BuildPointsVertexData(_spineLines, &_selectedPointVbo, &_selectedPointVao, _selectedIndex, QVector3D(0, 0, 0), QVector3D(0.5, 0.5, 1));
    }
    else
    {        
        // Draw mode: we don't
        _oriStrokeLines.push_back(AVector(x, y));
    }
}

// mouse move
void StrokePainter::mouseMoveEvent(float x, float y)
{
    float curveLength = UtilityFunctions::CurveLength(_oriStrokeLines);

    if(_selectedIndex != -1)
    {
        // Selection mode, move the point
        _spineLines[_selectedIndex] = AVector(x, y);

        // Recalculate grids and iterate a step
        CalculateInitialRibbon();
        CalculateVertices();
        ConformalMappingOneStep();

        _vDataHelper->BuildPointsVertexData(_spineLines, &_selectedPointVbo, &_selectedPointVao, _selectedIndex, QVector3D(0, 0, 0), QVector3D(0.5, 0.5, 1));

    }
    else
    {
        // Draw mode
        _oriStrokeLines.push_back(AVector(x, y));
        _vDataHelper->BuildLinesVertexData(_oriStrokeLines, &_oriStrokeLinesVbo, &_oriStrokeLinesVao, QVector3D(0, 0, 0));
    }
}

// mouse release
void StrokePainter::mouseReleaseEvent(float x, float y)
{
    _isMouseDown = false;

    _oriStrokeLines.push_back(AVector(x, y));

    float curveLength = UtilityFunctions::CurveLength(_oriStrokeLines);

    // Ugly conditional !
    if(_selectedIndex == -1 && curveLength > 50)
    {
        _selectedPointVao.destroy();

        _leftLines.clear();
        _rightLines.clear();
        _spineLines.clear();

        CalculateSpines(); // I moved CalculateSpines() from CalculateInitialRibbon()
        CalculateInitialRibbon();
        CalculateVertices();
    }
}

void StrokePainter::Draw()
{
    if(SystemParams::show_mesh && _selectedPointVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glPointSize(10.0f);
        _selectedPointVao.bind();
        glDrawArrays(GL_POINTS, 0, _spineLines.size());
        _selectedPointVao.release();
    }

    /*if(SystemParams::show_mesh && _constrainedPointsVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glPointSize(4.0f);
        _constrainedPointsVao.bind();
        glDrawArrays(GL_POINTS, 0, _constrainedPoints.size());
        _constrainedPointsVao.release();
    }
    */

    if(SystemParams::show_mesh && _debugPointsVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glPointSize(4.0f);
        _debugPointsVao.bind();
        glDrawArrays(GL_POINTS, 0, _debugPoints.size());
        _debugPointsVao.release();
    }
    if(SystemParams::show_mesh && _debugLinesVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glLineWidth(2.0f);
        _debugLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _debugLines.size() * 2);
        _debugLinesVao.release();
    }

    if(_isMouseDown && _oriStrokeLinesVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glLineWidth(2.0f);
        _oriStrokeLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _oriStrokeLines.size() * 2);
        _oriStrokeLinesVao.release();
    }

    if(SystemParams::show_mesh && _spineLinesVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glLineWidth(2.0f);
        _spineLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _spineLines.size() * 2);
        _spineLinesVao.release();
    }

    // Quad mesh
    if(SystemParams::show_mesh && _quadMeshesVao.isCreated())
    {
        glLineWidth(1.0f);
        _quadMeshesVao.bind();
        glDrawArrays(GL_LINES, 0, _qMeshNumData);
        _quadMeshesVao.release();
    }

    // Texture Rectangle
    if(SystemParams::show_texture && _qmTexNumbers[0] > 0)
    {
        _vDataHelper->NeedToDrawWithColor(0.0);

        if(_masterTextures[0]) { _masterTextures[0]->bind(); }
        _qmTexVaos[0].bind();
        glDrawArrays(GL_QUADS, 0, _qmTexNumbers[0]);
        _qmTexVaos[0].release();
        if(_masterTextures[0]) { _masterTextures[0]->release(); }
    }


    // Texture Kite
    if(SystemParams::show_texture && _qmTexNumbers[1] > 0)
    {
        _vDataHelper->NeedToDrawWithColor(0.0);
        if(_masterTextures[1]) { _masterTextures[1]->bind(); }
        _qmTexVaos[1].bind();
        glDrawArrays(GL_QUADS, 0, _qmTexNumbers[1]);
        _qmTexVaos[1].release();
        if(_masterTextures[1]) { _masterTextures[1]->release(); }
    }
}




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
    _iterDist(std::numeric_limits<float>::max()),
    _qMeshNumData(0),
    _masterImages(std::vector<QImage>(2)),              // current only support two textures
    _masterTextures(std::vector<QOpenGLTexture*>(2)),
    _qmTexNumbers(std::vector<int>(2)),
    _qmTexVbos(std::vector<QOpenGLBuffer>(2)),
    _qmTexVaos(std::vector<QOpenGLVertexArrayObject>(2))
{
    //_masterImages = std::vector<QImage>(2);
    //_masterTextures = std::vector<QOpenGLTexture*>(2);
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
    //_aQuadMesh._img.load(img);
    //_aQuadMesh._imgTexture = new QOpenGLTexture(_aQuadMesh._img);
}

void StrokePainter::SetCornerTexture(QString img)
{
    _masterImages[1].load(img);
    _masterTextures[1] = new QOpenGLTexture(_masterImages[1]);
}

/*
void StrokePainter::CalculateInitialRibbon2()
{
    _strokeLines.clear();
    _debugLines.clear();

    std::vector<AVector> tempLine;
    CurveRDP::SimplifyRDP(_oriStrokeLines, tempLine, SystemParams::rdp_epsilon);
    _strokeLines = std::vector<AVector>(tempLine);

    BuildLinesVertexData(_strokeLines, &_strokeLinesVbo, &_strokeLinesVao, QVector3D(0.5, 0.5, 1));
    BuildLinesVertexData(_debugLines, &_debugLinesVbo, &_debugLinesVao, QVector3D(1, 0, 0));
}
*/

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
    CalculateSpines();
    CalculateLeftRightLines();
    CalculateKitesAndRectangles();

    _vDataHelper->BuildLinesVertexData(_spineLines, &_spineLinesVbo, &_spineLinesVao, QVector3D(0.5, 0.5, 1));
    _vDataHelper->BuildLinesVertexData(_leftLines, &_leftLinesVbo, &_leftLinesVao, QVector3D(0.5, 0.5, 1));
    _vDataHelper->BuildLinesVertexData(_rightLines, &_rightLinesVbo, &_rightLinesVao, QVector3D(0.5, 0.5, 1));

    //_vDataHelper->BuildLinesVertexData(_junctionRibLines, &_junctionRibLinesVbo, &_junctionRibLinesVao, QVector3D(0.5, 0.5, 1));
    //_vDataHelper->BuildLinesVertexData(_debugLines, &_debugLinesVbo, &_debugLinesVao, QVector3D(1, 0, 0), QVector3D(0, 1, 0)); // modification
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
    //_debugLines.clear();
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
                // turn right: positive
                AVector lMid = _leftLines[a];
                AVector rMid = _rightLines[a];
                AVector lStart = rMid + AVector(dir1.y, -dir1.x) * strokeWidth;   // left
                AVector lEnd = rMid + AVector(dir2.y, -dir2.x) * strokeWidth; // left

                QuadMesh qMesh;
                qMesh._leftStartPt  = lStart;
                qMesh._leftEndPt    = lMid;
                qMesh._rightStartPt = rMid;
                qMesh._rightEndPt   = lEnd;
                qMesh._sharpPt = lMid;
                qMesh._isRightKite = true;
                //qMesh._leftLine   = ALine(lStart, rMid);
                //qMesh._rightLine  = ALine(lMid,   lEnd);
                //qMesh._topLine    = ALine(lStart, lMid);
                //qMesh._bottomLine = ALine(rMid,   lEnd);
                qMesh._quadMeshType = QuadMeshType::MESH_KITE;
                _quadMeshes.push_back(qMesh);
                //std::cout << "kite " << a <<  "\n";
                //_debugLines.push_back(ALine(lStart,  rMid));
                //_debugLines.push_back(ALine(lEnd, rMid));
                //_debugLines.push_back(ALine(lStart, lMid));
                //_debugLines.push_back(ALine(lMid, lEnd));

            }
            else if(rot1 < 0)
            {
                // turn left: negative
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
                //qMesh._leftLine   = ALine(rStart, rMid);
                //qMesh._rightLine  = ALine(lMid,   rEnd);
                //qMesh._topLine    = ALine(rStart, lMid);
                //qMesh._bottomLine = ALine(rMid,   rEnd);
                qMesh._quadMeshType = QuadMeshType::MESH_KITE;
                _quadMeshes.push_back(qMesh);

                //std::cout << "kite " << a <<  "\n";
                //_debugLines.push_back(ALine(lMid,  rStart));
                //_debugLines.push_back(ALine(lMid,  rEnd));
                //_debugLines.push_back(ALine(rStart,  rMid));
                //_debugLines.push_back(ALine(rMid,  rEnd));
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
                // turn right: positive. use right
                AVector leftDir(dir1.y, -dir1.x);
                leftEnd = _rightLines[a+1] + leftDir * strokeWidth;
            }
            else if(rot < 0)
            {
                // turn left: negative. use left
                AVector rightDir(-dir1.y, dir1.x);
                rightEnd = _leftLines[a+1] + rightDir * strokeWidth;
            }

            QuadMesh qMesh;
            qMesh._leftStartPt  = _leftLines[a];
            qMesh._leftEndPt    = leftEnd;
            qMesh._rightStartPt = _rightLines[a];
            qMesh._rightEndPt   = rightEnd;
            //qMesh._leftLine   = ALine(_leftLines[a], _rightLines[a]);
            //qMesh._rightLine  = ALine(leftEnd, rightEnd);
            //qMesh._topLine    = ALine(_leftLines[a], leftEnd);
            //qMesh._bottomLine = ALine(_rightLines[a], rightEnd);
            qMesh._quadMeshType = QuadMeshType::MESH_RECTANGLE;
            _quadMeshes.push_back(qMesh);
            //std::cout << "rectangle " << a <<  "\n";
            //_debugLines.push_back(ALine(_leftLines[a], leftEnd));
            //_debugLines.push_back(ALine(_rightLines[a], rightEnd));
            //_debugLines.push_back(ALine(_leftLines[a], _rightLines[a]));
            //_debugLines.push_back(ALine(leftEnd, rightEnd));

        }
        else if(a == 0 && _spineLines.size() == 2)  // START
        {
            QuadMesh qMesh;
            qMesh._leftStartPt  = _leftLines[a];
            qMesh._leftEndPt    = _leftLines[a+1];
            qMesh._rightStartPt = _rightLines[a];
            qMesh._rightEndPt   = _rightLines[a+1];
            //qMesh._leftLine   = ALine(_leftLines[a],   _rightLines[a]);
            //qMesh._rightLine  = ALine(_leftLines[a+1], _rightLines[a+1]);
            //qMesh._topLine    = ALine(_leftLines[a],   _leftLines[a+1]);
            //qMesh._bottomLine = ALine(_rightLines[a],  _rightLines[a+1]);
            qMesh._quadMeshType = QuadMeshType::MESH_RECTANGLE;
            _quadMeshes.push_back(qMesh);
            //std::cout << "rectangle " << a <<  "\n";
            //_debugLines.push_back(ALine(_leftLines[a], _leftLines[a+1]));
            //_debugLines.push_back(ALine(_rightLines[a], _rightLines[a+1]));
            //_debugLines.push_back(ALine(_leftLines[a], _rightLines[a]));
            //_debugLines.push_back(ALine(_leftLines[a+1], _rightLines[a+1]));
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
                // turn right: positive. use right
                AVector leftDir(dir2.y, -dir2.x);
                leftStart = _rightLines[a] + leftDir * strokeWidth;
            }
            else if(rot < 0)
            {
                // turn left: negative. use left
                AVector rightDir(-dir2.y, dir2.x);
                rightStart = _leftLines[a] + rightDir * strokeWidth;
            }

            QuadMesh qMesh;
            qMesh._leftStartPt  = leftStart;
            qMesh._leftEndPt    = _leftLines[a+1];
            qMesh._rightStartPt = rightStart;
            qMesh._rightEndPt   = _rightLines[a+1];
            //qMesh._leftLine   = ALine(leftStart, rightStart);
            //qMesh._rightLine  = ALine(_leftLines[a+1], _rightLines[a+1]);
            //qMesh._topLine    = ALine(leftStart, _leftLines[a+1]);
            //qMesh._bottomLine = ALine(rightStart, _rightLines[a+1]);
            qMesh._quadMeshType = QuadMeshType::MESH_RECTANGLE;
            _quadMeshes.push_back(qMesh);

            //std::cout << "rectangle " << a <<  "\n";
            //_debugLines.push_back(ALine(leftStart, _leftLines[a+1]));
            //_debugLines.push_back(ALine(rightStart, _rightLines[a+1]));
            //_debugLines.push_back(ALine(leftStart, rightStart));
            //_debugLines.push_back(ALine(_leftLines[a+1], _rightLines[a+1]));
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
            //qMesh._leftLine   = ALine(leftStart,  rightStart);
            //qMesh._rightLine  = ALine(leftEnd,    rightEnd);
            //qMesh._topLine    = ALine(leftStart,  leftEnd);
            //qMesh._bottomLine = ALine(rightStart, rightEnd);
            qMesh._quadMeshType = QuadMeshType::MESH_RECTANGLE;
            _quadMeshes.push_back(qMesh);
            //std::cout << "rectangle " << a <<  "\n";
            //_debugLines.push_back(ALine(leftStart,  leftEnd));
            //_debugLines.push_back(ALine(rightStart, rightEnd));
            //_debugLines.push_back(ALine(leftStart,  rightStart));
            //_debugLines.push_back(ALine(leftEnd,    rightEnd));
        }
    }

    //std::cout << "\n\n";
}

void StrokePainter::CalculateVertices2(QuadMesh* qMesh)
{
    qMesh->_plusSignVertices.clear();

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
    if(qMesh->_quadMeshType == QuadMeshType::MESH_KITE)
    {
        meshSize /= 2.0f;
    }

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
                    _debugPoints.push_back(pt);
                    shouldMove = false;
                }
            }

            if(SystemParams::fixed_separation_constraint)
            {
                if(qMesh->_quadMeshType == QuadMeshType::MESH_KITE)
                {
                    if(pt.Distance(qMesh->_sharpPt) < std::numeric_limits<float>::epsilon() * 100)
                    {
                        _debugPoints.push_back(pt);
                        shouldMove = false;
                    }
                    else if(qMesh->_isRightKite && (xIter == 0 || yIter == yLoop - 1))
                    {
                        _debugPoints.push_back(pt);
                        shouldMove = false;
                    }
                    else if(!qMesh->_isRightKite && (xIter == xLoop - 1 || yIter == 0))
                    {
                        _debugPoints.push_back(pt);
                        shouldMove = false;
                    }
                }
            }

            PlusSignVertex psVert = PlusSignVertex(pt, shouldMove, junctionRibsConstrained, spinesConstrained);
            columnVertices.push_back(psVert);
        }
        qMesh->_plusSignVertices.push_back(columnVertices);
    }
}


// global conformal mapping
// so it does not consider corners as special cases
void StrokePainter::CalculateVertices1()
{
    //_aQuadMesh._plusSignVertices.clear();

    for(uint a = 0; a < _spineLines.size() - 1; a++)
    {
        AVector mStartPt = _spineLines[a];
        AVector mEndPt   = _spineLines[a + 1];

        // no corner avoidance
        int intMeshHeight = SystemParams::stroke_width / SystemParams::mesh_size;
        int intMeshWidth = mStartPt.Distance(mEndPt) / SystemParams::mesh_size;

        //if(SystemParams::junction_ribs_constraint)
        {
            // with corner avoidance
            intMeshHeight = SystemParams::stroke_width / SystemParams::mesh_size;
            intMeshWidth =  (int)(mStartPt.Distance(mEndPt) / SystemParams::stroke_width) * intMeshHeight;
        }

        AVector lStartPt = _leftLines[a];
        AVector lEndPt   = _leftLines[a + 1];

        AVector rStartPt = _rightLines[a];
        AVector rEndPt   = _rightLines[a + 1];

        AVector vVec = rStartPt - lStartPt;

        AVector uHVec = (lEndPt - lStartPt);
        AVector bHVec = (rEndPt - rStartPt);

        int xLoop = intMeshWidth;
        int yLoop = intMeshHeight + 1;

        bool isTheEnd = false;
        if(a == _spineLines.size() - 2)
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
                bool junctionRibsConstrained = false;
                bool spinesConstrained = false;
                /*if(SystemParams::miter_joint_constraint && xIter == 0 && yIter == 0)
                    { shouldMove = false; }
                else if(SystemParams::miter_joint_constraint && xIter == 0 && yIter == yLoop - 1 )
                    { shouldMove = false; }
                else if(SystemParams::miter_joint_constraint && isTheEnd && xIter == xLoop - 1 && yIter == 0)
                    { shouldMove = false; }
                else if(SystemParams::miter_joint_constraint && isTheEnd && xIter == xLoop - 1 && yIter == yLoop - 1 )
                    { shouldMove = false; }
                */

                //if(SystemParams::junction_ribs_constraint &&  a > 0 && xIter == 0)
                //    { junctionRibsConstrained = true; }

                // odd only
                /*if(SystemParams::spines_constraint && yLoop % 2 != 0)
                {
                    int yMid = yLoop / 2;
                    if(yIter == yMid)
                        { spinesConstrained = true; }
                }*/

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
                columnVertices.push_back(PlusSignVertex(pt, shouldMove, junctionRibsConstrained, spinesConstrained));
            }
            //_aQuadMesh._plusSignVertices.push_back(columnVertices);
        }
    }

    /*
    // set mesh size
    _aQuadMesh._mesh_width = _aQuadMesh._plusSignVertices.size();
    _aQuadMesh._mesh_height = _aQuadMesh._plusSignVertices[0].size();

    // add border
    _aQuadMesh._borderLines.clear();
    _aQuadMesh.AddBorders(_rightLines);
    _aQuadMesh.AddBorders(_leftLines);
    _aQuadMesh.AddBorder(ALine(_leftLines[0], _rightLines[0]));
    _aQuadMesh.AddBorder(ALine(_leftLines[_leftLines.size() - 1], _rightLines[_rightLines.size() - 1]));

    // don't create vertex data here
    _vDataHelper->BuildLinesVertexData(_aQuadMesh._plusSignVertices, &_aQuadMesh._plusSignVerticesVbo, &_aQuadMesh._plusSignVerticesVao, _aQuadMesh._mesh_width, _aQuadMesh._mesh_height, QVector3D(1, 0, 0));
    _vDataHelper->BuildTexturedStrokeVertexData(_aQuadMesh._plusSignVertices, &_aQuadMesh._texturedStrokeVbo, &_aQuadMesh._texturedStrokeVao, _aQuadMesh._mesh_width, _aQuadMesh._mesh_height);
    //_vDataHelper->BuildConstrainedPointsVertexData(_aQuadMesh._plusSignVertices, &_constrainedPointsVbo, &_constrainedPointsVao, &_numConstrainedPoints, _aQuadMesh._mesh_width, _aQuadMesh._mesh_height, QVector3D(0.5, 0.5, 1));
    */
}

/*AVector StrokePainter::GetClosestPointFromMiddleVerticalLines(AVector pt)
{
    AVector closestPt = pt;
    float dist = std::numeric_limits<float>::max();
    for(int a = 0; a < _junctionRibLines.size(); a++)
    {
        AVector pt1 = _junctionRibLines[a].GetPointA();
        AVector pt2 = _junctionRibLines[a].GetPointB();
        AVector cPt = UtilityFunctions::GetClosestPoint(pt1, pt2, pt);
        if(pt.Distance(cPt) < dist)
        {
            dist = pt.Distance(cPt);
            closestPt = cPt;
        }
    }
    return closestPt;
}*/

AVector StrokePainter::GetClosestPointFromStrokePoints(AVector pt)
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

AVector StrokePainter::GetClosestPointFromStrokeLines(AVector pt)
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

AVector StrokePainter::GetClosestPointFromBorders(QuadMesh qMesh, AVector pt)
{
    AVector closestPt = pt;
    float dist = std::numeric_limits<float>::max();
    std::vector<ALine> borderLines;
    borderLines.push_back(ALine(qMesh._leftStartPt,  qMesh._rightStartPt));
    borderLines.push_back(ALine(qMesh._leftEndPt,    qMesh._rightEndPt));
    borderLines.push_back(ALine(qMesh._leftStartPt,  qMesh._leftEndPt));
    borderLines.push_back(ALine(qMesh._rightStartPt, qMesh._rightEndPt));
    for(uint a = 0; a < borderLines.size(); a++)
    {
        AVector cPt = UtilityFunctions::GetClosestPoint(borderLines[a].GetPointA(), borderLines[a].GetPointB(), pt);
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
    for(uint a = 0; a < _leftLines.size() - 1; a++)
    {
        AVector pt1 = _leftLines[a];
        AVector pt2 = _leftLines[a+1];
        AVector cPt = UtilityFunctions::GetClosestPoint(pt1, pt2, pt);
        if(pt.Distance(cPt) < dist)
        {
            dist = pt.Distance(cPt);
            closestPt = cPt;
        }
    }

    // right
    for(uint a = 0; a < _rightLines.size() - 1; a++)
    {
        AVector pt1 = _rightLines[a];
        AVector pt2 = _rightLines[a+1];
        AVector cPt = UtilityFunctions::GetClosestPoint(pt1, pt2, pt);
        if(pt.Distance(cPt) < dist)
        {
            dist = pt.Distance(cPt);
            closestPt = cPt;
        }
    }

    // start
    AVector s1 = _leftLines[0];
    AVector s2 = _rightLines[0];
    AVector scPt = UtilityFunctions::GetClosestPoint(s1, s2, pt);
    if(pt.Distance(scPt) < dist)
    {
        dist = pt.Distance(scPt);
        closestPt = scPt;
    }

    // end
    AVector e1 = _leftLines[_leftLines.size() - 1];
    AVector e2 = _rightLines[_rightLines.size() - 1];
    AVector ecPt = UtilityFunctions::GetClosestPoint(e1, e2, pt);
    if(pt.Distance(ecPt) < dist)
    {
        dist = pt.Distance(ecPt);
        closestPt = ecPt;
    }

    return closestPt;
}

void StrokePainter::ConformalMappingOneStep3()
{
    this->_iterDist = 0;
    for(uint a = 0; a < _quadMeshes.size(); a++)
    {
        if(_quadMeshes[a]._quadMeshType == QuadMeshType::MESH_KITE)
        {
            ConformalMappingOneStep3(&_quadMeshes[a]);
        }
    }
    _qMeshNumData = 0;
    _vDataHelper->BuildLinesVertexData(_quadMeshes, &_quadMeshesVbo, &_quadMeshesVao, _qMeshNumData, QVector3D(0, 0, 0), QVector3D(0, 0, 1));
    _vDataHelper->BuildTexturedStrokeVertexData(_quadMeshes, &_qmTexVbos[0], &_qmTexVaos[0], _qmTexNumbers[0], QuadMeshType::MESH_RECTANGLE);
    _vDataHelper->BuildTexturedStrokeVertexData(_quadMeshes, &_qmTexVbos[1], &_qmTexVaos[1], _qmTexNumbers[1], QuadMeshType::MESH_KITE);
}

void StrokePainter::ConformalMappingOneStep3(QuadMesh* qMesh)
{
    std::vector<std::vector<PlusSignVertex>> tempVertices = qMesh->_plusSignVertices;
    int meshWidth = qMesh->_plusSignVertices.size();
    int meshHeight = qMesh->_plusSignVertices[0].size();

    for(int a = 0; a < meshWidth; a++)
    {
        for(int b = 0; b < meshHeight; b++)
        {
            if(!tempVertices[a][b].shouldMove)
                { continue; }

            AVector curPos = tempVertices[a][b].position;
            AVector sumPositions(0, 0);
            float sumArmLengths = 0;
            float sumArmAngles = 0;
            int numNeighbor = 0;

            // have left and right
            if(a > 0 && a < meshWidth - 1)
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
            else if(a < meshWidth - 1)
            {
                PlusSignVertex rVertex = tempVertices[a + 1][b];
                AVector fakeNeighbor = rVertex.position + UtilityFunctions::Rotate( AVector(-1, 0) * rVertex.armLength, rVertex.angle);

                sumPositions += fakeNeighbor;
                sumArmLengths += curPos.Distance(rVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(1, 0), (rVertex.position - curPos).Norm());

                numNeighbor++;
            }

            // have up and down
            if(b > 0 && b < meshHeight - 1)
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
            else if(b < meshHeight - 1)
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
                tempVertices[a][b].position = GetClosestPointFromBorders(*qMesh, sumPositions);
            }
            else
            {
                tempVertices[a][b].position = sumPositions;
            }

            /*
            if(numNeighbor < 4)
                { tempVertices[a][b].position = GetClosestPointFromBorders(sumPositions); }
            else if(tempVertices[a][b].midHorizontalConstrained && tempVertices[a][b].midVerticalConstrained)
                { tempVertices[a][b].position = GetClosestPointFromStrokePoints(sumPositions); }
            else if(tempVertices[a][b].midHorizontalConstrained)
                { tempVertices[a][b].position = GetClosestPointFromStrokeLines(sumPositions); }
            //else if(tempVertices[a][b].midVerticalConstrained)
            //    { tempVertices[a][b].position = GetClosestPointFromMiddleVerticalLines(sumPositions); }
            else
                { tempVertices[a][b].position = sumPositions; }*/
        }
    }

    float sumDist = 0;
    for(int a = 0; a < meshWidth; a++)
    {
        for(int b = 0; b < meshHeight; b++)
            { sumDist += qMesh->_plusSignVertices[a][b].position.Distance(tempVertices[a][b].position); }
    }
    _iterDist += sumDist;
    qMesh->_plusSignVertices = tempVertices;
}

void StrokePainter::ConformalMappingOneStep2()
{
    /*
    std::vector<std::vector<PlusSignVertex>> tempVertices = _aQuadMesh._plusSignVertices;
    for(int a = 0; a < _aQuadMesh._mesh_width; a++)
    {
        for(int b = 0; b < _aQuadMesh._mesh_height; b++)
        {
            if(!tempVertices[a][b].shouldMove)
                { continue; }

            AVector curPos = tempVertices[a][b].position;
            AVector sumPositions(0, 0);
            float sumArmLengths = 0;
            float sumArmAngles = 0;
            int numNeighbor = 0;

            // have left and right
            if(a > 0 && a < _aQuadMesh._mesh_width - 1)
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
            else if(a < _aQuadMesh._mesh_width - 1)
            {
                PlusSignVertex rVertex = tempVertices[a + 1][b];
                AVector fakeNeighbor = rVertex.position + UtilityFunctions::Rotate( AVector(-1, 0) * rVertex.armLength, rVertex.angle);

                sumPositions += fakeNeighbor;
                sumArmLengths += curPos.Distance(rVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(1, 0), (rVertex.position - curPos).Norm());

                numNeighbor++;
            }

            // have up and down
            if(b > 0 && b < _aQuadMesh._mesh_height - 1)
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
            else if(b < _aQuadMesh._mesh_height - 1)
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
                { tempVertices[a][b].position = GetClosestPointFromBorders(sumPositions); }
            else if(tempVertices[a][b].midHorizontalConstrained && tempVertices[a][b].midVerticalConstrained)
                { tempVertices[a][b].position = GetClosestPointFromStrokePoints(sumPositions); }
            else if(tempVertices[a][b].midHorizontalConstrained)
                { tempVertices[a][b].position = GetClosestPointFromStrokeLines(sumPositions); }
            else if(tempVertices[a][b].midVerticalConstrained)
                { tempVertices[a][b].position = GetClosestPointFromMiddleVerticalLines(sumPositions); }
            else
                { tempVertices[a][b].position = sumPositions; }
        }
    }

    float sumDist = 0;
    for(int a = 0; a < _aQuadMesh._mesh_width; a++)
    {
        for(int b = 0; b < _aQuadMesh._mesh_height; b++)
            { sumDist += _aQuadMesh._plusSignVertices[a][b].position.Distance(tempVertices[a][b].position); }
    }
    _iterDist = sumDist;

    // modification
    _aQuadMesh._plusSignVertices = tempVertices;
    _vDataHelper->BuildLinesVertexData(_aQuadMesh._plusSignVertices, &_aQuadMesh._plusSignVerticesVbo, &_aQuadMesh._plusSignVerticesVao, _aQuadMesh._mesh_width, _aQuadMesh._mesh_height, QVector3D(1, 0, 0));
    _vDataHelper->BuildTexturedStrokeVertexData(_aQuadMesh._plusSignVertices, &_aQuadMesh._texturedStrokeVbo, &_aQuadMesh._texturedStrokeVao, _aQuadMesh._mesh_width, _aQuadMesh._mesh_height);
    _vDataHelper->BuildConstrainedPointsVertexData(_aQuadMesh._plusSignVertices, &_constrainedPointsVbo, &_constrainedPointsVao, &_numConstrainedPoints, _aQuadMesh._mesh_width, _aQuadMesh._mesh_height, QVector3D(0.5, 0.5, 1));
    //_vDataHelper->BuildLinesVertexData(_debugLines, &_debugLinesVbo, &_debugLinesVao, QVector3D(0, 0, 1));
    //_vDataHelper->BuildPointsVertexData(_debugPoints, &_debugPointsVbo, &_debugPointsVao, QVector3D(0, 0.5, 0));
    */
}

// a very simple version
void StrokePainter::ConformalMappingOneStep1()
{
    /*
    std::vector<std::vector<PlusSignVertex>> tempVertices = _aQuadMesh._plusSignVertices;

    for(int a = 0; a < _aQuadMesh._mesh_width; a++)
    {
        for(int b = 0; b < _aQuadMesh._mesh_height; b++)
        {
            AVector sumPositions(0, 0);
            int numNeighbor = 0;

            // left
            if(a > 0)
            {
                sumPositions += tempVertices[a - 1][b].position;
                numNeighbor++;
            }

            // right
            if(a < _aQuadMesh._mesh_width - 1)
            {
                sumPositions = sumPositions + tempVertices[a + 1][b].position;
                numNeighbor++;
            }

            // up
            if(b > 0)
            {
                sumPositions += tempVertices[a][b - 1].position;
                numNeighbor++;
            }

            // bottom
            if(b < _aQuadMesh._mesh_height - 1)
            {
                sumPositions += tempVertices[a][b + 1].position;
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
    for(int a = 0; a < _aQuadMesh._mesh_width; a++)
    {
        for(int b = 0; b < _aQuadMesh._mesh_height; b++)
        {
            sumDist += _aQuadMesh._plusSignVertices[a][b].position.Distance(tempVertices[a][b].position);
        }
    }
    _iterDist = sumDist;
    _aQuadMesh._plusSignVertices = tempVertices;
    _vDataHelper->BuildLinesVertexData(_aQuadMesh._plusSignVertices, &_aQuadMesh._plusSignVerticesVbo, &_aQuadMesh._plusSignVerticesVao, _aQuadMesh._mesh_width, _aQuadMesh._mesh_height, QVector3D(1, 0, 0));
    */
}

// mouse press
void StrokePainter::mousePressEvent(float x, float y)
{
    _isMouseDown = true;

    //_vertices.clear();
    //_aQuadMesh._plusSignVertices.clear();

    //_debugLines.clear();
    //_junctionRibLines.clear();
    _leftLines.clear();
    _rightLines.clear();

    //_numConstrainedPoints = 0;

    _spineLines.clear();
    _oriStrokeLines.clear();
    _oriStrokeLines.push_back(AVector(x, y));
}

// mouse move
void StrokePainter::mouseMoveEvent(float x, float y)
{
    _oriStrokeLines.push_back(AVector(x, y));
    _spineLines = std::vector<AVector>(_oriStrokeLines);
    _vDataHelper->BuildLinesVertexData(_spineLines, &_spineLinesVbo, &_spineLinesVao, QVector3D(0, 0, 0));
}

// mouse release
void StrokePainter::mouseReleaseEvent(float x, float y)
{
    _isMouseDown = false;

    _oriStrokeLines.push_back(AVector(x, y));
    CalculateInitialRibbon();    // modification
    //CalculateVertices1(); // modification
    //CalculateVertices2(); // modification

    //_debugLines.clear();
    _debugPoints.clear();
    for(uint a = 0; a < _quadMeshes.size(); a++)
    {
        CalculateVertices2(&_quadMeshes[a]);
    }
    _qMeshNumData = 0;
    //_vDataHelper->BuildLinesVertexData(_debugLines, &_debugLinesVbo, &_debugLinesVao, QVector3D(1, 0, 0));
    _vDataHelper->BuildPointsVertexData(_debugPoints, &_debugPointsVbo, &_debugPointsVao, QVector3D(1, 0, 0));
    _vDataHelper->BuildLinesVertexData(_quadMeshes, &_quadMeshesVbo, &_quadMeshesVao, _qMeshNumData, QVector3D(0, 0, 0), QVector3D(0, 0, 1));
    //std::cout << "\n\n";
    //std::cout << "_qMeshNumData " << _qMeshNumData << "\n";
}

void StrokePainter::Draw()
{
    // modification
    if((_isMouseDown || SystemParams::show_mesh ) && _spineLinesVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glLineWidth(2.0f);
        _spineLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _spineLines.size() * 2);
        _spineLinesVao.release();
    }

    /*if(SystemParams::junction_ribs_constraint && _junctionRibLinesVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glLineWidth(2.0f);
        _junctionRibLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _junctionRibLines.size() * 2);
        _junctionRibLinesVao.release();
    }*/

    /*if(SystemParams::show_mesh && SystemParams::miter_joint_constraint && _constrainedPointsVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glPointSize(10.0f);
        _constrainedPointsVao.bind();
        glDrawArrays(GL_POINTS, 0, _numConstrainedPoints);
        _constrainedPointsVao.release();
    }*/

    // Quad mesh
    if(SystemParams::show_mesh && _quadMeshesVao.isCreated())
    {
        glLineWidth(1.0f);
        _quadMeshesVao.bind();
        glDrawArrays(GL_LINES, 0, _qMeshNumData);
        _quadMeshesVao.release();
    }

    if(SystemParams::show_mesh && _debugPointsVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glPointSize(4.0f);
        _debugPointsVao.bind();
        glDrawArrays(GL_POINTS, 0, _debugPoints.size());
        _debugPointsVao.release();
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

    // Texture
    /*if(SystemParams::show_texture && _quadMeshesTexVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(0.0);

        if(_masterTextures[0]) { _masterTextures[0]->bind(); }

        _quadMeshesTexVao.bind();
        glDrawArrays(GL_QUADS, 0, _qMeshTexNumData);
        _quadMeshesTexVao.release();

        if(_masterTextures[0]) { _masterTextures[0]->release(); }
    }*/

    /*
    // Quad mesh
    if(SystemParams::show_mesh && _aQuadMesh._plusSignVerticesVao.isCreated() && _aQuadMesh._plusSignVertices.size() > 0)
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        int wMin1 = _aQuadMesh._mesh_width - 1;
        int hMin1 = _aQuadMesh._mesh_height - 1;
        int meshSize = ((wMin1 * _aQuadMesh._mesh_width) + (hMin1 * _aQuadMesh._mesh_height)) * 2;
        glLineWidth(1.0f);
        _aQuadMesh._plusSignVerticesVao.bind();
        glDrawArrays(GL_LINES, 0, meshSize);
        _aQuadMesh._plusSignVerticesVao.release();
    }
    */

    /*
    // Texture
    if(SystemParams::show_texture && _aQuadMesh._texturedStrokeVao.isCreated() && _aQuadMesh._plusSignVertices.size() > 0)
    {
        _vDataHelper->NeedToDrawWithColor(0.0);
        int meshSize = (_aQuadMesh._mesh_width - 1) * (_aQuadMesh._mesh_height - 1) * 4;
        glLineWidth(1.0f);
        if(_aQuadMesh._imgTexture) { _aQuadMesh._imgTexture->bind(); }
        _aQuadMesh._texturedStrokeVao.bind();
        glDrawArrays(GL_QUADS, 0, meshSize);
        _aQuadMesh._texturedStrokeVao.release();
        if(_aQuadMesh._imgTexture) { _aQuadMesh._imgTexture->release(); }
    }
    */

    /*if(_debugLinesVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glLineWidth(4.0f);
        _debugLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _debugLines.size() * 2);
        _debugLinesVao.release();
    }*/


    /*
    if(_strokeLinesVao.isCreated() && _plusSignVertices.size() == 0)
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glLineWidth(2.0f);
        _strokeLinesVao.bind();
        glDrawArrays(GL_LINES, 0, (_strokeLines.size() - 1) * 2);
        _strokeLinesVao.release();
    }*/

    /*
    if(_pointsVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glPointSize(10.0f);
        _pointsVao.bind();
        glDrawArrays(GL_POINTS, 0, _points.size());
        _pointsVao.release();
    }*/

    /*if(_verticesVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glPointSize(10.0f);
        _verticesVao.bind();
        glDrawArrays(GL_POINTS, 0, _vertices.size());
        _verticesVao.release();
    }*/

    /*
    if(_leftLinesVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);

        glLineWidth(2.0f);
        _leftLinesVao.bind();
        glDrawArrays(GL_LINES, 0, (_leftLines.size() - 1) * 2);
        _leftLinesVao.release();
    }
    */
    /*
    if(_rightLinesVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glLineWidth(2.0f);
        _rightLinesVao.bind();
        glDrawArrays(GL_LINES, 0, (_rightLines.size() - 1) * 2);
        _rightLinesVao.release();
    }
    */

    /*
    if(_ribLinesVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glLineWidth(2.0f);
        _ribLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _ribLines.size() * 2);
        _ribLinesVao.release();
    }

    if(_gridLinesVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glLineWidth(2.0f);
        _gridLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _gridLines.size() * 2);
        _gridLinesVao.release();
    }
    */
}



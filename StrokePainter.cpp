
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
    _images(std::vector<QImage>(3)),              // support three textures
    _oglTextures(std::vector<QOpenGLTexture*>(3)),
    _vertexNumbers(std::vector<int>(3)),
    _textureSizes(std::vector<QSizeF>(3)),
    _texVbos(std::vector<QOpenGLBuffer>(3)),
    _texVaos(std::vector<QOpenGLVertexArrayObject>(3)),
    _selectedIndex(-1),
    _maxDist(2.0f)
{
    /*
    _rectMeshesColor = QVector3D(0, 0, 0);
    _legMeshesColor      = QVector3D(0.25, 0.75, 0.25);
    _kiteMeshesColor      = QVector3D(0, 0, 1);
    */

    _rectMeshesColor = QVector3D(1, 1, 1);
    _legMeshesColor      = QVector3D(0.5, 0.75, 0.5);
    _kiteMeshesColor      = QVector3D(0.5, 0.5, 1.0);

    _selectedPointColor   = QVector3D(0.5, 0.5, 1.0);
    _unselectedPointColor = QVector3D(1, 1, 1);

    _oriStrokeColor  = QVector3D(1, 1, 1);
    _spineLinesColor = QVector3D(0.5, 0.5, 1);

    _constrainedPointColor = QVector3D(1, 0, 0);

    _debugPointsColor = QVector3D(1, 0, 0);
}

StrokePainter::~StrokePainter()
{
    if(_vDataHelper) delete _vDataHelper;
}

// this function is awkward...
void StrokePainter::SetVertexDataHelper(QOpenGLShaderProgram* shaderProgram)
{
    _vDataHelper = new VertexDataHelper(shaderProgram);
}

void StrokePainter::SetKiteTexture(QString img)
{
    _images[0].load(img);
    QImage qImg = _images[0];
    _oglTextures[0] = new QOpenGLTexture(qImg);
    float length = ((float)qImg.width()) / ((float)qImg.height()) * SystemParams::stroke_width;
    _textureSizes[0] = QSizeF(length, SystemParams::stroke_width);
}

void StrokePainter::SetLegTexture(QString img)
{
    _images[1].load(img);
    QImage qImg = _images[1];
    _oglTextures[1] = new QOpenGLTexture(qImg);
    float length = ((float)qImg.width()) / ((float)qImg.height()) * SystemParams::stroke_width;
    _textureSizes[1] = QSizeF(length, SystemParams::stroke_width);
}

void StrokePainter::SetRectilinearTexture(QString img)
{
    _images[2].load(img);
    QImage qImg = _images[2];
    _oglTextures[2] = new QOpenGLTexture(qImg);
    float length = ((float)qImg.width()) / ((float)qImg.height()) * SystemParams::stroke_width;
    _textureSizes[2] = QSizeF(length, SystemParams::stroke_width);
}


void StrokePainter::CalculateInitialLeftRightLines()
{
    float halfStrokeWidth = SystemParams::stroke_width / 2.0f;

    // calculate left and right lines;
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
    // to do: weird error checking
    if(_spineLines.size() == 0)
    {
        std::cout << "you need to draw something\n";
        return;
    }

    //CalculateSpines(); // I removed this, but I'm not sure whether I should bring it back...

    CalculateInitialLeftRightLines();
    CalculateInitialSegments();
    DecomposeSegments();

    // delete this
    //_vDataHelper->BuildPointsVertexData(_debugPoints, &_debugPointsVbo, &_debugPointsVao, _debugPointsColor);
    //_vDataHelper->BuildLinesVertexData(_debugLines, &_debugLinesVbo, &_debugLinesVao, QVector3D(0, 0.25, 0));

    _vDataHelper->BuildLinesVertexData(_spineLines, &_spineLinesVbo, &_spineLinesVao, _spineLinesColor);
    //_vDataHelper->BuildLinesVertexData(_leftLines, &_leftLinesVbo, &_leftLinesVao, QVector3D(0.5, 0.5, 1));
    //_vDataHelper->BuildLinesVertexData(_rightLines, &_rightLinesVbo, &_rightLinesVao, QVector3D(0.5, 0.5, 1));
}

void StrokePainter::CalculateSpines()
{
    _spineLines.clear();
    std::vector<AVector> tempLine;
    CurveRDP::SimplifyRDP(_oriStrokeLines, tempLine, SystemParams::rdp_epsilon);
    _spineLines = std::vector<AVector>(tempLine);
}

void StrokePainter::DecomposeSegments()
{
    std::vector<QuadMesh> tempQuadMeshes = _quadMeshes;
    std::vector<AVector>  tempLeftLines  = _leftLines;
    std::vector<AVector>  tempRightLines = _rightLines;

    _quadMeshes.clear();
    _leftLines.clear();
    _rightLines.clear();

    float legLength = _textureSizes[1].width();
    float rectilinearLength = _textureSizes[2].width();

    float rectFactor = 0.5f;

    for(uint a = 0; a < tempQuadMeshes.size(); a++)
    {
        QuadMesh qMesh = tempQuadMeshes[a];
        if(qMesh._quadMeshType == QuadMeshType::MESH_KITE)
        {
            _quadMeshes.push_back(qMesh);
            _leftLines.push_back(tempLeftLines[a]);
            _rightLines.push_back(tempRightLines[a]);

            continue;
        }

        ALine topLine(qMesh._leftStartPt,   qMesh._leftEndPt);
        ALine bottomLine(qMesh._rightStartPt, qMesh._rightEndPt);

        float spineLength = qMesh._leftStartPt.Distance(qMesh._leftEndPt);
        if(a == 0)
        {
            // to do: if(spineLength < legLength + (rectilinearLength * rectFactor)) --> leg leg

            AVector left2 = topLine.GetPointInBetween((spineLength - legLength) / spineLength);
            AVector right2 = bottomLine.GetPointInBetween((spineLength - legLength) / spineLength);

            // rect
            QuadMesh qMesh1(qMesh._leftStartPt,  left2, qMesh._rightStartPt, right2, QuadMeshType::MESH_RECTILINEAR);
            _quadMeshes.push_back(qMesh1);

            // leg
            QuadMesh qMesh2(left2,  qMesh._leftEndPt, right2, qMesh._rightEndPt, QuadMeshType::MESH_LEG);
            _quadMeshes.push_back(qMesh2);
        }
        else if(a == tempQuadMeshes.size() - 1)
        {
            // to do: if(spineLength < legLength + (rectilinearLength * rectFactor)) --> leg leg

            AVector left1 = topLine.GetPointInBetween(legLength / spineLength);
            AVector right1 = bottomLine.GetPointInBetween(legLength / spineLength);

            // leg
            QuadMesh qMesh1(qMesh._leftStartPt,  left1, qMesh._rightStartPt, right1, QuadMeshType::MESH_LEG);
            _quadMeshes.push_back(qMesh1);

            // rect
            QuadMesh qMesh2(left1,  qMesh._leftEndPt, right1, qMesh._rightEndPt, QuadMeshType::MESH_RECTILINEAR);
            _quadMeshes.push_back(qMesh2);
        }
        else if(spineLength < ((legLength * 2.0f) + (rectilinearLength * rectFactor)))
        {
            AVector leftMid  = topLine.GetMiddlePoint();
            AVector rightMid = bottomLine.GetMiddlePoint();

            // leg
            QuadMesh qMesh1(qMesh._leftStartPt,  leftMid, qMesh._rightStartPt, rightMid, QuadMeshType::MESH_LEG);
            _quadMeshes.push_back(qMesh1);

            // leg
            QuadMesh qMesh2(leftMid, qMesh._leftEndPt, rightMid, qMesh._rightEndPt, QuadMeshType::MESH_LEG);
            _quadMeshes.push_back(qMesh2);
        }
        else
        {
            AVector left1 = topLine.GetPointInBetween(legLength / spineLength);
            AVector right1 = bottomLine.GetPointInBetween(legLength / spineLength);

            AVector left2 = topLine.GetPointInBetween((spineLength - legLength) / spineLength);
            AVector right2 = bottomLine.GetPointInBetween((spineLength - legLength) / spineLength);

            // leg
            QuadMesh qMesh1(qMesh._leftStartPt,  left1, qMesh._rightStartPt, right1, QuadMeshType::MESH_LEG);
            _quadMeshes.push_back(qMesh1);

            // rect
            QuadMesh qMesh2(left1,  left2, right1, right2, QuadMeshType::MESH_RECTILINEAR);
            _quadMeshes.push_back(qMesh2);

            // leg
            QuadMesh qMesh3(left2,  qMesh._leftEndPt, right2, qMesh._rightEndPt, QuadMeshType::MESH_LEG);
            _quadMeshes.push_back(qMesh3);
        }
    }
}

void StrokePainter::CalculateInitialSegments()
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
            if(rot1 > 0) // turn right: positive rotation
            {
                QuadMesh qMesh(_rightLines[a] + AVector(dir1.y, -dir1.x) * strokeWidth, /* lStart */
                               _leftLines[a],                                           /* lMid   */
                               _rightLines[a],                                          /* rMid   */
                               _rightLines[a] + AVector(dir2.y, -dir2.x) * strokeWidth, /* lEnd   */
                               _leftLines[a],                                           /* lMid   */
                               true, QuadMeshType::MESH_KITE);
                _quadMeshes.push_back(qMesh);
            }
            else if(rot1 < 0) // turn left: negative rotation
            {
                QuadMesh qMesh(_leftLines[a] + AVector(-dir1.y, dir1.x) * strokeWidth, /* rStart */
                               _leftLines[a],                                          /* lMid   */
                               _rightLines[a],                                         /* rMid   */
                               _leftLines[a] + AVector(-dir2.y, dir2.x) * strokeWidth, /* rEnd   */
                               _rightLines[a],                                         /* rMid   */
                               false, QuadMeshType::MESH_KITE);
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
            if(rot > 0)      // turn right: positive rotation
                { leftEnd = _rightLines[a+1] + AVector(dir1.y, -dir1.x) * strokeWidth; /* leftDir */ }
            else if(rot < 0) // turn left: negative rotation
                { rightEnd = _leftLines[a+1] + AVector(-dir1.y, dir1.x) * strokeWidth; /* rightDir */ }

            QuadMesh qMesh(_leftLines[a], leftEnd, _rightLines[a], rightEnd, QuadMeshType::MESH_RECTANGLE);
            _quadMeshes.push_back(qMesh);
        }
        else if(a == 0 && _spineLines.size() == 2)  // start
        {
            QuadMesh qMesh(_leftLines[a], _leftLines[a+1], _rightLines[a], _rightLines[a+1], QuadMeshType::MESH_RECTANGLE);
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
            if(rot > 0)      // turn right: positive rotation
                { leftStart = _rightLines[a] + AVector(dir2.y, -dir2.x) * strokeWidth; /* leftDir */ }
            else if(rot < 0) // turn left: negative rotation
                { rightStart = _leftLines[a] + AVector(-dir2.y, dir2.x) * strokeWidth; /* rightDir */ }

            QuadMesh qMesh(leftStart, _leftLines[a+1], rightStart, _rightLines[a+1], QuadMeshType::MESH_RECTANGLE);
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

            AVector leftStart  = _leftLines[a];
            AVector rightStart = _rightLines[a];
            AVector leftEnd    = _leftLines[a+1];
            AVector rightEnd   = _rightLines[a+1];

            /* start */
            if(rot1 > 0)      { leftStart  = _rightLines[a] + AVector(dir2.y, -dir2.x) * strokeWidth; /* leftDir */  }
            else if(rot1 < 0) { rightStart = _leftLines[a]  + AVector(-dir2.y, dir2.x) * strokeWidth; /* rightDir */ }

            /* end */
            if(rot2 > 0)      { leftEnd  = _rightLines[a+1] + AVector(dir2.y, -dir2.x) * strokeWidth; /* leftDir */  }
            else if(rot2 < 0) { rightEnd = _leftLines[a+1]  + AVector(-dir2.y, dir2.x) * strokeWidth; /* rightDir */ }

            QuadMesh qMesh(leftStart, leftEnd, rightStart, rightEnd, QuadMeshType::MESH_RECTANGLE);
            _quadMeshes.push_back(qMesh);
        }
    }
}



void StrokePainter::CalculateVertices(QuadMesh *prevQMesh, QuadMesh *curQMesh, QuadMesh *nextQMesh)
{
    curQMesh->_psVertices.clear();  // for conformal mapping
    curQMesh->_opsVertices.clear(); // initial vertices (should not be modified)

    // four corner points
    AVector lStartPt = curQMesh->_leftStartPt;
    AVector lEndPt   = curQMesh->_leftEndPt ;
    AVector rStartPt = curQMesh->_rightStartPt;
    AVector rEndPt   = curQMesh->_rightEndPt;

    AVector vVec  = rStartPt - lStartPt;  // a vertical vector (pointing downward)
    AVector uHVec = lEndPt - lStartPt; // an upper horizontal vector from start to end
    AVector bHVec = rEndPt - rStartPt; // a lower horizontal vector from start to end

    AVector mStartPt = ALine(lStartPt, rStartPt).GetMiddlePoint();  // for calculating mesh width
    AVector mEndPt   = ALine(lEndPt, rEndPt).GetMiddlePoint();      // for calculating mesh width

    // texture length
    float textureLength = SystemParams::stroke_width; // KITE
    if(curQMesh->_quadMeshType == QuadMeshType::MESH_LEG)
        { textureLength = _textureSizes[1].width(); }
    else if(curQMesh->_quadMeshType == QuadMeshType::MESH_RECTILINEAR)
        { textureLength = _textureSizes[2].width(); }

    // store this value
    curQMesh->_textureLength = textureLength;

    int textureNum = (int)std::round(mStartPt.Distance(mEndPt) / textureLength);

    int intMeshHeight = SystemParams::stroke_width / SystemParams::grid_cell_size;
    int intMeshWidth =  textureNum * (int)(textureLength / SystemParams::grid_cell_size);

    // to do: fix this bug
    if(intMeshWidth == 0) { intMeshWidth = intMeshHeight; }

    if(curQMesh->_quadMeshType == QuadMeshType::MESH_KITE) { intMeshWidth = intMeshHeight; }

    int xLoop = intMeshWidth + 1;
    int yLoop = intMeshHeight + 1;

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
            bool ribConstraint = false;

            if(curQMesh->_quadMeshType == QuadMeshType::MESH_RECTILINEAR)
            {
                // SHOULD_MOVE CHECK
                // no prev
                if(!prevQMesh && ( (xIter == 0 && yIter == 0) || (xIter == 0 && yIter == yLoop - 1) ))
                    { _constrainedPoints.push_back(pt);  shouldMove = false; }
                else if(!prevQMesh && xIter == 0)
                    { _constrainedPoints.push_back(pt); shouldMove = false; }

                // no next
                if(!nextQMesh && ( (xIter == xLoop - 1 && yIter == 0)  || (xIter == xLoop - 1 && yIter == yLoop - 1)  ))
                    { _constrainedPoints.push_back(pt);  shouldMove = false; }
                else if(!nextQMesh && xIter == xLoop - 1)
                    { _constrainedPoints.push_back(pt); shouldMove = false; }

                // RIB CONSTRAINT CHECK
                if(xIter == 0 || xIter == xLoop - 1)
                {
                    ribConstraint = true;
                    //_debugPoints.push_back(pt);
                }
            }
            else if(curQMesh->_quadMeshType == QuadMeshType::MESH_LEG)
            {
                // SHOULD_MOVE CHECK
                // prev is right kite, mark downleft
                if(prevQMesh && prevQMesh->_quadMeshType == QuadMeshType::MESH_KITE && prevQMesh->_isRightKite && (xIter == 0 && yIter == yLoop - 1))
                    { _constrainedPoints.push_back(pt); shouldMove = false; }
                // prev is left kite, mark upright
                else if(prevQMesh && prevQMesh->_quadMeshType == QuadMeshType::MESH_KITE && !prevQMesh->_isRightKite && (xIter == 0 && yIter == 0))
                    { _constrainedPoints.push_back(pt);  shouldMove = false; }

                // next is right kite, mark downright
                if(nextQMesh && nextQMesh->_quadMeshType == QuadMeshType::MESH_KITE && nextQMesh->_isRightKite && (xIter == xLoop - 1 && yIter == yLoop - 1))
                    { _constrainedPoints.push_back(pt);  shouldMove = false; }
                // next is left kite, mark upright
                else if(nextQMesh && nextQMesh->_quadMeshType == QuadMeshType::MESH_KITE && !nextQMesh->_isRightKite && (xIter == xLoop - 1 && yIter == 0))
                    { _constrainedPoints.push_back(pt);  shouldMove = false; }

                // RIB CONSTRAINT CHECK
                if(xIter == 0 || xIter == xLoop - 1)
                {
                    ribConstraint = true;
                    //_debugPoints.push_back(pt);
                }
            }
            else if(curQMesh->_quadMeshType == QuadMeshType::MESH_KITE)
            {
                // SHOULD_MOVE CHECK
                if( (xIter == 0 && yIter == yLoop - 1) || (xIter == xLoop - 1 && yIter == 0))
                    { _constrainedPoints.push_back(pt);  shouldMove = false; }

                // RIB CONSTRAINT CHECK
                // left, bottom
                if(curQMesh->_isRightKite && (xIter == 0 || yIter == yLoop - 1))
                {
                    ribConstraint = true;
                    //_debugPoints.push_back(pt);
                }
                // top bottom
                else if(!curQMesh->_isRightKite && (xIter == xLoop - 1 || yIter == 0))
                {
                    ribConstraint = true;
                    //_debugPoints.push_back(pt);
                }
            }

            PlusSignVertex psVert = PlusSignVertex(pt, shouldMove);
            psVert._isBoundaryRibConstrained = ribConstraint;
            columnVertices.push_back(psVert);
        }
        curQMesh->_psVertices.push_back(columnVertices);
        curQMesh->_opsVertices.push_back(columnVertices);
    }
}

void StrokePainter::CalculateVertices()
{
    // to do: weird error checking
    if(_quadMeshes.size() == 0)
    {
        std::cout << "you need to draw something\n";
        return;
    }

    _debugPoints.clear();

    _constrainedPoints.clear();
    for(uint a = 0; a < _quadMeshes.size(); a++)        
    {
        QuadMesh* prevQMesh = 0;
        QuadMesh* curQMesh = &_quadMeshes[a];
        QuadMesh* nextQMesh = 0;

        if(a > 0) { prevQMesh = &_quadMeshes[a - 1]; }
        if(a < _quadMeshes.size() - 1) { nextQMesh = &_quadMeshes[a + 1]; }

        if(SystemParams::enable_conformal_mapping)
        {
            CalculateVertices(prevQMesh, curQMesh, nextQMesh);
            //CalculateVertices1(curQMesh);
        }
        else
        {
            CalculateLinearVertices(curQMesh);
        }

        // lower side boundary: curQMesh->GetHeight() - 2

        std::vector<AVector> sideBoundary = curQMesh->GetSideBoundary(curQMesh->GetHeight() - 2);
        //std::cout << sideBoundary.size() << "\n";
        _debugPoints.insert(_debugPoints.end(), sideBoundary.begin(), sideBoundary.end());
    }

    // delete this
    _vDataHelper->BuildPointsVertexData(_debugPoints, &_debugPointsVbo, &_debugPointsVao, _debugPointsColor);

    _qMeshNumData = 0;
    _vDataHelper->BuildPointsVertexData(_constrainedPoints, &_constrainedPointsVbo, &_constrainedPointsVao, _constrainedPointColor);
    _vDataHelper->BuildLinesVertexData(_quadMeshes, &_quadMeshesVbo, &_quadMeshesVao, _qMeshNumData, _rectMeshesColor, _kiteMeshesColor, _legMeshesColor);
    _vDataHelper->BuildTexturedStrokeVertexData(_quadMeshes, &_texVbos[2], &_texVaos[2], _vertexNumbers[2], QuadMeshType::MESH_RECTILINEAR);
    _vDataHelper->BuildTexturedStrokeVertexData(_quadMeshes, &_texVbos[1], &_texVaos[1], _vertexNumbers[1], QuadMeshType::MESH_LEG);
    _vDataHelper->BuildTexturedStrokeVertexData(_quadMeshes, &_texVbos[0], &_texVaos[0], _vertexNumbers[0], QuadMeshType::MESH_KITE);
}

// illustrator-style texture mapping
void StrokePainter::CalculateLinearVertices(QuadMesh *qMesh)
{
    qMesh->_psVertices.clear();
    qMesh->_opsVertices.clear();

    AVector lStartPt = qMesh->_leftStartPt;
    AVector lEndPt   = qMesh->_leftEndPt ;
    AVector rStartPt = qMesh->_rightStartPt;
    AVector rEndPt   = qMesh->_rightEndPt;

    AVector topVec    = lEndPt   - lStartPt;   // kite only
    AVector rightVec  = lEndPt   - rEndPt;     // kite only
    AVector leftVec   = rStartPt - lStartPt;
    AVector bottomVec = rEndPt   - rStartPt;

    AVector vVec  = rStartPt - lStartPt;
    AVector uHVec = (lEndPt  - lStartPt);
    AVector bHVec = (rEndPt  - rStartPt);

    AVector mStartPt = ALine(lStartPt, rStartPt).GetMiddlePoint();
    AVector mEndPt   = ALine(lEndPt, rEndPt).GetMiddlePoint();

    // texture length
    float textureLength = SystemParams::stroke_width; // KITE
    if(qMesh->_quadMeshType == QuadMeshType::MESH_LEG)
        { textureLength = _textureSizes[1].width(); }
    else if(qMesh->_quadMeshType == QuadMeshType::MESH_RECTILINEAR)
        { textureLength = _textureSizes[2].width(); }

    // store this value
    qMesh->_textureLength = textureLength;

    int textureNum = (int)std::round(mStartPt.Distance(mEndPt) / textureLength);

    int intMeshHeight = SystemParams::stroke_width / (SystemParams::grid_cell_size);
    int intMeshWidth =  textureNum * textureLength / (SystemParams::grid_cell_size);

    // to do: fix this bug
    if(intMeshWidth == 0) { intMeshWidth = intMeshHeight; }

    if(qMesh->_quadMeshType == QuadMeshType::MESH_KITE) { intMeshWidth = intMeshHeight; }

    int xLoop = intMeshWidth + 1;
    int yLoop = intMeshHeight + 1;

    for(int xIter = 0; xIter < xLoop; xIter++)
    {
        std::vector<PlusSignVertex> columnVertices;

        for(int yIter = 0; yIter < yLoop;  yIter++)
        {
            float xFactor = (float)xIter / (float)intMeshWidth;
            float yFactor = (float)yIter / (float)intMeshHeight;

            if(qMesh->_quadMeshType == QuadMeshType::MESH_KITE)
            {
                AVector pt;

                int sumIdx = xIter + yIter;
                if(sumIdx <= intMeshHeight)
                {
                    pt = lStartPt + leftVec * yFactor;
                    pt = pt + topVec * xFactor;
                }
                else
                {
                    pt = rStartPt + bottomVec * xFactor;
                    pt = pt + rightVec * (1.0 - yFactor);
                }

                PlusSignVertex psVert = PlusSignVertex(pt, false);
                columnVertices.push_back(psVert);
            }
            else
            {
                // this code sucks...

                AVector hVec = uHVec * (1.0f - yFactor) + bHVec * yFactor;

                AVector pt = lStartPt + vVec * yFactor;
                pt = pt + hVec * xFactor;

                PlusSignVertex psVert = PlusSignVertex(pt, false);
                columnVertices.push_back(psVert);
            }
        }
        qMesh->_psVertices.push_back(columnVertices);
        qMesh->_opsVertices.push_back(columnVertices);
    }
}

/*
// to do: this function needs to be fixed since the code is obsolete
void StrokePainter::CalculateVertices1(QuadMesh* qMesh)
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

    // this part is obsolete
    float meshSize = SystemParams::grid_cell_size;
    int intMeshHeight = SystemParams::stroke_width / meshSize;
    int intMeshWidth =  (int)(mStartPt.Distance(mEndPt) / SystemParams::stroke_width) * intMeshHeight;

    // to do: fix this bug
    if(intMeshWidth == 0) { intMeshWidth = intMeshHeight; }

    if(qMesh->_quadMeshType == QuadMeshType::MESH_KITE) { intMeshWidth = intMeshHeight; }

    int xLoop = intMeshWidth + 1;
    int yLoop = intMeshHeight + 1;

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

            if(qMesh->_quadMeshType == QuadMeshType::MESH_KITE)
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

            PlusSignVertex psVert = PlusSignVertex(pt, shouldMove);
            columnVertices.push_back(psVert);
        }
        qMesh->_psVertices.push_back(columnVertices);
        qMesh->_opsVertices.push_back(columnVertices);
    }
}*/

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
    // to do: I will revive this function later...

    /*_cMapping->ConformalMappingOneStepSimple(_quadMeshes);

    _qMeshNumData = 0;
    _vDataHelper->BuildLinesVertexData(_quadMeshes, &_quadMeshesVbo, &_quadMeshesVao, _qMeshNumData, _rectMeshesColor, _kiteMeshesColor);
    _vDataHelper->(_quadMeshes, &_texVbos[1], &_texVaos[1], _vertexNumbers[1], QuadMeshType::MESH_RECTANGLE);
    _vDataHelper->BuildTexturedStrokeVertexData(_quadMeshes, &_texVbos[0], &_texVaos[0], _vertexNumbers[0], QuadMeshType::MESH_KITE);*/
}

void StrokePainter::ConformalMappingOneStep()
{
    //_cMapping->ConformalMappingOneStep(_quadMeshes);

    /*
    _debugLines = _cMapping->_debugLines;
    _vDataHelper->BuildLinesVertexData(_debugLines, &_debugLinesVbo, &_debugLinesVao, QVector3D(0, 0.25, 0));
    _debugPoints = _cMapping->_debugPoints;
    _vDataHelper->BuildPointsVertexData(_debugPoints, &_debugPointsVbo, &_debugPointsVao, QVector3D(0, 0.25, 0));
    */

    _qMeshNumData = 0;
    _vDataHelper->BuildLinesVertexData(_quadMeshes, &_quadMeshesVbo, &_quadMeshesVao, _qMeshNumData, _rectMeshesColor, _kiteMeshesColor, _legMeshesColor);
    _vDataHelper->BuildTexturedStrokeVertexData(_quadMeshes, &_texVbos[2], &_texVaos[2], _vertexNumbers[2], QuadMeshType::MESH_RECTILINEAR);
    _vDataHelper->BuildTexturedStrokeVertexData(_quadMeshes, &_texVbos[1], &_texVaos[1], _vertexNumbers[1], QuadMeshType::MESH_LEG);
    _vDataHelper->BuildTexturedStrokeVertexData(_quadMeshes, &_texVbos[0], &_texVaos[0], _vertexNumbers[0], QuadMeshType::MESH_KITE);
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

    /*
    _qMeshNumData = 0;
    _vDataHelper->BuildLinesVertexData(_quadMeshes, &_quadMeshesVbo, &_quadMeshesVao, _qMeshNumData, _rectMeshesColor, _kiteMeshesColor, _legMeshesColor);
    _vDataHelper->BuildTexturedStrokeVertexData(_quadMeshes, &_texVbos[1], &_texVaos[1], _vertexNumbers[1], QuadMeshType::MESH_RECTANGLE);
    _vDataHelper->BuildTexturedStrokeVertexData(_quadMeshes, &_texVbos[0], &_texVaos[0], _vertexNumbers[0], QuadMeshType::MESH_KITE);*/
}

// mouse press
void StrokePainter::mousePressEvent(float x, float y)
{
    _isMouseDown = true;

    _oriStrokeLines.clear();
    _selectedPointVao.destroy();

    // Get the closest point within a distance _maxDist
    _selectedIndex = GetClosestIndexFromSpinePoints(AVector(x, y), _maxDist);

    if(_selectedIndex != -1) // Selection mode: If we get a point
        { _vDataHelper->BuildPointsVertexData(_spineLines, &_selectedPointVbo, &_selectedPointVao, _selectedIndex, _unselectedPointColor, _selectedPointColor); }
    else    // Draw mode: we don't edit anything
        {  _oriStrokeLines.push_back(AVector(x, y)); }
}

// mouse move
void StrokePainter::mouseMoveEvent(float x, float y)
{
    if(_selectedIndex != -1)
    {
        // Selection mode, move the point
        _spineLines[_selectedIndex] = AVector(x, y);

        // Recalculate grids and iterate a step
        CalculateInitialRibbon();
        CalculateVertices();

        _vDataHelper->BuildPointsVertexData(_spineLines, &_selectedPointVbo, &_selectedPointVao, _selectedIndex, _unselectedPointColor, _selectedPointColor);
    }
    else
    {
        // Draw mode
        _oriStrokeLines.push_back(AVector(x, y));
        _vDataHelper->BuildLinesVertexData(_oriStrokeLines, &_oriStrokeLinesVbo, &_oriStrokeLinesVao, _oriStrokeColor);
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

        CalculateSpines(); // I moved CalculateSpines() from CalculateInitialRibbon() to here
        CalculateInitialRibbon();
        CalculateVertices();
    }
}

void StrokePainter::Draw()
{
    // Left lines
    if(SystemParams::show_mesh && _leftLinesVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glLineWidth(4.0f);
        _leftLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _leftLines.size() * 2);
        _leftLinesVao.release();
    }
    // Right lines
    if(SystemParams::show_mesh && _rightLinesVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glLineWidth(4.0f);
        _rightLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _rightLines.size() * 2);
        _rightLinesVao.release();
    }

    // Selected handle points
    if(SystemParams::show_mesh && _selectedPointVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glPointSize(10.0f);
        _selectedPointVao.bind();
        glDrawArrays(GL_POINTS, 0, _spineLines.size());
        _selectedPointVao.release();
    }

    // Constrained points
    if(SystemParams::show_mesh && _constrainedPointsVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glPointSize(4.0f);
        _constrainedPointsVao.bind();
        glDrawArrays(GL_POINTS, 0, _constrainedPoints.size());
        _constrainedPointsVao.release();
    }

    // Debug points
    if(SystemParams::show_mesh && _debugPointsVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glPointSize(8.0f);
        _debugPointsVao.bind();
        glDrawArrays(GL_POINTS, 0, _debugPoints.size());
        _debugPointsVao.release();
    }

    // Debug lines
    if(SystemParams::show_mesh && _debugLinesVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glLineWidth(3.0f);
        _debugLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _debugLines.size() * 2);
        _debugLinesVao.release();
    }

    // Original stroke
    if(_isMouseDown && _oriStrokeLinesVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glLineWidth(2.0f);
        _oriStrokeLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _oriStrokeLines.size() * 2);
        _oriStrokeLinesVao.release();
    }

    // Spine lines
    /*if(SystemParams::show_mesh && _spineLinesVao.isCreated())
    {
        _vDataHelper->NeedToDrawWithColor(1.0);
        glLineWidth(2.0f);
        _spineLinesVao.bind();
        glDrawArrays(GL_LINES, 0, _spineLines.size() * 2);
        _spineLinesVao.release();
    }
    */

    // Quad mesh
    if(SystemParams::show_mesh && _quadMeshesVao.isCreated())
    {
        glLineWidth(1.0f);
        _quadMeshesVao.bind();
        glDrawArrays(GL_LINES, 0, _qMeshNumData);
        _quadMeshesVao.release();
    }

    // Rect Texture
    if(SystemParams::show_texture && _vertexNumbers[2] > 0)
    {
        _vDataHelper->NeedToDrawWithColor(0.0);

        if(_oglTextures[2]) { _oglTextures[2]->bind(); }
        _texVaos[2].bind();
        glDrawArrays(GL_QUADS, 0, _vertexNumbers[2]);
        _texVaos[2].release();
        if(_oglTextures[2]) { _oglTextures[2]->release(); }
    }

    // Leg Texture
    if(SystemParams::show_texture && _vertexNumbers[1] > 0)
    {
        _vDataHelper->NeedToDrawWithColor(0.0);

        if(_oglTextures[1]) { _oglTextures[1]->bind(); }
        _texVaos[1].bind();
        glDrawArrays(GL_QUADS, 0, _vertexNumbers[1]);
        _texVaos[1].release();
        if(_oglTextures[1]) { _oglTextures[1]->release(); }
    }

    // Kite Texture
    if(SystemParams::show_texture && _vertexNumbers[0] > 0)
    {
        _vDataHelper->NeedToDrawWithColor(0.0);
        if(_oglTextures[0]) { _oglTextures[0]->bind(); }
        _texVaos[0].bind();
        glDrawArrays(GL_QUADS, 0, _vertexNumbers[0]);
        _texVaos[0].release();
        if(_oglTextures[0]) { _oglTextures[0]->release(); }
    }
}



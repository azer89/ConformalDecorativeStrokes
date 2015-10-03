
#include "QuadMesh.h"

#include "UtilityFunctions.h"

/**
  * Reza Adhitya Saputra
  * radhitya@uwaterloo.ca
  */


QuadMesh::QuadMesh()
{
}

QuadMesh::QuadMesh(AVector      leftStartPt,
                   AVector      leftEndPt,
                   AVector      rightStartPt,
                   AVector      rightEndPt,
                   AVector      sharpPt,
                   bool         isRightKite,
                   QuadMeshType quadMeshType)
{
    this->_leftStartPt  = leftStartPt;
    this->_leftEndPt    = leftEndPt;
    this->_rightStartPt = rightStartPt;
    this->_rightEndPt   = rightEndPt;
    this->_innerConcavePt      = sharpPt;
    this->_isRightKite  = isRightKite;
    this->_quadMeshType = quadMeshType;

    //this->_isBoundaryRibConstrained = false;
}

QuadMesh::QuadMesh(AVector      leftStartPt,
                   AVector      leftEndPt,
                   AVector      rightStartPt,
                   AVector      rightEndPt,
                   QuadMeshType quadMeshType)
{
    this->_leftStartPt  = leftStartPt;
    this->_leftEndPt    = leftEndPt;
    this->_rightStartPt = rightStartPt;
    this->_rightEndPt   = rightEndPt;
    this->_quadMeshType = quadMeshType;

    //this->_isBoundaryRibConstrained = false;
}

QuadMesh::~QuadMesh()
{
}

int QuadMesh::GetWidth() { return this->_psVertices.size(); }
int QuadMesh::GetHeight() { return this->_psVertices[0].size(); }

std::vector<AVector> QuadMesh::GetABoundary(int index, bool isXUnchanged, bool isOri)
{
    // is this correct ?
    std::vector<std::vector<PlusSignVertex>> psVertices = (isOri) ? _opsVertices : _psVertices;

    std::vector<AVector> vertices;
    if(isXUnchanged) // column
    {
        int meshHeight = GetHeight();
        for(int yIter = 0; yIter < meshHeight; yIter++)
            { vertices.push_back(psVertices[index][yIter].position); }
    }
    else // row
    {
        int meshWidth = GetWidth();
        for(int xIter = 0; xIter < meshWidth; xIter++)
            { vertices.push_back(psVertices[xIter][index].position); }
    }
    return vertices;
}

void QuadMesh::SetSlidingConstraintFlag(int index, bool boolValue, std::vector<AVector> &debugPoints)
{
    int w = this->GetWidth();
    int h = this->GetHeight();

    if(this->_quadMeshType == QuadMeshType::MESH_KITE && this->_isRightKite)
    {
        for(int xIter = 0; xIter < w - index; xIter++)
        {
            _psVertices[xIter][index]._isSlideConstrained = boolValue;
            if((xIter + index) == h - 1)
            {
                if(boolValue)  debugPoints.push_back(_psVertices[xIter][index].position);
                _psVertices[xIter][index].shouldMove = !boolValue;
            }
        }

        for(int yIter = (index + 1); yIter < h; yIter++)
        {
            int xIndex = w - (index + 1);

            _psVertices[xIndex][yIter]._isSlideConstrained = boolValue;
        }
    }
    else if(this->_quadMeshType == QuadMeshType::MESH_KITE && !this->_isRightKite)
    {
        // invert
        index = h - (index + 1);

        for(int yIter = 0; yIter < h - index; yIter++)
        {
            _psVertices[index][yIter]._isSlideConstrained = boolValue;

            if((index + yIter) == h - 1)
            {
                if(boolValue)  debugPoints.push_back(_psVertices[index][yIter].position);
                _psVertices[index][yIter].shouldMove = !boolValue;
            }
        }

        for(int xIter = (index + 1); xIter < w; xIter++)
        {
            int yIndex = h - (index + 1);
            _psVertices[xIter][yIndex]._isSlideConstrained = boolValue;
        }
    }
    else if(this->_quadMeshType == QuadMeshType::MESH_LEG)
    {
        for(int xIter = 0; xIter < w; xIter++)
        {
            _psVertices[xIter][index]._isSlideConstrained = boolValue;
        }
    }
    else if(this->_quadMeshType == QuadMeshType::MESH_RECTILINEAR)
    {
        for(int xIter = 0; xIter < w; xIter++)
        {
            _psVertices[xIter][index]._isSlideConstrained = boolValue;
        }
    }
}

std::vector<AVector> QuadMesh::GetSideBoundary(int index)
{
    std::vector<AVector> vertices;
    int w = this->GetWidth();
    int h = this->GetHeight();

    if(this->_quadMeshType == QuadMeshType::MESH_KITE && this->_isRightKite)
    {
        for(int xIter = 0; xIter < w - index; xIter++)
        {
            vertices.push_back(_psVertices[xIter][index].position);
        }

        for(int yIter = (index + 1); yIter < h; yIter++)
        {
            int xIndex = w - (index + 1);
            vertices.push_back(_psVertices[xIndex][yIter].position);
        }
    }
    else if(this->_quadMeshType == QuadMeshType::MESH_KITE && !this->_isRightKite)
    {
        // invert
        index = h - (index + 1);

        for(int yIter = 0; yIter < h - index; yIter++)
        {
            vertices.push_back(_psVertices[index][yIter].position);
        }

        for(int xIter = (index + 1); xIter < w; xIter++)
        {
            int yIndex = h - (index + 1);
            vertices.push_back(_psVertices[xIter][yIndex].position);
        }
    }
    else if(this->_quadMeshType == QuadMeshType::MESH_LEG)
    {
        for(int xIter = 0; xIter < w; xIter++)
        {
            vertices.push_back(_psVertices[xIter][index].position);
        }
    }
    else if(this->_quadMeshType == QuadMeshType::MESH_RECTILINEAR)
    {
        for(int xIter = 0; xIter < w; xIter++)
        {
            vertices.push_back(_psVertices[xIter][index].position);
        }
    }

    return vertices;
}

// not compatible with dirichlet boundary condition
AVector QuadMesh::GetClosestPointFromBorders(AVector pt)
{
    AVector closestPt = pt;
    float dist = std::numeric_limits<float>::max();
    std::vector<ALine> borderLines;
    borderLines.push_back(ALine(_leftStartPt,  _rightStartPt));
    borderLines.push_back(ALine(_leftEndPt,    _rightEndPt));
    borderLines.push_back(ALine(_leftStartPt,  _leftEndPt));
    borderLines.push_back(ALine(_rightStartPt, _rightEndPt));
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


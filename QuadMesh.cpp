
#include "QuadMesh.h"

#include "UtilityFunctions.h"

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


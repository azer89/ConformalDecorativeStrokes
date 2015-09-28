#include "ConformalMapping.h"

#include "AVector.h"
#include "ALine.h"
#include "PlusSignVertex.h"

#include "SystemParams.h"
#include "UtilityFunctions.h"

ConformalMapping::ConformalMapping() :
    _iterDist(std::numeric_limits<float>::max())
{
}

/**
 * A simple averaging calculation.
 * This function does not consider the neighbouring legs
 */
void ConformalMapping::ConformalMappingOneStepSimple(std::vector<QuadMesh>& quadMeshes)
{
    this->_debugPoints.clear(); // delete this
    this->_debugLines.clear();  // delete this

    this->_iterDist = 0;
    for(uint a = 0; a < quadMeshes.size(); a++)
    {
        ConformalMappingOneStepSimple(&quadMeshes[a]);
    }
}

/**
 * A simple averaging calculation.
 * This function does not consider the neighbouring legs
 */
void ConformalMapping::ConformalMappingOneStepSimple(QuadMesh *qMesh)
{
    std::vector<std::vector<PlusSignVertex>> tempVertices = qMesh->_psVertices;
    int meshWidth = qMesh->GetWidth();
    int meshHeight = qMesh->GetHeight();

    for(int a = 0; a < meshWidth; a++)
    {
        for(int b = 0; b < meshHeight; b++)
        {
            if(!tempVertices[a][b].shouldMove) { continue; }

            AVector sumPositions(0, 0);
            int numNeighbor = 0;

            if(a > 0) // left
                { sumPositions += tempVertices[a - 1][b].position; numNeighbor++; }

            if(a < meshWidth - 1) // right
                { sumPositions = sumPositions + tempVertices[a + 1][b].position; numNeighbor++; }

            if(b > 0) // up
                { sumPositions += tempVertices[a][b - 1].position; numNeighbor++; }

            if(b < meshHeight - 1) // bottom
                { sumPositions += tempVertices[a][b + 1].position; numNeighbor++; }

            sumPositions  = sumPositions / (float)numNeighbor;

            if(numNeighbor < 4) { tempVertices[a][b].position = qMesh->GetClosestPointFromBorders(sumPositions); }
            else { tempVertices[a][b].position = sumPositions; }
        }
    }

    float sumDist = 0;
    for(int a = 0; a < meshWidth; a++)
    {
        for(int b = 0; b < meshHeight; b++)
            { sumDist += qMesh->_psVertices[a][b].position.Distance(tempVertices[a][b].position); }
    }
    _iterDist = sumDist;
    qMesh->_psVertices = tempVertices;
}

void ConformalMapping::ConformalMappingOneStep(std::vector<QuadMesh>& quadMeshes)
{
    this->_debugPoints.clear();
    this->_debugLines.clear();

    this->_iterDist = 0;
    for(uint a = 0; a < quadMeshes.size(); a++)
    {
        QuadMesh* prevQMesh = (a > 0) ? &quadMeshes[a - 1] : 0 ;
        QuadMesh* curQMesh = &quadMeshes[a];
        QuadMesh* nextQMesh = (a < quadMeshes.size() - 1) ? &quadMeshes[a + 1] : 0 ;

        ConformalMappingOneStep(prevQMesh, curQMesh, nextQMesh);
    }
}

/**
 * unused
 */
void ConformalMapping::MappingInterpolation(std::vector<QuadMesh>& quadMeshes)
{
    for(uint a = 0; a < quadMeshes.size(); a++)
    {
        if(quadMeshes[a]._quadMeshType == QuadMeshType::MESH_KITE)
        {
            MappingInterpolation(&quadMeshes[a]);
        }
    }
}

AVector ConformalMapping::GetClosestPointFromRibs(int x, int y, AVector pt, QuadMesh* curQMesh)
{
    std::vector<ALine> borderLines;

    if(curQMesh->_quadMeshType == QuadMeshType::MESH_KITE && curQMesh->_isRightKite)
    {
        // left, bottom
        borderLines.push_back(ALine(curQMesh->_leftStartPt, curQMesh->_rightStartPt));
        borderLines.push_back(ALine(curQMesh->_rightStartPt, curQMesh->_rightEndPt));
    }
    else if(curQMesh->_quadMeshType == QuadMeshType::MESH_KITE && !curQMesh->_isRightKite)
    {
        // top, right
        borderLines.push_back(ALine(curQMesh->_leftStartPt, curQMesh->_leftEndPt));
        borderLines.push_back(ALine(curQMesh->_leftEndPt, curQMesh->_rightEndPt));
    }
    else if(curQMesh->_quadMeshType == QuadMeshType::MESH_LEG)
    {
        borderLines.push_back(ALine(curQMesh->_leftStartPt, curQMesh->_rightStartPt));
        borderLines.push_back(ALine(curQMesh->_leftEndPt, curQMesh->_rightEndPt));
    }
    else if(curQMesh->_quadMeshType == QuadMeshType::MESH_RECTILINEAR)
    {
        borderLines.push_back(ALine(curQMesh->_leftStartPt, curQMesh->_rightStartPt));
        borderLines.push_back(ALine(curQMesh->_leftEndPt, curQMesh->_rightEndPt));
    }

    AVector closestPt = pt;
    float dist = std::numeric_limits<float>::max();
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

/**
 * sliding boundary condition
 */
AVector ConformalMapping::GetClosestPointFromBorders(int x, int y, AVector pt, QuadMesh* prevQMesh, QuadMesh* curQMesh, QuadMesh* nextQMesh)
{
    int cWidth  = curQMesh->GetWidth();
    int cHeight = curQMesh->GetHeight();
    std::vector<ALine> borderLines;

    if(curQMesh->_quadMeshType == QuadMeshType::MESH_KITE && curQMesh->_isRightKite)
    {
        if(y == 0) // update top vertices
        {
            borderLines.push_back(ALine(curQMesh->_leftStartPt, curQMesh->_leftEndPt));   /* add own (top)*/
            borderLines.push_back(ALine(prevQMesh->_leftStartPt, prevQMesh->_leftEndPt)); /* add previous (top) */
        }
        else if(x == cWidth - 1 ) // update right vertices
        {
            borderLines.push_back(ALine(curQMesh->_leftEndPt, curQMesh->_rightEndPt));    /* add own (right) */
            borderLines.push_back(ALine(nextQMesh->_leftStartPt, nextQMesh->_leftEndPt)); /* add next neighbor (top) */
        }
    }
    else if(curQMesh->_quadMeshType == QuadMeshType::MESH_KITE && !curQMesh->_isRightKite)
    {
        if(x == 0) // update left vertices
        {
            borderLines.push_back(ALine(curQMesh->_leftStartPt, curQMesh->_rightStartPt));  /* add own (left) */
            borderLines.push_back(ALine(prevQMesh->_rightStartPt, prevQMesh->_rightEndPt)); /* add previous neighbor (bottom) */
        }
        else if(y == cHeight - 1) // update bottom vertices
        {
            borderLines.push_back(ALine(curQMesh->_rightStartPt, curQMesh->_rightEndPt));   /* add own (bottom) */
            borderLines.push_back(ALine(nextQMesh->_rightStartPt, nextQMesh->_rightEndPt)); /* add next neighbor (bottom) */
        }
    }
    else if(curQMesh->_quadMeshType == QuadMeshType::MESH_LEG) // MESH_LEG
    {
        // to do: should add the entire borders (take from left and right lines...)

        if(y == 0) // update top vertices
        {
            borderLines.push_back(ALine(curQMesh->_leftStartPt, curQMesh->_leftEndPt)); /* add own (top) */

            if(prevQMesh && prevQMesh->_quadMeshType == QuadMeshType::MESH_KITE && prevQMesh->_isRightKite)
            {
                borderLines.push_back(ALine(prevQMesh->_leftEndPt, prevQMesh->_rightEndPt)); /* add neighbor (right) */
            }
            else if(prevQMesh && prevQMesh->_quadMeshType != QuadMeshType::MESH_KITE)
            {
                borderLines.push_back(ALine(prevQMesh->_leftStartPt, prevQMesh->_leftEndPt));
            }

            if(nextQMesh && nextQMesh->_quadMeshType == QuadMeshType::MESH_KITE && nextQMesh->_isRightKite)
            {
                borderLines.push_back(ALine(nextQMesh->_leftStartPt, nextQMesh->_leftEndPt)); /* add neighbor (top) */
            }
            else if(nextQMesh && nextQMesh->_quadMeshType != QuadMeshType::MESH_KITE)
            {
                borderLines.push_back(ALine(nextQMesh->_leftStartPt, nextQMesh->_leftEndPt));
            }
        }
        else if(y == cHeight - 1) // update bottom vertices
        {
            borderLines.push_back(ALine(curQMesh->_rightStartPt, curQMesh->_rightEndPt)); /* add own (bottom) */

            if(prevQMesh && prevQMesh->_quadMeshType == QuadMeshType::MESH_KITE && !prevQMesh->_isRightKite)
            {
                borderLines.push_back(ALine(prevQMesh->_rightStartPt, prevQMesh->_rightEndPt)); /* add neighbor (bottom) */
            }
            else if(prevQMesh && prevQMesh->_quadMeshType != QuadMeshType::MESH_KITE)
            {
                borderLines.push_back(ALine(prevQMesh->_rightStartPt, prevQMesh->_rightEndPt));
            }

            if(nextQMesh && nextQMesh->_quadMeshType == QuadMeshType::MESH_KITE  && !nextQMesh->_isRightKite)
            {
                borderLines.push_back(ALine(nextQMesh->_leftStartPt, nextQMesh->_rightStartPt)); /* add neighbor (left) */
            }
            else if(nextQMesh && nextQMesh->_quadMeshType != QuadMeshType::MESH_KITE)
            {
                borderLines.push_back(ALine(nextQMesh->_rightStartPt, nextQMesh->_rightEndPt));
            }
        }
    }
    else if(curQMesh->_quadMeshType == QuadMeshType::MESH_RECTILINEAR)
    {
        if(y == 0) // update top vertices
        {
            borderLines.push_back(ALine(curQMesh->_leftStartPt, curQMesh->_leftEndPt)); /* add own (top) */

            if(prevQMesh) // must be MESH_LEG
            {
                borderLines.push_back(ALine(prevQMesh->_leftStartPt, prevQMesh->_leftEndPt));
            }

            if(nextQMesh)  // must be MESH_LEG
            {
                borderLines.push_back(ALine(nextQMesh->_leftStartPt, nextQMesh->_leftEndPt));
            }
        }
        else if(y == cHeight - 1) // update bottom vertices
        {
            borderLines.push_back(ALine(curQMesh->_rightStartPt, curQMesh->_rightEndPt)); /* add own (bottom) */

            if(prevQMesh)  // must be MESH_LEG
            {
                borderLines.push_back(ALine(prevQMesh->_rightStartPt, prevQMesh->_rightEndPt));
            }

            if(nextQMesh)   // must be MESH_LEG
            {
                borderLines.push_back(ALine(nextQMesh->_rightStartPt, nextQMesh->_rightEndPt));
            }
        }
    }

    /*
    AVector closestPt = pt;
    float dist = std::numeric_limits<float>::max();
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
    */

    return UtilityFunctions::GetClosestPoint(borderLines, pt);
}

void ConformalMapping::UpdateNeighbor(int x, int y,
                                      QuadMesh* prevQMesh, QuadMesh* curQMesh, QuadMesh* nextQMesh,
                                      std::vector<std::vector<PlusSignVertex>>& pTempVertices,
                                      std::vector<std::vector<PlusSignVertex>>& cTempVertices,
                                      std::vector<std::vector<PlusSignVertex>>& nTempVertices)
{
    int pWidth  = (prevQMesh) ? prevQMesh->GetWidth() : 0;
    int pHeight = (prevQMesh) ? prevQMesh->GetHeight() : 0;
    int cWidth  = curQMesh->GetWidth();
    int cHeight = curQMesh->GetHeight();
    int nWidth  = (nextQMesh) ? nextQMesh->GetWidth() : 0;
    int nHeight = (nextQMesh) ? nextQMesh->GetHeight() : 0;

    if(curQMesh->_quadMeshType == QuadMeshType::MESH_KITE && curQMesh->_isRightKite)
    {
        if(x == 0)
        {
            pTempVertices[pWidth - 1][y].position = cTempVertices[x][y].position;
        }
        else if( y == (cHeight - 1))
        {
            nTempVertices[0][nHeight - (x + 1)].position = cTempVertices[x][y].position;
        }
    }
    else if(curQMesh->_quadMeshType == QuadMeshType::MESH_KITE && !curQMesh->_isRightKite)
    {
        if(x == (cWidth - 1))
        {
            nTempVertices[0][y].position  = cTempVertices[x][y].position;
        }
        else if(y == 0)
        {
            pTempVertices[pWidth - 1][pHeight - (x + 1)].position = cTempVertices[x][y].position;
        }
    }
    else if(curQMesh->_quadMeshType == QuadMeshType::MESH_LEG) // MESH_LEG, neighbor is either MESH_KITE or MESH_RECTILINEAR
    {
        if(x == 0 && prevQMesh &&  prevQMesh->_quadMeshType == QuadMeshType::MESH_KITE && prevQMesh->_isRightKite)
        {
            pTempVertices[pWidth - (y + 1)][pHeight - 1].position = cTempVertices[x][y].position;
        }
        else if(x == 0 && prevQMesh && prevQMesh->_quadMeshType == QuadMeshType::MESH_KITE && !prevQMesh->_isRightKite)
        {
            pTempVertices[pWidth - 1][y].position = cTempVertices[x][y].position;
        }
        else if(x == 0 && prevQMesh && prevQMesh->_quadMeshType != QuadMeshType::MESH_KITE)
        {
            pTempVertices[pWidth - 1][y].position = cTempVertices[x][y].position;
        }

        else if(x == (cWidth - 1) && nextQMesh->_quadMeshType == QuadMeshType::MESH_KITE && nextQMesh && nextQMesh->_isRightKite)
        {
            nTempVertices[0][y].position = cTempVertices[x][y].position;
        }
        else if(x == (cWidth - 1) && nextQMesh->_quadMeshType == QuadMeshType::MESH_KITE && nextQMesh && !nextQMesh->_isRightKite)
        {
            nTempVertices[nWidth - (y + 1)][0].position = cTempVertices[x][y].position;
        }
        else if(x == (cWidth - 1) && nextQMesh->_quadMeshType != QuadMeshType::MESH_KITE)
        {
            nTempVertices[0][y].position = cTempVertices[x][y].position;
        }
    }
    else if(curQMesh->_quadMeshType == QuadMeshType::MESH_RECTILINEAR) // neighbors are MESH_LEG
    {
        if(x == 0 && prevQMesh)
        {
            pTempVertices[pWidth - 1][y].position = cTempVertices[x][y].position;
        }
        else if(x == (cWidth - 1) && nextQMesh)
        {
            nTempVertices[0][y].position = cTempVertices[x][y].position;
        }
    }
}

PlusSignVertex ConformalMapping::GetNeighbor(int x, int y,
                                             NeighborDirection dir,
                                             QuadMesh* prevQMesh, QuadMesh* curQMesh, QuadMesh* nextQMesh,
                                             std::vector<std::vector<PlusSignVertex> > pTempVertices,
                                             std::vector<std::vector<PlusSignVertex> > cTempVertices,
                                             std::vector<std::vector<PlusSignVertex> > nTempVertices)
{

    int pWidth  = (prevQMesh) ? prevQMesh->GetWidth() : 0 ;
    int pHeight = (prevQMesh) ? prevQMesh->GetHeight() : 0 ;
    int cWidth  = curQMesh->GetWidth();
    int cHeight = curQMesh->GetHeight();
    int nWidth  = (nextQMesh) ? nextQMesh->GetWidth() : 0 ;
    int nHeight = (nextQMesh) ? nextQMesh->GetHeight() : 0 ;

    if(dir == NeighborDirection::ND_LEFT && x > 0)
        { return cTempVertices[x - 1][y]; }
    else if(dir == NeighborDirection::ND_RIGHT && x < cWidth - 1)
        { return cTempVertices[x + 1][y]; }
    else if(dir == NeighborDirection::ND_UP && y > 0)
        { return cTempVertices[x][y - 1]; }
    else if(dir == NeighborDirection::ND_DOWN && y < cHeight - 1)
        { return cTempVertices[x][y + 1]; }


    if(curQMesh->_quadMeshType == QuadMeshType::MESH_KITE && curQMesh->_isRightKite)
    {
        if(dir == NeighborDirection::ND_LEFT)
        {
            return pTempVertices[pWidth - 2][y];
        }
        else if(dir == NeighborDirection::ND_DOWN)
        {
            return nTempVertices[1][nHeight - (x + 1)];
        }
    }
    else if(curQMesh->_quadMeshType == QuadMeshType::MESH_KITE && !curQMesh->_isRightKite)
    {
        if(dir == NeighborDirection::ND_RIGHT)
        {
            return nTempVertices[1][y];
        }
        else if(dir == NeighborDirection::ND_UP)
        {
            return pTempVertices[pWidth - 2][pHeight - (x + 1)];
        }
    }
    else if(curQMesh->_quadMeshType == QuadMeshType::MESH_LEG) // MESH_LEG
    {
        if(dir == NeighborDirection::ND_LEFT && prevQMesh && prevQMesh->_quadMeshType == QuadMeshType::MESH_KITE && prevQMesh->_isRightKite)
        {
            return pTempVertices[pWidth - (y + 1)][pHeight - 2];

        }
        else if(dir == NeighborDirection::ND_LEFT && prevQMesh && prevQMesh->_quadMeshType == QuadMeshType::MESH_KITE && !prevQMesh->_isRightKite)
        {
            return pTempVertices[pWidth - 2][y];
        }
        else if(dir == NeighborDirection::ND_LEFT && prevQMesh && prevQMesh->_quadMeshType != QuadMeshType::MESH_KITE)
        {
            return pTempVertices[pWidth - 2][y];
        }

        else if(dir == NeighborDirection::ND_RIGHT && nextQMesh && nextQMesh->_quadMeshType == QuadMeshType::MESH_KITE && nextQMesh->_isRightKite)
        {
            return nTempVertices[1][y];
        }
        else if(dir == NeighborDirection::ND_RIGHT && nextQMesh && nextQMesh->_quadMeshType == QuadMeshType::MESH_KITE && !nextQMesh->_isRightKite)
        {
            return nTempVertices[nWidth - (y + 1)][1];
        }
        else if(dir == NeighborDirection::ND_RIGHT && nextQMesh && nextQMesh->_quadMeshType != QuadMeshType::MESH_KITE)
        {
            return nTempVertices[1][y];
        }
    }
    else if(curQMesh->_quadMeshType == QuadMeshType::MESH_RECTILINEAR)
    {
        if(dir == NeighborDirection::ND_LEFT && prevQMesh)
        {
            return pTempVertices[pWidth - 2][y];
        }
        else if(dir == NeighborDirection::ND_RIGHT && nextQMesh)
        {
            return nTempVertices[1][y];
        }
    }

    return PlusSignVertex();
}

void ConformalMapping::ConformalMappingOneStep(QuadMesh* prevQMesh, QuadMesh* curQMesh, QuadMesh* nextQMesh)
{
    //std::cout << _slidingConstraints.size() << "~\n";

    // to do:
    // copying arrays == slow
    std::vector<std::vector<PlusSignVertex>> pTempVertices;
    std::vector<std::vector<PlusSignVertex>> cTempVertices = curQMesh->_psVertices;
    std::vector<std::vector<PlusSignVertex>> nTempVertices;

    if(prevQMesh) { pTempVertices = prevQMesh->_psVertices; }
    if(nextQMesh) { nTempVertices = nextQMesh->_psVertices; }

    int meshWidth = curQMesh->GetWidth();
    int meshHeight = curQMesh->GetHeight();

    for(int a = 0; a < meshWidth; a++)
    {
        for(int b = 0; b < meshHeight; b++)
        {
            if(!cTempVertices[a][b].shouldMove) { continue; }

            AVector curPos = cTempVertices[a][b].position;
            AVector sumPositions(0, 0);
            float sumArmLengths = 0;
            float sumArmAngles = 0;
            int numNeighbor = 0;        // considering vertices on the neighboring meshes
            int numRealNeighbor = 0;    // neighboring vertices within a mesh

            // Neighbors
            PlusSignVertex lVertex = GetNeighbor(a, b, NeighborDirection::ND_LEFT,  prevQMesh, curQMesh, nextQMesh, pTempVertices, cTempVertices, nTempVertices);
            PlusSignVertex rVertex = GetNeighbor(a, b, NeighborDirection::ND_RIGHT, prevQMesh, curQMesh, nextQMesh, pTempVertices, cTempVertices, nTempVertices);
            PlusSignVertex uVertex = GetNeighbor(a, b, NeighborDirection::ND_UP,    prevQMesh, curQMesh, nextQMesh, pTempVertices, cTempVertices, nTempVertices);
            PlusSignVertex bVertex = GetNeighbor(a, b, NeighborDirection::ND_DOWN,  prevQMesh, curQMesh, nextQMesh, pTempVertices, cTempVertices, nTempVertices);

            if(a > 0 && a < meshWidth - 1)  { numRealNeighbor += 2; }
            else if(a > 0)                  { numRealNeighbor++; }
            else if(a < meshWidth - 1)      { numRealNeighbor++; }

            if(b > 0 && b < meshHeight - 1) { numRealNeighbor += 2; }
            else if(b > 0)                  { numRealNeighbor++; }
            else if(b < meshHeight - 1)     { numRealNeighbor++; }

            // have left and right
            if(lVertex.IsValid() && rVertex.IsValid())
            {
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
            else if(lVertex.IsValid())
            {
                AVector fakeNeighbor = lVertex.position + UtilityFunctions::Rotate( AVector(1, 0) * lVertex.armLength, lVertex.angle);
                sumPositions += fakeNeighbor;
                sumArmLengths += curPos.Distance(lVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(-1, 0), (lVertex.position - curPos).Norm());

                numNeighbor++;
            }
            // have right only
            else if(rVertex.IsValid())
            {
                AVector fakeNeighbor = rVertex.position + UtilityFunctions::Rotate( AVector(-1, 0) * rVertex.armLength, rVertex.angle);
                sumPositions += fakeNeighbor;
                sumArmLengths += curPos.Distance(rVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(1, 0), (rVertex.position - curPos).Norm());

                numNeighbor++;
            }

            // have up and down
            if(uVertex.IsValid() && bVertex.IsValid())
            {
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
            else if(uVertex.IsValid())
            {
                AVector fakeNeighbor = uVertex.position + UtilityFunctions::Rotate( AVector(0, 1) * uVertex.armLength, uVertex.angle);
                sumPositions += fakeNeighbor;
                sumArmLengths += curPos.Distance(uVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(0, -1), (uVertex.position - curPos).Norm());

                numNeighbor++;
            }
            // have down only
            else if(bVertex.IsValid())
            {
                AVector fakeNeighbor = bVertex.position + UtilityFunctions::Rotate( AVector(0, -1) * bVertex.armLength, bVertex.angle);
                sumPositions += fakeNeighbor;
                sumArmLengths += curPos.Distance(bVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(0, 1), (bVertex.position - curPos).Norm());

                numNeighbor++;
            }

            sumPositions  = sumPositions / (float)numNeighbor;
            sumArmAngles  = sumArmAngles / (float)numNeighbor;
            sumArmLengths = sumArmLengths / (float)numNeighbor;

            cTempVertices[a][b].armLength = sumArmLengths;
            cTempVertices[a][b].angle = sumArmAngles;

            // ???
            if(cTempVertices[a][b]._isSlideConstrained)
            {
                cTempVertices[a][b].position = UtilityFunctions::GetClosestPoint(_slidingConstraints, sumPositions);
            }
            else if(SystemParams::segment_constraint && cTempVertices[a][b]._isBoundaryRibConstrained)
            {
                cTempVertices[a][b].position = GetClosestPointFromRibs(a, b, sumPositions, curQMesh);
            }
            else if(numNeighbor < 4)
                { cTempVertices[a][b].position = GetClosestPointFromBorders(a, b, sumPositions,  prevQMesh, curQMesh, nextQMesh); }
            else
                { cTempVertices[a][b].position = sumPositions; }


            // update a neighbor mesh
            if(numRealNeighbor < 4)
            {
                UpdateNeighbor(a, b, prevQMesh, curQMesh, nextQMesh, pTempVertices, cTempVertices, nTempVertices);
            }
        }
    }

    float sumDist = 0;
    for(int a = 0; a < meshWidth; a++)
    {
        for(int b = 0; b < meshHeight; b++)
            { sumDist += curQMesh->_psVertices[a][b].position.Distance(cTempVertices[a][b].position); }
    }
    _iterDist += sumDist;

    // transfer (slow...)
    curQMesh->_psVertices = cTempVertices;
    if(prevQMesh) { prevQMesh->_psVertices = pTempVertices; }
    if(nextQMesh) { nextQMesh->_psVertices = nTempVertices; }
}

/**
 * Unused
 */
void ConformalMapping::MappingInterpolation(QuadMesh *qMesh)
{
    std::vector<std::vector<PlusSignVertex>> tempVertices = qMesh->_psVertices;

    // delete this
    _debugPoints.clear();
    _debugLines.clear();

    // implement right kite first, then left kite

    int meshWidth = qMesh-> GetWidth();
    int meshHeight = qMesh-> GetHeight();

    // right kite fixed boundaries (left and bottom)
    std::vector<AVector> leftBoundary1 = qMesh->GetABoundary(0, true, true);   // original
    std::vector<AVector> leftBoundary2 = qMesh->GetABoundary(0, true, false);     // conformal

    std::vector<AVector> bottomBoundary1 = qMesh->GetABoundary(meshHeight - 1, false, true);   // original
    std::vector<AVector> bottomBoundary2 = qMesh->GetABoundary(meshHeight - 1, false, false);     // conformal

    std::vector<std::pair<int, int>> leftPairIndices;
    std::vector<std::pair<int, int>> bottomPairIndices;
    std::vector<float> leftRatios;
    std::vector<float> bottomRatios;

    GetClosestIndicesAndRatios(leftBoundary1,   leftBoundary2,   leftPairIndices, leftRatios);
    GetClosestIndicesAndRatios(bottomBoundary1, bottomBoundary2, bottomPairIndices, bottomRatios);

    for(int a = 0; a < meshWidth; a++)
    {
        for(int b = 0; b < meshHeight; b++)
        {
            if(!qMesh->_psVertices[a][b].shouldMove)
            {
                _debugPoints.push_back(qMesh->_psVertices[a][b].position); // delete this
            }
            else if(a == 0 || b == meshHeight - 1)
            {
                tempVertices[a][b].position = qMesh->_opsVertices[a][b].position;
                _debugPoints.push_back(qMesh->_opsVertices[a][b].position);
            }
            else if(a == meshWidth - 1)
            {
                int l1 = leftPairIndices[b].first;
                int l2 = leftPairIndices[b].second;

                AVector ur = qMesh->_psVertices[a][l1].position;
                AVector br = qMesh->_psVertices[a][l2].position;

                AVector newPos = ur + (br - ur) * leftRatios[b];
                tempVertices[a][b].position = newPos;
                _debugPoints.push_back(newPos);
            }
            else if(b == 0)
            {
                int b1 = bottomPairIndices[a].first;
                int b2 = bottomPairIndices[a].second;

                AVector ul = qMesh->_psVertices[b1][b].position;
                AVector ur = qMesh->_psVertices[b2][b].position;

                AVector newPos = ul + (ur - ul) * bottomRatios[a];
                tempVertices[a][b].position = newPos;
                _debugPoints.push_back(newPos);
            }
            else
            {
                // todo : a bug where these indices can be -1
                int l1 = leftPairIndices[b].first;
                int l2 = leftPairIndices[b].second;
                int b1 = bottomPairIndices[a].first;
                int b2 = bottomPairIndices[a].second;

                AVector ul = qMesh->_psVertices[b1][l1].position;
                AVector ur = qMesh->_psVertices[b2][l1].position;
                AVector bl = qMesh->_psVertices[b1][l2].position;
                AVector br = qMesh->_psVertices[b2][l2].position;

                AVector newPos = UtilityFunctions::GetQuadrilateralPosition(ul, ur, bl, br, leftRatios[b], bottomRatios[a]);

                _debugPoints.push_back(newPos);
                tempVertices[a][b].position = newPos;
            }
        }
    }
    qMesh->_psVertices = tempVertices;
}

void ConformalMapping::GetClosestIndicesAndRatios(std::vector<AVector> boundary1,
                                                  std::vector<AVector> boundary2,
                                                  std::vector<std::pair<int, int>>& pairIndices,
                                                  std::vector<float>& ratios)
{
    for(uint i = 0; i < boundary1.size(); i++)
    {
        AVector pt0 = boundary1[i];
        float dist = std::numeric_limits<float>::max();
        int index1 = -1;
        int index2 = -1;
        for(uint j = 0; j < boundary2.size() - 1; j++)
        {
            AVector pt1 = boundary2[j];
            AVector pt2 = boundary2[j + 1];
            if(!UtilityFunctions::DoesAPointLieOnALine(pt0, ALine(pt1, pt2)))
                { continue; }

            float d = std::min(pt1.Distance(pt0), pt2.Distance(pt0));

            if(d < dist)
            {
                dist = d;
                index1 = j;
                index2 = j + 1;
            }
        }
        pairIndices.push_back(std::pair<int, int>(index1, index2));

        float dist1 = boundary2[index1].Distance(pt0);
        float dist2 = boundary2[index2].Distance(pt0);

        ratios.push_back(dist1 / (dist1 + dist2));
    }
}

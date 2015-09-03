#include "ConformalMapping.h"

#include "AVector.h"
#include "ALine.h"
#include "PlusSignVertex.h"

#include "UtilityFunctions.h"

ConformalMapping::ConformalMapping() :
    _iterDist(std::numeric_limits<float>::max())
{
}

void ConformalMapping::ConformalMappingOneStepSimple(std::vector<QuadMesh>& quadMeshes)
{
    this->_debugPoints.clear();
    this->_debugLines.clear();

    this->_iterDist = 0;
    for(uint a = 0; a < quadMeshes.size(); a++)
    {
        //if(quadMeshes[a]._quadMeshType == QuadMeshType::MESH_KITE)
        //{
            ConformalMappingOneStepSimple(&quadMeshes[a]);
        //}
    }
}

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
            {
                sumPositions += tempVertices[a - 1][b].position;
                numNeighbor++;
            }


            if(a < meshWidth - 1) // right
            {
                sumPositions = sumPositions + tempVertices[a + 1][b].position;
                numNeighbor++;
            }


            if(b > 0) // up
            {
                sumPositions += tempVertices[a][b - 1].position;
                numNeighbor++;
            }


            if(b < meshHeight - 1) // bottom
            {
                sumPositions += tempVertices[a][b + 1].position;
                numNeighbor++;
            }

            sumPositions  = sumPositions / (float)numNeighbor;

            if(numNeighbor < 4) { tempVertices[a][b].position = qMesh->GetClosestPointFromBorders(sumPositions); }
            else { tempVertices[a][b].position = sumPositions; }
        }
    }

    float sumDist = 0;
    for(int a = 0; a < meshWidth; a++)
    {
        for(int b = 0; b < meshHeight; b++)
        {
            sumDist += qMesh->_psVertices[a][b].position.Distance(tempVertices[a][b].position);
        }
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
        //if(quadMeshes[a]._quadMeshType == QuadMeshType::MESH_KITE)
        //{
        QuadMesh* prevQMesh = 0;
        QuadMesh* curQMesh = &quadMeshes[a];
        QuadMesh* nextQMesh = 0;

        if(a > 0) { prevQMesh = &quadMeshes[a - 1]; }
        if(a < quadMeshes.size() - 1) { nextQMesh = &quadMeshes[a + 1]; }

        ConformalMappingOneStep(prevQMesh, curQMesh, nextQMesh);
        //}
    }
}

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

PlusSignVertex ConformalMapping::GetNeighbor(int x, int y,
                                             NeighborDirection dir,
                                             QuadMesh* prevQMesh, QuadMesh* curQMesh, QuadMesh* nextQMesh,
                                             std::vector<std::vector<PlusSignVertex> > pTempVertices,
                                             std::vector<std::vector<PlusSignVertex> > cTempVertices,
                                             std::vector<std::vector<PlusSignVertex> > nTempVertices)
{

    int pWidth  = 0;
    int pHeight = 0;
    int cWidth  = curQMesh->GetWidth();
    int cHeight = curQMesh->GetHeight();
    int nWidth  = 0;
    int nHeight = 0;

    if(prevQMesh)
    {
        pWidth  = prevQMesh->GetWidth();
        pHeight = prevQMesh->GetHeight();
    }

    if(nextQMesh)
    {
        nWidth  = nextQMesh->GetWidth();
        nHeight = nextQMesh->GetHeight();
    }

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
            PlusSignVertex tempVert = pTempVertices[pWidth - 2][y];
            //_debugLines.push_back(ALine(curQMesh->_opsVertices[x][y].position, tempVert.position));
        }
        else if(dir == NeighborDirection::ND_DOWN)
        {
            PlusSignVertex tempVert = nTempVertices[1][nHeight - (x + 1)];
            //_debugLines.push_back(ALine(curQMesh->_opsVertices[x][y].position, tempVert.position));
        }
    }
    else if(curQMesh->_quadMeshType == QuadMeshType::MESH_KITE && !curQMesh->_isRightKite)
    {
        if(dir == NeighborDirection::ND_RIGHT)
        {
            PlusSignVertex tempVert = nTempVertices[1][y];
            //_debugLines.push_back(ALine(curQMesh->_opsVertices[x][y].position, tempVert.position));
        }
        else if(dir == NeighborDirection::ND_UP)
        {
            PlusSignVertex tempVert = pTempVertices[pWidth - 2][pHeight - (x + 1)];
            //_debugLines.push_back(ALine(curQMesh->_opsVertices[x][y].position, tempVert.position));
        }
    }
    else if(curQMesh->_quadMeshType == QuadMeshType::MESH_RECTANGLE)
    {
        if(dir == NeighborDirection::ND_LEFT && prevQMesh && prevQMesh->_isRightKite)
        {
            PlusSignVertex tempVert = pTempVertices[pWidth - (y + 1)][pHeight - 2];
            //_debugLines.push_back(ALine(curQMesh->_opsVertices[x][y].position, tempVert.position));
        }
        else if(dir == NeighborDirection::ND_LEFT && prevQMesh && !prevQMesh->_isRightKite)
        {
            PlusSignVertex tempVert = pTempVertices[pWidth - 2][y];
            //_debugLines.push_back(ALine(curQMesh->_opsVertices[x][y].position, tempVert.position));
        }
        else if(dir == NeighborDirection::ND_RIGHT && nextQMesh && nextQMesh->_isRightKite)
        {
            PlusSignVertex tempVert = nTempVertices[1][y];
            //_debugLines.push_back(ALine(curQMesh->_opsVertices[x][y].position, tempVert.position));
        }
        else if(dir == NeighborDirection::ND_RIGHT && nextQMesh && !nextQMesh->_isRightKite)
        {
            PlusSignVertex tempVert = nTempVertices[nWidth - (y + 1)][1];
            //_debugLines.push_back(ALine(curQMesh->_opsVertices[x][y].position, tempVert.position));
        }
    }


    /*
    if(curQMesh->_quadMeshType == QuadMeshType::MESH_KITE && curQMesh->_isRightKite)
    {
        if(dir == NeighborDirection::ND_LEFT)
        {
            PlusSignVertex tempVert = pTempVertices[pWidth - 1][y];
            //_debugPoints.push_back(tempVert.position);
        }
        else if(dir == NeighborDirection::ND_DOWN)
        {
            PlusSignVertex tempVert = nTempVertices[0][nHeight - (x + 1)];
            //_debugPoints.push_back(tempVert.position);
        }
    }
    else if(curQMesh->_quadMeshType == QuadMeshType::MESH_KITE && !curQMesh->_isRightKite)
    {
        if(dir == NeighborDirection::ND_RIGHT)
        {
            PlusSignVertex tempVert = nTempVertices[0][y];
            //_debugPoints.push_back(tempVert.position);
        }
        else if(dir == NeighborDirection::ND_UP)
        {
            PlusSignVertex tempVert = pTempVertices[pWidth - 1][pHeight - (x + 1)];
            //_debugPoints.push_back(tempVert.position);
        }
    }
    else if(curQMesh->_quadMeshType == QuadMeshType::MESH_RECTANGLE)
    {
        if(dir == NeighborDirection::ND_LEFT && prevQMesh && prevQMesh->_isRightKite)
        {
            PlusSignVertex tempVert = pTempVertices[pWidth - (y + 1)][pHeight - 1];
            //_debugPoints.push_back(tempVert.position);
        }
        else if(dir == NeighborDirection::ND_LEFT && prevQMesh && !prevQMesh->_isRightKite)
        {
            PlusSignVertex tempVert = pTempVertices[pWidth - 1][y];
            //_debugPoints.push_back(tempVert.position);
        }
        else if(dir == NeighborDirection::ND_RIGHT && nextQMesh && nextQMesh->_isRightKite)
        {
            PlusSignVertex tempVert = nTempVertices[0][y];
            //_debugPoints.push_back(tempVert.position);
        }
        else if(dir == NeighborDirection::ND_RIGHT && nextQMesh && !nextQMesh->_isRightKite)
        {
            PlusSignVertex tempVert = nTempVertices[nWidth - (y + 1)][0];
            //_debugPoints.push_back(tempVert.position);
        }
    }*/


    return PlusSignVertex();
}

void ConformalMapping::ConformalMappingOneStep(QuadMesh* prevQMesh, QuadMesh* curQMesh, QuadMesh* nextQMesh)
{
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
            // UNCOMMENT THIS
            if(!cTempVertices[a][b].shouldMove) { continue; }

            AVector curPos = cTempVertices[a][b].position;
            AVector sumPositions(0, 0);
            float sumArmLengths = 0;
            float sumArmAngles = 0;
            int numNeighbor = 0;

            // Neighbors
            PlusSignVertex lVertex = GetNeighbor(a, b, NeighborDirection::ND_LEFT,  prevQMesh, curQMesh, nextQMesh, pTempVertices, cTempVertices, nTempVertices);
            PlusSignVertex rVertex = GetNeighbor(a, b, NeighborDirection::ND_RIGHT, prevQMesh, curQMesh, nextQMesh, pTempVertices, cTempVertices, nTempVertices);
            PlusSignVertex uVertex = GetNeighbor(a, b, NeighborDirection::ND_UP,    prevQMesh, curQMesh, nextQMesh, pTempVertices, cTempVertices, nTempVertices);
            PlusSignVertex bVertex = GetNeighbor(a, b, NeighborDirection::ND_DOWN,  prevQMesh, curQMesh, nextQMesh, pTempVertices, cTempVertices, nTempVertices);

            // DELETE THIS
            //if(!cTempVertices[a][b].shouldMove) { continue; }

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



            if(numNeighbor < 4)
            {
                cTempVertices[a][b].position = curQMesh->GetClosestPointFromBorders(sumPositions);
            }
            else
            {
                cTempVertices[a][b].position = sumPositions;
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
    curQMesh->_psVertices = cTempVertices;
}

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

    // debugging
    //_debugPoints = bottomBoundary2;

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
                int l1 = leftPairIndices[b].first;
                int l2 = leftPairIndices[b].second;

                int b1 = bottomPairIndices[a].first;
                int b2 = bottomPairIndices[a].second;

                //if(l1 == -1 || l2 == -1 || b1 == -1 || b2 == -1)
                //{
                //    std::cout << "shit\n";
                //    continue;
                //}

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
    for(int i = 0; i < boundary1.size(); i++)
    {
        AVector pt0 = boundary1[i];
        float dist = std::numeric_limits<float>::max();
        int index1 = -1;
        int index2 = -1;
        for(int j = 0; j < boundary2.size() - 1; j++)
        {
            AVector pt1 = boundary2[j];
            AVector pt2 = boundary2[j + 1];
            if(!UtilityFunctions::DoesAPointLieOnALine(pt0, ALine(pt1, pt2)))
            {
                continue;
            }

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

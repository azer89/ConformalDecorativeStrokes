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
    this->_iterDist = 0;
    for(uint a = 0; a < quadMeshes.size(); a++)
    {
        if(quadMeshes[a]._quadMeshType == QuadMeshType::MESH_KITE)
        {
            ConformalMappingOneStepSimple(&quadMeshes[a]);
        }
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
            if(!tempVertices[a][b].shouldMove)
                { continue; }

            AVector sumPositions(0, 0);
            int numNeighbor = 0;

            // left
            if(a > 0)
            {
                sumPositions += tempVertices[a - 1][b].position;
                numNeighbor++;
            }

            // right
            if(a < meshWidth - 1)
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
            if(b < meshHeight - 1)
            {
                sumPositions += tempVertices[a][b + 1].position;
                numNeighbor++;
            }

            sumPositions  = sumPositions / (float)numNeighbor;

            if(numNeighbor < 4)
            {
                tempVertices[a][b].position = qMesh->GetClosestPointFromBorders(sumPositions);
            }
            else
            {
                tempVertices[a][b].position = sumPositions;
            }
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
    this->_iterDist = 0;
    for(uint a = 0; a < quadMeshes.size(); a++)
    {
        if(quadMeshes[a]._quadMeshType == QuadMeshType::MESH_KITE)
        {
            ConformalMappingOneStep(&quadMeshes[a]);
        }
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

void ConformalMapping::ConformalMappingOneStep(QuadMesh *qMesh)
{
    std::vector<std::vector<PlusSignVertex>> tempVertices = qMesh->_psVertices;
    int meshWidth = qMesh->GetWidth();
    int meshHeight = qMesh->GetHeight();

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
                tempVertices[a][b].position = qMesh->GetClosestPointFromBorders(sumPositions);
            }
            else
            {
                tempVertices[a][b].position = sumPositions;
            }
        }
    }

    float sumDist = 0;
    for(int a = 0; a < meshWidth; a++)
    {
        for(int b = 0; b < meshHeight; b++)
            { sumDist += qMesh->_psVertices[a][b].position.Distance(tempVertices[a][b].position); }
    }
    _iterDist += sumDist;
    qMesh->_psVertices = tempVertices;
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

    // debugging
    /*_debugLines.clear();
    for(uint a = 0; a < bottomBoundary1.size(); a++)
    {
        AVector pt0 = bottomBoundary1[a];
        int idx1 = bottomPairIndices[a].first;
        int idx2 = bottomPairIndices[a].second;
        AVector pt1 = qMesh->_psVertices[idx1][meshHeight - 2].position;
        AVector pt2 = qMesh->_psVertices[idx2][meshHeight - 2].position;
        _debugLines.push_back(ALine(pt0, pt1));
        _debugLines.push_back(ALine(pt0, pt2));
    }

    //_debugLines.clear();
    for(uint a = 0; a < leftBoundary1.size(); a++)
    {
        AVector pt0 = leftBoundary1[a];
        int idx1 = leftPairIndices[a].first;
        int idx2 = leftPairIndices[a].second;
        AVector pt1 = qMesh->_psVertices[1][idx1].position;
        AVector pt2 = qMesh->_psVertices[1][idx2].position;
        _debugLines.push_back(ALine(pt0, pt1));
        _debugLines.push_back(ALine(pt0, pt2));
    }*/
    //std::cout << leftBoundary1.size() << " " << _debugPoints.size() << " " << _debugLines.size() << "\n";

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

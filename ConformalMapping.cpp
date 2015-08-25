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
    std::vector<std::vector<PlusSignVertex>> tempVertices = qMesh->_plusSignVertices;
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
            sumDist += qMesh->_plusSignVertices[a][b].position.Distance(tempVertices[a][b].position);
        }
    }
    _iterDist = sumDist;
    qMesh->_plusSignVertices = tempVertices;
}

void ConformalMapping::ConformalMappingOneStep(std::vector<QuadMesh>& quadMeshes)
{
    this->_iterDist = 0;
    for(uint a = 0; a < quadMeshes.size(); a++)
    {
        if(quadMeshes[a]._quadMeshType == QuadMeshType::MESH_KITE)
        {
            QuadMesh oriQMesh = quadMeshes[a];
            //oriQMesh = QuadMesh();

            ConformalMappingOneStep(&quadMeshes[a]);

            //MappingInterpolation(oriQMesh, &quadMeshes[a]);
        }
    }
}

void ConformalMapping::ConformalMappingOneStep(QuadMesh *qMesh)
{
    std::vector<std::vector<PlusSignVertex>> tempVertices = qMesh->_plusSignVertices;
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
            { sumDist += qMesh->_plusSignVertices[a][b].position.Distance(tempVertices[a][b].position); }
    }
    _iterDist += sumDist;
    qMesh->_plusSignVertices = tempVertices;
}

void ConformalMapping::MappingInterpolation(QuadMesh oriQMesh, QuadMesh *qMesh)
{
    // implement right kite first, then left kite

    std::vector<std::vector<PlusSignVertex>> tempVertices = oriQMesh._plusSignVertices;

    int meshWidth = qMesh-> GetWidth();
    int meshHeight = qMesh-> GetHeight();

    // right kite fixed boundaries (left and bottom)
    std::vector<AVector> leftBoundary1 = oriQMesh.GetABoundary(0, true);   // original
    std::vector<AVector> leftBoundary2 = qMesh->GetABoundary(0, true);     // conformal

    std::vector<AVector> bottomBoundary1 = oriQMesh.GetABoundary(meshHeight - 1, false);   // original
    std::vector<AVector> bottomBoundary2 = qMesh->GetABoundary(meshHeight - 1, false);     // conformal

    std::vector<std::pair<int, int>> xPairIndices;
    std::vector<std::pair<int, int>> yPairIndices;
    std::vector<float> xRatios;
    std::vector<float> yRatios;

    GetClosestIndicesAndRatios(leftBoundary1,   leftBoundary2,   xPairIndices, xRatios);
    GetClosestIndicesAndRatios(bottomBoundary1, bottomBoundary2, yPairIndices, yRatios);

    for(int a = 0; a < meshWidth; a++)
    {
        for(int b = 0; b < meshHeight; b++)
        {
            int xIndex1 = xPairIndices[a].first;
            int xIndex2 = xPairIndices[a].second;
            float xRatio = xRatios[a];

            int yIndex1 = yPairIndices[b].first;
            int yIndex2 = yPairIndices[b].second;
            float yRatio = yRatios[a];

            AVector ulVec = qMesh->_plusSignVertices[xIndex1][yIndex1].position;
            AVector urVec = qMesh->_plusSignVertices[xIndex1][yIndex2].position;
            AVector blVec = qMesh->_plusSignVertices[xIndex2][yIndex1].position;
            AVector brVec = qMesh->_plusSignVertices[xIndex2][yIndex2].position;
        }
    }
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

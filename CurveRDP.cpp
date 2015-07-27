/**
 *
 * Reza Adhitya Saputra (reza.adhitya.saputra@gmail.com)
 * Version: 2015
 *
 */


#include "CurveRDP.h"

#include <limits>

CurveRDP::CurveRDP()
{
}

float CurveRDP::PerpendicularDistance(AVector p, AVector p1, AVector p2)
{
    float result;
    if (abs(p1.x - p2.x) < std::numeric_limits<float>::epsilon()) { result = abs(p.x - p1.x); }
    else
    {
        float slope = (p2.y - p1.y) / (p2.x - p1.x);
        float intercept = p1.y - (slope * p1.x);
        result = abs(slope * p.x - p.y + intercept) / sqrt(pow(slope, 2) + 1);
    }
    return result;
}

void CurveRDP::SimplifyRDP(std::vector<AVector>& oldCurves, std::vector<AVector>& newCurves, float epsilon)
{
    newCurves.clear();
    newCurves.push_back(oldCurves[0]);
    SimplifyRDPRecursive(oldCurves, newCurves, epsilon, 0, oldCurves.size() - 1);
    newCurves.push_back(oldCurves[oldCurves.size() - 1]);
}

void CurveRDP::SimplifyRDPRecursive(std::vector<AVector>& oldCurves, std::vector<AVector>& newCurves, float epsilon, int startIndex, int endIndex)
{
    AVector firstPoint = oldCurves[startIndex];
    AVector lastPoint = oldCurves[endIndex];

    int index = -1;
    float dist = std::numeric_limits<float>::min();
    for (int i = startIndex + 1; i < endIndex; i++)
    {
        float cDist = PerpendicularDistance(oldCurves[i], firstPoint, lastPoint);
        if (cDist > dist)
        {
            dist = cDist;
            index = i;
        }
    }

    if (index != -1 && dist > epsilon)
    {
        SimplifyRDPRecursive(oldCurves, newCurves, epsilon, startIndex, index);
        newCurves.push_back(oldCurves[index]);
        SimplifyRDPRecursive(oldCurves, newCurves, epsilon, index, endIndex);
    }
}

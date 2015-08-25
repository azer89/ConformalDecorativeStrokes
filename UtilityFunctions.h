#ifndef UTILITYFUNCTIONS_H
#define UTILITYFUNCTIONS_H

#include "AVector.h"
#include "ALine.h"

#include <vector>

class UtilityFunctions
{
public:
    UtilityFunctions();

    static void UniformResample(std::vector<AVector>& oriCurve, std::vector<AVector>& resampleCurve, float resample_size);

    static void DivideLines(std::vector<AVector>& oriCurve, std::vector<AVector>& resampleCurve, float resample_size);
    static void CombineLines(std::vector<AVector>& oriCurve, std::vector<AVector>& resampleCurve, float resample_size);

    static double CurveLength(std::vector<AVector> curves);

    static void GetBisectorJoints(ALine curLine, ALine prevLine, ALine nextLine, double t0, double t1, AVector* pA, AVector* pB, AVector* pC, AVector* pD);

    static void GetMiterJoints(ALine prevLine, ALine curLine, double t0, double t1, AVector* pA, AVector* pB);

    static bool CheckCollinearCase(ALine ray1, ALine ray2);
    static AVector GetFiniteIntersection(ALine rayA, ALine rayB);
    static AVector GetClosestPoint(AVector v, AVector w, AVector p);
    static float GetRotation(AVector pt1, AVector pt2);
    static bool CheckHorizontalVerticalCase(ALine ray1, ALine ray2);

    // rotate a point around center (0, 0)
    static AVector Rotate(AVector pt, float rad);
    static AVector Rotate(AVector pt, AVector centerPt, float rad);

    AVector GetCoordinateFromQuadrilateral(AVector ul, AVector ur, AVector bl, AVector br, float xRatio, float yRatio);
};

#endif // UTILITYFUNCTIONS_H

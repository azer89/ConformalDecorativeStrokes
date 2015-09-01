#ifndef CURVERDP_H
#define CURVERDP_H

#include "AVector.h"

#include <vector>

class CurveRDP
{
public:
    CurveRDP();

    /**
     * distance a point to a finite line
     */
    static float PerpendicularDistance(AVector p, AVector p1, AVector p2) ;

    /**
     * RDP partitioning
     */
    static void SimplifyRDP(std::vector<AVector>& oldCurves, std::vector<AVector>& newCurves, float epsilon);

    /**
     * RDP partitioning
     */
    static void SimplifyRDPRecursive(std::vector<AVector>& oldCurves, std::vector<AVector>& newCurves, float epsilon, int startIndex, int endIndex);
};

#endif // CURVERDP_H

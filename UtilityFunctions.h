#ifndef UTILITYFUNCTIONS_H
#define UTILITYFUNCTIONS_H

#include "AVector.h"
#include "ALine.h"

#include <vector>

class UtilityFunctions
{
public:
    UtilityFunctions();

    static void UniformResample(std::vector<AVector>& oriCurve, std::vector<AVector>& resampleCurve, double resample_size);
    static double CurveLength(std::vector<AVector> curves);

    static void GetSegmentPoints(ALine curLine,
                                 ALine prevLine,
                                 ALine nextLine,
                                 double t0,
                                 double t1,
                                 AVector* pA,
                                 AVector* pB,
                                 AVector* pC,
                                 AVector* pD);
};

#endif // UTILITYFUNCTIONS_H

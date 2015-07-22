#include "UtilityFunctions.h"

UtilityFunctions::UtilityFunctions()
{
}

void UtilityFunctions::UniformResample(std::vector<AVector>& oriCurve, std::vector<AVector>& resampleCurve, double resample_size  )
{
    resampleCurve.clear();

    if(oriCurve.size() < 2) return;

    double pl_length = CurveLength(oriCurve);
    int N = pl_length / resample_size;

    for(int a = 0; a <= N; a++) { resampleCurve.push_back(AVector(0,0)); }
    resampleCurve[0].x = oriCurve[0].x;
    resampleCurve[0].y = oriCurve[0].y;
    resampleCurve[N].x = oriCurve[oriCurve.size()-1].x;
    resampleCurve[N].y = oriCurve[oriCurve.size()-1].y;

    int curr = 0;
    double dist = 0.0;
    for (int i = 1; i < N; )
    {
        double last_dist = oriCurve[curr].Distance(oriCurve[curr+1]);

        dist += last_dist;

        if (dist >= resample_size)
        {
            //put a point on line
            double _d = last_dist - (dist-resample_size);
            AVector cp(oriCurve[curr].x, oriCurve[curr].y);
            AVector cp1(oriCurve[curr+1].x, oriCurve[curr+1].y);

            AVector dirv = cp1-cp;
            dirv = dirv * (1.0 / dirv.Length());

            resampleCurve[i] = cp + dirv * _d;
            i++;

            dist = last_dist - _d; //remaining dist

            //if remaining dist to next point needs more sampling... (within some epsilon)
            while (dist - resample_size > 1e-3)
            {
                resampleCurve[i] = resampleCurve[i-1] + dirv * resample_size;
                dist -= resample_size;
                i++;
            }
        }
        curr++;
    }
}

void UtilityFunctions::GetSegmentPoints(ALine curLine,
                                        ALine prevLine,
                                        ALine nextLine,
                                        double t0,
                                        double t1,
                                        AVector* pA,
                                        AVector* pB,
                                        AVector* pC,
                                        AVector* pD)
{
    // point
    AVector p0 = curLine.GetPointA();
    AVector p1 = curLine.GetPointB();

    // direction
    AVector prevDirNorm  = prevLine.Direction().Norm();
    AVector curDirNorm   = curLine.Direction().Norm();
    AVector nextDirNorm  = nextLine.Direction().Norm();

    AVector d0;
    AVector d1;

    if(!prevLine.Invalid() && !nextLine.Invalid())		// normal drawing
    {
        d0 = (prevDirNorm + curDirNorm) / 2.0;
        d1 = (nextDirNorm + curDirNorm) / 2.0;
    }
    else if(prevLine.Invalid() && nextLine.Invalid())	// line consists of only one line segment
    {
        d0 = curDirNorm;
        d1 = curDirNorm;
    }
    else if(prevLine.Invalid())							// draw starting segment
    {
        d0 = curDirNorm;
        d1 = (nextDirNorm + curDirNorm) / 2.0;
    }
    else if(nextLine.Invalid())							// draw ending segment
    {
        d0 = (prevDirNorm + curDirNorm) / 2.0;
        d1 = curDirNorm;
    }

    // thickness
    float p0HalfWidth = t0;
    float p1HalfWidth = t1;

    if(p0HalfWidth <= 0.0) p0HalfWidth = 0.5;
    if(p1HalfWidth <= 0.0) p1HalfWidth = 0.5;

    d0 = d0.Norm();
    d1 = d1.Norm();

    d0 *= p0HalfWidth;
    d1 *= p1HalfWidth;

    AVector d0Left (-d0.y,  d0.x);
    AVector d0Right( d0.y, -d0.x);
    AVector d1Left (-d1.y,  d1.x);
    AVector d1Right( d1.y, -d1.x);

    *pA = p0 + d0Left;
    *pB = p0 + d0Right;
    *pC = p1 + d1Left;
    *pD = p1 + d1Right;
}

double UtilityFunctions::CurveLength(std::vector<AVector> curves)
{
    double length = 0.0;
    for(size_t a = 1; a < curves.size(); a++) { length += curves[a].Distance(curves[a-1]); }
    return length;
}

#include "UtilityFunctions.h"

UtilityFunctions::UtilityFunctions()
{
}

void UtilityFunctions::UniformResample(std::vector<AVector>& oriCurve, std::vector<AVector>& resampleCurve, float resample_size  )
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

bool UtilityFunctions::CheckCollinearCase(ALine ray1, ALine ray2)
{
    float eps_val = std::numeric_limits<float>::epsilon() * 1000;

    AVector midPoint = ray1.GetPointA() + (ray2.GetPointA() - ray1.GetPointA()) * 0.5f;

    float u1 = (midPoint.x - ray1.GetPointA().x) / ray1.GetPointB().x;
    float u2 = (midPoint.y - ray1.GetPointA().y) / ray1.GetPointB().y;
    float v1 = (midPoint.x - ray2.GetPointA().x) / ray2.GetPointB().x;
    float v2 = (midPoint.y - ray2.GetPointA().y) / ray2.GetPointB().y;

    if(abs(u1 - u2) < eps_val && abs(v1 - v2) < eps_val && u1 > 0 && u2 > 0 && v1 > 0 && v2 > 0)
        { return true; }

    return false;
}

bool UtilityFunctions::CheckHorizontalVerticalCase(ALine ray1, ALine ray2)
{
    float eps_val = std::numeric_limits<float>::epsilon() * 1000;
    AVector midPoint = ray1.GetPointA() + (ray2.GetPointA() - ray1.GetPointA()) * 0.5f;

    float u1 = (midPoint.x - ray1.GetPointA().x) / ray1.GetPointB().x;
    float u2 = (midPoint.y - ray1.GetPointA().y) / ray1.GetPointB().y;
    float v1 = (midPoint.x - ray2.GetPointA().x) / ray2.GetPointB().x;
    float v2 = (midPoint.y - ray2.GetPointA().y) / ray2.GetPointB().y;

    // vertical case
    if(abs(ray1.GetPointB().x) < eps_val && abs(ray2.GetPointB().x) <  eps_val &&
       ray1.GetPointB().y != 0 && ray2.GetPointB().y != 0 &&
       u2 > 0 && v2 > 0 )
    {
        return true;
    }
    // horizontal case
    else if(ray1.GetPointB().x != 0 && ray2.GetPointB().x != 0 &&
            abs(ray1.GetPointB().y) < eps_val && abs(ray2.GetPointB().y) < eps_val &&
            u1 > 0 && v1 > 0)
    {
        return true;
    }
    return false;
}

AVector UtilityFunctions::GetFiniteIntersection(ALine rayA, ALine rayB)
{
    float eps_val = std::numeric_limits<float>::epsilon() * 1000.0f;

    AVector intersectionPoint;

    float dx = rayB.GetPointA().x - rayA.GetPointA().x;
    float dy = rayB.GetPointA().y - rayA.GetPointA().y;
    float det = rayB.GetPointB().x * rayA.GetPointB().y - rayB.GetPointB().y * rayA.GetPointB().x;
    float u = (dy * rayB.GetPointB().x - dx * rayB.GetPointB().y) / det;
    float v = (dy * rayA.GetPointB().x - dx * rayA.GetPointB().y) / det;

    if((det > eps_val || det < -eps_val) && u > 0 && v > 0)
    {
        intersectionPoint = rayA.GetPointA() + rayA.GetPointB() * u;
    }
    else if(CheckCollinearCase(rayA, rayB) || CheckHorizontalVerticalCase(rayA, rayB))
    {
        intersectionPoint = rayA.GetPointA() + (rayB.GetPointA() - rayA.GetPointA()) * 0.5f;
    }

    return intersectionPoint;
}

void UtilityFunctions::GetMiterJoints(ALine prevLine,
                                      ALine curLine,
                                      double t0,
                                      double t1,
                                      AVector* pA,
                                      AVector* pB)
{
    AVector pDir  = prevLine.Direction();
    AVector cDir  = curLine.Direction();

    pDir = pDir.Norm();
    cDir = cDir.Norm();

    AVector pDirRight(-pDir.y,   pDir.x);
    AVector pDirLeft(pDir.y,  -pDir.x);

    AVector cDirRight(-cDir.y,   cDir.x);
    AVector cDirLeft(cDir.y,  -cDir.x);

    ALine pLeftRay (prevLine.GetPointA() + pDirLeft  * t0, pDir);
    ALine pRightRay(prevLine.GetPointA() + pDirRight * t1, pDir);

    ALine cLeftInvRay (curLine.GetPointB()  + cDirLeft  * t0, AVector(-cDir.x, -cDir.y));
    ALine cRightInvRay(curLine.GetPointB()  + cDirRight * t1, AVector(-cDir.x, -cDir.y));

    *pA = GetFiniteIntersection(pLeftRay,  cLeftInvRay);
    *pB = GetFiniteIntersection(pRightRay, cRightInvRay);
}

void UtilityFunctions::GetBisectorJoints(ALine curLine,
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

void UtilityFunctions::DivideLines(std::vector<AVector>& oriCurve, std::vector<AVector>& resampleCurve, float resample_size)
{
    for(uint a = 0; a < oriCurve.size() - 1; a++)
    {
        AVector pt1 = oriCurve[a];
        AVector pt2 = oriCurve[a+1];
        float dist = pt1.Distance(pt2);

        resampleCurve.push_back(pt1);

        if(dist < resample_size) { continue; }

        float deltaDist = resample_size;
        AVector dirVec = (pt2 - pt1).Norm();

        float iterDist = deltaDist;
        while(iterDist < dist)
        {
            resampleCurve.push_back(pt1 + dirVec * iterDist);
            iterDist += deltaDist;
        }
    }

    resampleCurve.push_back(oriCurve[oriCurve.size() - 1]);
}

void UtilityFunctions::CombineLines(std::vector<AVector>& oriCurve, std::vector<AVector>& resampleCurve, float resample_size)
{
    uint iter = 0;
    while(iter < oriCurve.size())
    {
        AVector pt1 = oriCurve[iter];
        resampleCurve.push_back(pt1);

        float iterDist = 0;
        while(iterDist < resample_size)
        {
            iter++;
            AVector pt2 = oriCurve[iter];
            iterDist = pt1.Distance(pt2);
        }
    }
}


float UtilityFunctions::GetRotation(AVector pt1, AVector pt2)
{
    float perpDot = pt1.x * pt2.y - pt1.y * pt2.x;
    float rotVal =  (float)atan2(perpDot, pt1.Dot(pt2));
    return rotVal;
}


double UtilityFunctions::CurveLength(std::vector<AVector> curves)
{
    double length = 0.0;
    for(size_t a = 1; a < curves.size(); a++) { length += curves[a].Distance(curves[a-1]); }
    return length;
}

// return the index of the closest line
int UtilityFunctions::GetClosestIndex(std::vector<std::vector<AVector>> lines, AVector pt)
{
    int index = -1;
    float dist = std::numeric_limits<float>::max();
    for(uint a = 0; a < lines.size(); a++)
    {
        AVector cPt = GetClosestPoint(lines[a], pt);
        if(pt.Distance(cPt) < dist)
        {
            dist = pt.Distance(cPt);
            index = a;
        }
    }
    return index;
}

AVector UtilityFunctions::GetClosestPoint(std::vector<std::vector<AVector> > lines, AVector pt)
{
    AVector closestPt = pt;
    float dist = std::numeric_limits<float>::max();
    for(uint a = 0; a < lines.size(); a++)
    {
        AVector cPt = GetClosestPoint(lines[a], pt);
        if(pt.Distance(cPt) < dist)
        {
            dist = pt.Distance(cPt);
            closestPt = cPt;
        }
    }
    return closestPt;
}

// return a point on the closest line
AVector UtilityFunctions::GetClosestPoint(std::vector<AVector> lines, AVector pt)
{
    AVector closestPt = pt;
    float dist = std::numeric_limits<float>::max();
    for(uint a = 0; a < lines.size() - 1; a++)
    {
        AVector pt1 = lines[a];
        AVector pt2 = lines[a+1];
        AVector cPt = UtilityFunctions::GetClosestPoint(pt1, pt2, pt);
        if(pt.Distance(cPt) < dist)
        {
            dist = pt.Distance(cPt);
            closestPt = cPt;
        }
    }
    return closestPt;
}

AVector UtilityFunctions::GetClosestPoint(std::vector<ALine> lines, AVector pt)
{
    AVector closestPt = pt;
    float dist = std::numeric_limits<float>::max();
    for(uint a = 0; a < lines.size(); a++)
    {
        AVector cPt = UtilityFunctions::GetClosestPoint(lines[a].GetPointA(), lines[a].GetPointB(), pt);
        if(pt.Distance(cPt) < dist)
        {
            dist = pt.Distance(cPt);
            closestPt = cPt;
        }
    }
    return closestPt;
}

AVector UtilityFunctions::GetClosestPoint(AVector v, AVector w, AVector pt)
{
    float eps_float = std::numeric_limits<float>::epsilon();
    float l2 = v.DistanceSquared(w);
    if (l2 > -eps_float && l2 < eps_float) return v;
    float t = (pt - v).Dot(w - v) / l2;
    if (t < 0.0)	  { return  v; }
    else if (t > 1.0) { return  w; }
    return v + (w - v) * t;
}

AVector UtilityFunctions::Rotate(AVector pt, float rad)
{
    double cs = cos(rad);
    double sn = sin(rad);

    double x = pt.x * cs - pt.y * sn;
    double y = pt.x * sn + pt.y * cs;

    return AVector(x, y);
}

AVector UtilityFunctions::Rotate(AVector pt, AVector centerPt, float rad)
{
    AVector newVec = Rotate(pt - centerPt, rad);
    return newVec + centerPt;
}

bool UtilityFunctions::DoesAPointLieOnALine(AVector pt, ALine ln)
{

    float lineLength = ln.Magnitude();
    float dist1 = ln.GetPointA().Distance(pt);
    float dist2 = ln.GetPointB().Distance(pt);

    float diff = std::abs(lineLength - dist1 - dist2);

    if(diff < std::numeric_limits<float>::epsilon() * 1000 )
    {
        return true;
    }

    return false;
}

AVector UtilityFunctions::GetQuadrilateralPosition(AVector ul, AVector ur, AVector bl, AVector br, float verticalRatio, float horizontalRatio)
{
    AVector lDir = ul.DirectionTo(bl);
    AVector rDir = ur.DirectionTo(br);
    AVector uDir = ul.DirectionTo(ur);
    AVector bDir = bl.DirectionTo(br);

    AVector vPt1 = ul + lDir * verticalRatio;
    AVector vPt2 = ur + rDir * verticalRatio;
    ALine vRay(vPt1, vPt1.DirectionTo(vPt2).Norm());

    AVector hPt1 = ul + uDir * horizontalRatio;
    AVector hPt2 = bl + bDir * horizontalRatio;
    ALine hRay(hPt1, hPt1.DirectionTo(hPt2).Norm());

    return GetFiniteIntersection(vRay, hRay);
}

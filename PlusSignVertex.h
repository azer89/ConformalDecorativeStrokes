#ifndef PLUSSIGNVERTEX_H
#define PLUSSIGNVERTEX_H

#include "AVector.h"
#include <QVector2D>
#include <QVector3D>

/**
 * See a related paper: "Warping Pictures Nicely"
 */
struct PlusSignVertex
{
    AVector position;

    float armLength;
    float angle;
    bool shouldMove;    // cannot move at all

    bool _isBoundaryRibConstrained; // maintain perpendicular segment boundary
    bool _isSlideConstrained;

    bool isValid;

public:

    PlusSignVertex(AVector position)
    {
        this->position = position;

        this->armLength = 0;
        this->angle = 0;
        this->shouldMove = true;

        this->isValid = true;
        this->_isBoundaryRibConstrained = false;
        this->_isSlideConstrained = false;
    }

    PlusSignVertex(AVector position, bool shouldMove)
    {
        this->position = position;

        this->armLength = 0;
        this->angle = 0;
        this->shouldMove = shouldMove;

        this->isValid = true;
        this->_isBoundaryRibConstrained = false;
        this->_isSlideConstrained = false;
    }

    PlusSignVertex()
    {
        this->position = AVector(-1, -1);

        this->armLength = 0;
        this->angle = 0;
        this->shouldMove = true;

        this->isValid = false;
        this->_isBoundaryRibConstrained = false;
        this->_isSlideConstrained = false;
    }

    bool IsValid()
    {
        return this->isValid;
    }
};


#endif // PLUSSIGNVERTEX_H

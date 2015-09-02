#ifndef PLUSSIGNVERTEX_H
#define PLUSSIGNVERTEX_H

#include "AVector.h"
#include <QVector2D>
#include <QVector3D>

/**
 * See related paper: "Warping Pictures Nicely"
 */
struct PlusSignVertex
{
    AVector position;
    QVector2D texCoord; // should delete this because it is useless
    QVector3D color;

    float armLength;
    float angle;
    bool shouldMove;                // cannot move at all
    //bool midVerticalConstrained;    // is fixed to a junction rib
    //bool midHorizontalConstrained;  // is fixed to a spine

    bool isValid;

public:

    PlusSignVertex(AVector position, QVector2D texCoord, QVector3D color)
    {
        this->position = position;
        this->texCoord = texCoord;
        this->color = color;

        this->armLength = 0;
        this->angle = 0;
        this->shouldMove = true;
        //this->midVerticalConstrained = false;
        //this->midHorizontalConstrained = false;

        this->isValid = true;
    }

    PlusSignVertex(AVector position, QVector2D texCoord)
    {
        this->position = position;
        this->texCoord = texCoord;
        this->color = QVector3D();

        this->armLength = 0;
        this->angle = 0;
        this->shouldMove = true;
        //this->midVerticalConstrained = false;
        //this->midHorizontalConstrained = false;

        this->isValid = true;
    }

    PlusSignVertex(AVector position)
    {
        this->position = position;
        this->texCoord = QVector2D();
        this->color = QVector3D();

        this->armLength = 0;
        this->angle = 0;
        this->shouldMove = true;
        //this->midVerticalConstrained = false;
        //this->midHorizontalConstrained = false;

        this->isValid = true;
    }

    /*PlusSignVertex(AVector position, bool shouldMove, bool midVerticalConstrained, bool midHorizontalConstrained)
    {
        this->position = position;
        this->texCoord = QVector2D();
        this->color = QVector3D();

        this->armLength = 0;
        this->angle = 0;
        this->shouldMove = shouldMove;
        //this->midVerticalConstrained = midVerticalConstrained;
        //this->midHorizontalConstrained = midHorizontalConstrained;

        this->isValid = true;
    }*/

    /*PlusSignVertex(AVector position, bool shouldMove, bool midVerticalConstrained)
    {
        this->position = position;
        this->texCoord = QVector2D();
        this->color = QVector3D();

        this->armLength = 0;
        this->angle = 0;
        this->shouldMove = shouldMove;
        this->midVerticalConstrained = midVerticalConstrained;
        this->midHorizontalConstrained = false;

        this->isValid = true;
    }*/

    PlusSignVertex(AVector position, bool shouldMove)
    {
        this->position = position;
        this->texCoord = QVector2D();
        this->color = QVector3D();

        this->armLength = 0;
        this->angle = 0;
        this->shouldMove = shouldMove;
        //this->midVerticalConstrained = false;
        //this->midHorizontalConstrained = false;

        this->isValid = true;
    }

    PlusSignVertex()
    {
        this->position = AVector(-1, -1);
        this->texCoord = QVector2D(0, 0);
        this->color = QVector3D(0, 0, 0);

        this->armLength = 0;
        this->angle = 0;
        this->shouldMove = true;
        //this->midVerticalConstrained = false;
        //this->midHorizontalConstrained = false;

        this->isValid = false;
    }

    bool IsValid()
    {
        return this->isValid;
        /*
        float eps_val = std::numeric_limits<float>::epsilon() * 1000;

        if(std::abs(this->position.x + 1.0f) < eps_val &&
           std::abs(this->position.y + 1.0f) < eps_val)
        {
            return false;
        }

        return true;
        */
    }
};


#endif // PLUSSIGNVERTEX_H

#ifndef PLUSSIGNVERTEX_H
#define PLUSSIGNVERTEX_H

#include "AVector.h"
#include <QVector2D>
#include <QVector3D>

struct PlusSignVertex
{
    AVector position;
    QVector2D texCoord; // should delete this because it is useless
    QVector3D color;

    float armLength;
    float angle;
    bool shouldMove;
    bool midVerticalConstrained;

public:

    PlusSignVertex(AVector position, QVector2D texCoord, QVector3D color)
    {
        this->position = position;
        this->texCoord = texCoord;
        this->color = color;

        this->armLength = 0;
        this->angle = 0;
        this->shouldMove = true;
        this->midVerticalConstrained = false;
    }

    PlusSignVertex(AVector position, QVector2D texCoord)
    {
        this->position = position;
        this->texCoord = texCoord;
        this->color = QVector3D();

        this->armLength = 0;
        this->angle = 0;
        this->shouldMove = true;
        this->midVerticalConstrained = false;
    }

    PlusSignVertex(AVector position)
    {
        this->position = position;
        this->texCoord = QVector2D();
        this->color = QVector3D();

        this->armLength = 0;
        this->angle = 0;
        this->shouldMove = true;
        this->midVerticalConstrained = false;
    }

    PlusSignVertex(AVector position, bool shouldMove, bool midVerticalConstrained)
    {
        this->position = position;
        this->texCoord = QVector2D();
        this->color = QVector3D();

        this->armLength = 0;
        this->angle = 0;
        this->shouldMove = shouldMove;
        this->midVerticalConstrained = midVerticalConstrained;
    }

    PlusSignVertex(AVector position, bool shouldMove)
    {
        this->position = position;
        this->texCoord = QVector2D();
        this->color = QVector3D();

        this->armLength = 0;
        this->angle = 0;
        this->shouldMove = shouldMove;
        this->midVerticalConstrained = false;
    }

    PlusSignVertex()
    {
        this->position = AVector(0, 0);
        this->texCoord = QVector2D(0, 0);
        this->color = QVector3D(0, 0, 0);

        this->armLength = 0;
        this->angle = 0;
        this->shouldMove = true;
        this->midVerticalConstrained = false;
    }
};


#endif // PLUSSIGNVERTEX_H

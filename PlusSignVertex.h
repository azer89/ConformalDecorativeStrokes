#ifndef PLUSSIGNVERTEX_H
#define PLUSSIGNVERTEX_H

#include "AVector.h"
#include <QVector2D>
#include <QVector3D>

struct PlusSignVertex
{
    AVector position;
    QVector2D texCoord;
    QVector3D color;

public:

    PlusSignVertex(AVector position, QVector2D texCoord, QVector3D color)
    {
        this->position = position;
        this->texCoord = texCoord;
        this->color = color;
    }

    PlusSignVertex(AVector position, QVector2D texCoord)
    {
        this->position = position;
        this->texCoord = texCoord;
        this->color = QVector3D();
    }

    PlusSignVertex()
    {
        this->position = AVector();
        this->texCoord = QVector2D();
        this->color = QVector3D();
    }
};


#endif // PLUSSIGNVERTEX_H

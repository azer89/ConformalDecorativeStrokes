#ifndef VERTEXDATA_H
#define VERTEXDATA_H

#include <QVector2D>
#include <QVector3D>

/**
  * Reza Adhitya Saputra
  * radhitya@uwaterloo.ca
  */


// VertexData
struct VertexData
{
    QVector3D position; // vertex position
    QVector2D texCoord; // uv coordinate
    QVector3D color;    // color

public:

    VertexData(QVector3D position, QVector2D texCoord, QVector3D color)
    {
        this->position = position;
        this->texCoord = texCoord;
        this->color = color;
    }

    VertexData(QVector3D position, QVector2D texCoord)
    {
        this->position = position;
        this->texCoord = texCoord;
        this->color = QVector3D();
    }

    VertexData()
    {
        this->position = QVector3D();
        this->texCoord = QVector2D();
        this->color = QVector3D();
    }
};

#endif // VERTEXDATA_H

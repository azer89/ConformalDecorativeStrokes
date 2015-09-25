#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QMatrix4x4>
#include <QtOpenGL/QGLWidget>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include "AVector.h"
#include "ALine.h"
#include "StrokePainter.h"

#include <vector>

class GLWidget : public QGLWidget
{
    Q_OBJECT

private:
    StrokePainter* _sPainter;

    bool    _isMouseDown;
    float   _zoomFactor;
    QPoint  _scrollOffset;

    // image size
    int _img_width;
    int _img_height;

    // shader
    QOpenGLShaderProgram* _shaderProgram;

    // for rendering
    int         _mvpMatrixLocation;
    int         _colorLocation;
    int         _vertexLocation;
    int         _use_color_location;
    QMatrix4x4  _perspMatrix;
    QMatrix4x4  _transformMatrix;

private:
    void SaveToSvg();

signals:
    void CalculateConformalMap();

protected:

    bool event( QEvent * event );

    void initializeGL();

    void paintGL();

    void resizeGL(int width, int height);

public:

    GLWidget( QGLFormat format, QWidget *parent = 0);

    ~GLWidget();

    QSize GetCanvasSize() { return QSize(_img_width, _img_height); }

    bool IsMouseDown() { return _isMouseDown; }

    bool  ShouldStop() { return _sPainter->ShouldStop(); }
    float IterationDelta() { return _sPainter->IterationDelta(); }

    /**
     * Stroke Painter
     */
    void SetKiteTexture(QString img) { _sPainter->SetKiteTexture(img); }
    void SetLegTexture(QString img) { _sPainter->SetLegTexture(img); }
    void SetRectilinearTexture(QString img) { _sPainter->SetRectilinearTexture(img); }
    void ConformalMappingOneStep() { _sPainter->ConformalMappingOneStep(); }
    void MappingInterpolation() { _sPainter->MappingInterpolation(); }
    void SelectSlidingConstraints(int x, int y)
    {
        float dx = x + _scrollOffset.x();
        dx /= _zoomFactor;

        float dy = y + _scrollOffset.y();
        dy /= _zoomFactor;

        _sPainter->SelectSlidingConstraints(dx, dy);
    }
    void CalculateVertices()
    {
        //_sPainter->CalculateSpines();
        _sPainter->CalculateInitialRibbon();
        _sPainter->CalculateVertices();
    }


    /**
     * Zoom in handler
     */
    void ZoomIn();

    /**
     * zoom out handler
     */
    void ZoomOut();

    /**
     * Set zoom value
     */
    void SetZoom(int val){this->_zoomFactor = val;}

    /**
     * get zoom value
     */
    float GetZoomFactor() { return this->_zoomFactor; }

    /**
     * Set horizontal scroll position
     */
    void HorizontalScroll(int val);


    /**
     * Set vertical scroll position
     */
    void VerticalScroll(int val);

    /**
     * Get scroll position (horizontal and vertical)
     */
    QPoint GetScrollOffset() {return this->_scrollOffset;}

    /**
     * Mouse press
     */
    void mousePressEvent(int x, int y);

    /**
     * Mouse move
     */
    void mouseMoveEvent(int x, int y);

    /**
     * Mouse release
     */
    void mouseReleaseEvent(int x, int y);

    // keyboard
    void keyboardPressEvent(Qt::Key key);

    void keyboardReleaseEvent(Qt::Key key);

    /**
     * Mouse double click.
     * Since Qt does not support double clicks, I use QTimer to simulate a double click.
     */
    void mouseDoubleClick(int x, int y);
};

#endif // GLWIDGET_H

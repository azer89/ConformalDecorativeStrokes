


#include "GLWidget.h"

#include <iostream>
#include <random>
#include <math.h>

#include <QGLFormat>
#include <QSvgGenerator>

#include "VertexData.h"
#include "SystemParams.h"

GLWidget::GLWidget(QGLFormat format, QWidget *parent) :
    QGLWidget(format, parent),
    _sPainter(0),
    _isMouseDown(false),
    _zoomFactor(10.0),
    _img_width(500),
    _img_height(500),
    _shaderProgram(0)
{
}

GLWidget::~GLWidget()
{
    if(_shaderProgram) delete _shaderProgram;
    if(_sPainter) delete _sPainter;
}

void GLWidget::initializeGL()
{
    QGLFormat glFormat = QGLWidget::format();
    if (!glFormat.sampleBuffers()) { std::cerr << "Could not enable sample buffers." << std::endl; return; }

    glShadeModel(GL_SMOOTH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor( 1.0, 1.0, 1.0, 1.0 );
    glEnable(GL_DEPTH_TEST);

    _shaderProgram = new QOpenGLShaderProgram();
    if (!_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, SystemParams::v_shader_file.c_str()))
        { std::cerr << "Cannot load vertex shader." << std::endl; return; }

    if (!_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, SystemParams::f_shader_file.c_str()))
        { std::cerr << "Cannot load fragment shader." << std::endl; return; }

    if ( !_shaderProgram->link() )
        { std::cerr << "Cannot link shaders." << std::endl; return; }

    _shaderProgram->bind();
    _mvpMatrixLocation = _shaderProgram->uniformLocation("mvpMatrix");
    _colorLocation = _shaderProgram->attributeLocation("vertexColor");
    _vertexLocation = _shaderProgram->attributeLocation("vert");
    _use_color_location = _shaderProgram->uniformLocation("use_color");

    _sPainter = new StrokePainter();
    _sPainter->SetVertexDataHelper(_shaderProgram);

    QString qFilenameA(SystemParams::leg_texture_file.c_str());
    this->SetLegTexture(qFilenameA);

    QString qFilenameB(SystemParams::kite_texture_file.c_str());
    this->SetKiteTexture(qFilenameB);
}

bool GLWidget::event( QEvent * event )
{
    return QGLWidget::event(event);
}


// This is an override function from Qt but I can't find its purpose
void GLWidget::resizeGL(int width, int height)
{
}


void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, this->width(),  this->height());

    int current_width = width();
    int current_height = height();

    // Set orthographic Matrix
    QMatrix4x4 orthoMatrix;

    orthoMatrix.ortho(0.0 +  _scrollOffset.x(),
                      (float)current_width +  _scrollOffset.x(),
                      (float)current_height + _scrollOffset.y(),
                      0.0 + _scrollOffset.y(),
                      -100, 100);

    // Translate the view to the middle
    QMatrix4x4 transformMatrix;
    transformMatrix.setToIdentity();
    transformMatrix.scale(_zoomFactor);

    _shaderProgram->setUniformValue(_mvpMatrixLocation, orthoMatrix * transformMatrix);

    _sPainter->Draw();
}

// Mouse is pressed
void GLWidget::mousePressEvent(int x, int y)
{
    _isMouseDown = true;

    float dx = x + _scrollOffset.x();
    dx /= _zoomFactor;

    float dy = y + _scrollOffset.y();
    dy /= _zoomFactor;

    _sPainter->mousePressEvent(dx, dy);


    this->repaint();
}

// Mouse is moved
void GLWidget::mouseMoveEvent(int x, int y)
{
    float dx = x + _scrollOffset.x();
    dx /= _zoomFactor;

    float dy = y + _scrollOffset.y();
    dy /= _zoomFactor;

    // your stuff

    if(_isMouseDown)
    {
        _sPainter->mouseMoveEvent(dx, dy);
    }

    this->repaint();
}


// Mouse is released
void GLWidget::mouseReleaseEvent(int x, int y)
{
    _isMouseDown = false;
    float dx = x + _scrollOffset.x();
    dx /= _zoomFactor;

    float dy = y + _scrollOffset.y();
    dy /= _zoomFactor;

    _sPainter->mouseReleaseEvent(dx, dy);

    int qmSize = _sPainter->QuadMeshSize();
    if(qmSize > 0)
    {
        emit CalculateConformalMap();
    }

    this->repaint();
}

void GLWidget::keyboardPressEvent(Qt::Key key)
{

}

void GLWidget::keyboardReleaseEvent(Qt::Key key)
{

}

void GLWidget::mouseDoubleClick(int x, int y)
{
    double dx = x + _scrollOffset.x();
    dx /= _zoomFactor;

    double dy = y + _scrollOffset.y();
    dy /= _zoomFactor;

    // your stuff

    this->repaint();
}


void GLWidget::HorizontalScroll(int val) { _scrollOffset.setX(val); }
void GLWidget::VerticalScroll(int val) { _scrollOffset.setY(val); }
void GLWidget::ZoomIn() { this->_zoomFactor += 0.5f; }
void GLWidget::ZoomOut() { this->_zoomFactor -= 0.5f; if(this->_zoomFactor < 0.1f) _zoomFactor = 0.1f; }

void GLWidget::SaveToSvg()
{
}



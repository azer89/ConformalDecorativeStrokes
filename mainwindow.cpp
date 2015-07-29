#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->widget, SIGNAL(CalculateConformalMap()), this, SLOT(AnimationStart()));

    animTimer = new QTimer(this);
    connect(animTimer, SIGNAL(timeout()), this, SLOT(AnimationThread()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete animTimer;
}

void MainWindow::AnimationThread()
{
    this->ui->widget->GetGLWidget()->ConformalMappingOneStep();
    this->ui->widget->GetGLWidget()->repaint();
}

void MainWindow::AnimationStart()
{
    //std::cout << "Animation Start\n";
    animTimer->start(1000);
}

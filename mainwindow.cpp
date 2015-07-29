#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->widget->GetGLWidget(), SIGNAL(CalculateConformalMap()), this, SLOT(AnimationStart()));

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
    if(!ui->widget->GetGLWidget()->IsMouseDown())
    {
        this->ui->widget->GetGLWidget()->ConformalMappingOneStep();
        this->ui->widget->GetGLWidget()->repaint();

        if(this->ui->widget->GetGLWidget()->ShouldStop())
        {
            std::cout << "iteration complete\n";
            animTimer->stop();
        }
    }
}

void MainWindow::AnimationStart()
{
    //std::cout << "Animation Start\n";

    if(animTimer->isActive())
    {
       animTimer->stop();
    }
    animTimer->start();
}

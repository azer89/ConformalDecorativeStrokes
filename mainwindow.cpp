#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

#include "SystemParams.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->widget->GetGLWidget(), SIGNAL(CalculateConformalMap()), this, SLOT(AnimationStart()));
    connect(ui->miterCheckBox,	 SIGNAL(stateChanged(int)), this, SLOT(SetParams()));
    connect(ui->actionOpenImage,	 SIGNAL(triggered()), this, SLOT(FileOpen()));

    animTimer = new QTimer(this);
    connect(animTimer, SIGNAL(timeout()), this, SLOT(AnimationThread()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete animTimer;
}

void MainWindow::FileOpen()
{
    QString qFilename = QFileDialog::getOpenFileName(this, "/home/azer/Desktop/");
    if(qFilename.isEmpty()) return;

    QPixmap image(qFilename);
    ui->inputImageLabel->setPixmap(image);
    ui->widget->GetGLWidget()->SetImage(qFilename);
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

void MainWindow::SetParams()
{
    //std::cout << "SetParams\n";
    //std::cout << ui->miterCheckBox->isChecked() << "\n";
    SystemParams::enforce_miter_joint = ui->miterCheckBox->isChecked();
}

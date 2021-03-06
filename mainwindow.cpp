#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

#include "SystemParams.h"

/**
  * Reza Adhitya Saputra
  * radhitya@uwaterloo.ca
  */


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->widget->GetGLWidget(),   SIGNAL(CalculateConformalMap()), this, SLOT(AnimationStart()));
    connect(ui->widget,   SIGNAL(ResetTime()), this, SLOT(ResetTime()));

    //connect(ui->conformalMappingCheckBox,SIGNAL(stateChanged(int)),       this, SLOT(SetParams()));
    connect(ui->quadSizeSpinBox,         SIGNAL(valueChanged(double)),    this, SLOT(SetParams()));
    connect(ui->iterThresholdSpinBox,    SIGNAL(valueChanged(double)),    this, SLOT(SetParams()));
    //connect(ui->kiteLegsSpinBox,         SIGNAL(valueChanged(double)),    this, SLOT(SetParams()));

    connect(ui->linearWarpingRadioButton,    SIGNAL(clicked(bool)),    this, SLOT(SetParams()));
    connect(ui->conformalWarpingRadioButton, SIGNAL(clicked(bool)),    this, SLOT(SetParams()));

    connect(ui->segmentConstraintCheckBox,            SIGNAL(stateChanged(int)),       this, SLOT(SetParams()));

    connect(ui->meshCheckBox,            SIGNAL(stateChanged(int)),       this, SLOT(SetDisplay()));
    connect(ui->textureCheckBox,         SIGNAL(stateChanged(int)),       this, SLOT(SetDisplay()));


    connect(ui->actionParameters,	         SIGNAL(triggered()), this, SLOT(ShowParamsDockWidget()));
    connect(ui->actionTextures,	             SIGNAL(triggered()), this, SLOT(ShowTexturesDockWidget()));
    connect(ui->actionSetKiteTexture,	     SIGNAL(triggered()), this, SLOT(SetKiteTexture()));
    connect(ui->actionSetLLegTexture,	     SIGNAL(triggered()), this, SLOT(SetLLegTexture()));
    connect(ui->actionSetRLegTexture,	     SIGNAL(triggered()), this, SLOT(SetRLegTexture()));
    connect(ui->actionSetRectilinearTexture, SIGNAL(triggered()), this, SLOT(SetRectilinearTexture()));



    connect(ui->lLegTextureButton,        SIGNAL(clicked()), this, SLOT(SetLLegTexture()));
    connect(ui->rLegTextureButton,        SIGNAL(clicked()), this, SLOT(SetRLegTexture()));
    connect(ui->kiteTextureButton,        SIGNAL(clicked()), this, SLOT(SetKiteTexture()));
    connect(ui->rectilinearTextureButton, SIGNAL(clicked()), this, SLOT(SetRectilinearTexture()));

    animTimer = new QTimer(this);
    connect(animTimer, SIGNAL(timeout()), this, SLOT(AnimationThread()));


    ui->kiteTextureButton->setIcon(QIcon(SystemParams::kite_texture_file.c_str()));
    ui->kiteTextureButton->setIconSize(ui->kiteTextureButton->size());
    ui->kiteTextureButton->setText("");

    ui->lLegTextureButton->setIcon(QIcon(SystemParams::l_leg_texture_file.c_str()));
    ui->lLegTextureButton->setIconSize(ui->lLegTextureButton->size());
    ui->lLegTextureButton->setText("");

    ui->rLegTextureButton->setIcon(QIcon(SystemParams::r_leg_texture_file.c_str()));
    ui->rLegTextureButton->setIconSize(ui->rLegTextureButton->size());
    ui->rLegTextureButton->setText("");

    ui->rectilinearTextureButton->setIcon(QIcon(SystemParams::rectilinear_texture_file.c_str()));
    ui->rectilinearTextureButton->setIconSize(ui->rectilinearTextureButton->size());
    ui->rectilinearTextureButton->setText("");
}

MainWindow::~MainWindow()
{
    delete ui;
    delete animTimer;
}

void MainWindow::ShowParamsDockWidget()
{
    ui->paramsDockWidget->show();
}

void MainWindow::ShowTexturesDockWidget()
{
    ui->texturesDockWidget->show();
}

// Kite Texture
void MainWindow::SetKiteTexture()
{
    bool isBusy = false;
    if(animTimer->isActive())
    {
       animTimer->stop();
       isBusy = true;
    }

    QString qFilename = QFileDialog::getOpenFileName(this,
                                                     "Set Kite Texture",
                                                     SystemParams::texture_dir.c_str());
    if(qFilename.isEmpty()) return;

    ui->kiteTextureButton->setIcon(QIcon(qFilename));
    ui->kiteTextureButton->setIconSize(ui->kiteTextureButton->size());

    ui->widget->GetGLWidget()->SetKiteTexture(qFilename);

    if(isBusy) { animTimer->start(); }
}

// Leg Texture
void MainWindow::SetLLegTexture()
{
    bool isBusy = false;
    if(animTimer->isActive())
    {
       animTimer->stop();
       isBusy = true;
    }

    QString qFilename = QFileDialog::getOpenFileName(this,
                                                     "Set Leg Texture",
                                                     SystemParams::texture_dir.c_str());
    if(qFilename.isEmpty()) return;

    ui->lLegTextureButton->setIcon(QIcon(qFilename));
    ui->lLegTextureButton->setIconSize(ui->lLegTextureButton->size());

    // todo: edit this !!!
    ui->widget->GetGLWidget()->SetLeftLegTexture(qFilename);
    //ui->widget->GetGLWidget()->SetRightLegTexture(qFilename);

    if(isBusy) { animTimer->start(); }
}

// Leg Texture
void MainWindow::SetRLegTexture()
{
    bool isBusy = false;
    if(animTimer->isActive())
    {
       animTimer->stop();
       isBusy = true;
    }

    QString qFilename = QFileDialog::getOpenFileName(this,
                                                     "Set Leg Texture",
                                                     SystemParams::texture_dir.c_str());
    if(qFilename.isEmpty()) return;

    ui->rLegTextureButton->setIcon(QIcon(qFilename));
    ui->rLegTextureButton->setIconSize(ui->rLegTextureButton->size());

    // todo: edit this !!!
    //ui->widget->GetGLWidget()->SetLeftLegTexture(qFilename);
    ui->widget->GetGLWidget()->SetRightLegTexture(qFilename);

    if(isBusy) { animTimer->start(); }
}

// Rectilinear Texture
void MainWindow::SetRectilinearTexture()
{
    bool isBusy = false;
    if(animTimer->isActive())
    {
       animTimer->stop();
       isBusy = true;
    }

    QString qFilename = QFileDialog::getOpenFileName(this,
                                                     "Set Rectilinear Texture",
                                                     SystemParams::texture_dir.c_str());
    if(qFilename.isEmpty()) return;

    ui->rectilinearTextureButton->setIcon(QIcon(qFilename));
    ui->rectilinearTextureButton->setIconSize(ui->rectilinearTextureButton->size());

    ui->widget->GetGLWidget()->SetRectilinearTexture(qFilename);

    if(isBusy) { animTimer->start(); }
}

void MainWindow::ResetTime()
{
    startTime = std::clock();
}

void MainWindow::AnimationThread()
{
    float duration = ( std::clock() - startTime ) / (double) CLOCKS_PER_SEC;
    ui->runningTimeLabel->setText("Time: " + QString::number(duration));
    ui->deltaLabel->setText("Delta: " + QString::number(ui->widget->GetGLWidget()->IterationDelta()));

    if(!ui->widget->GetGLWidget()->IsMouseDown())
    {
        this->ui->widget->GetGLWidget()->ConformalMappingOneStep();
        this->ui->widget->GetGLWidget()->repaint();

        if(this->ui->widget->GetGLWidget()->ShouldStop()/*  || duration > 300.0f */ )
        {
            //this->ui->widget->GetGLWidget()->MappingInterpolation();
            //this->ui->widget->GetGLWidget()->repaint();
            //ui->runningTimeLabel->setText("Complete in: " + QString::number(duration));
            //ui->deltaLabel->setText("Delta: " + QString::number(ui->widget->GetGLWidget()->IterationDelta()));

            animTimer->stop();
        }
    }
}

void MainWindow::AnimationStart()
{
    if(animTimer->isActive())
    {
       animTimer->stop();
    }

    if(SystemParams::enable_conformal_mapping)
    {
        startTime = std::clock();
        animTimer->start();
    }
}

void MainWindow::SetDisplay()
{
    SystemParams::show_mesh = ui->meshCheckBox->isChecked();
    SystemParams::show_texture = ui->textureCheckBox->isChecked();
    this->ui->widget->GetGLWidget()->repaint();
}

void MainWindow::SetParams()
{
    SystemParams::iter_threshold = ui->iterThresholdSpinBox->value();
    SystemParams::enable_conformal_mapping = ui->conformalWarpingRadioButton->isChecked();

    SystemParams::segment_constraint = ui->segmentConstraintCheckBox->isChecked();

    SystemParams::grid_cell_size = ui->quadSizeSpinBox->value();

    ui->runningTimeLabel->setText("Time: 0");
    ui->deltaLabel->setText("Delta: 0");

    this->ui->widget->GetGLWidget()->CalculateVertices();

    if(animTimer->isActive())
    {
       animTimer->stop();
    }

    if(SystemParams::enable_conformal_mapping)
    {
        startTime = std::clock();
        animTimer->start();
    }

    this->ui->widget->GetGLWidget()->repaint();
}

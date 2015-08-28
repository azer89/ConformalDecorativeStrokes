#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

#include "SystemParams.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->widget->GetGLWidget(),   SIGNAL(CalculateConformalMap()), this, SLOT(AnimationStart()));

    connect(ui->fixedSeparationCheckBox, SIGNAL(stateChanged(int)),       this, SLOT(SetParams()));
    connect(ui->conformalMappingCheckBox,SIGNAL(stateChanged(int)),       this, SLOT(SetParams()));
    connect(ui->quadSizeSpinBox,         SIGNAL(valueChanged(double)),    this, SLOT(SetParams()));

    connect(ui->meshCheckBox,            SIGNAL(stateChanged(int)),       this, SLOT(SetDisplay()));
    connect(ui->textureCheckBox,         SIGNAL(stateChanged(int)),       this, SLOT(SetDisplay()));

    connect(ui->actionSetStrokeTexture,	 SIGNAL(triggered()),             this, SLOT(SetStrokeTexture()));
    connect(ui->actionSetCornerTexture,	 SIGNAL(triggered()),             this, SLOT(SetCornerTexture()));
    connect(ui->textureAButton,          SIGNAL(clicked()),               this, SLOT(SetStrokeTexture()));
    connect(ui->textureBButton,          SIGNAL(clicked()),               this, SLOT(SetCornerTexture()));

    animTimer = new QTimer(this);
    connect(animTimer, SIGNAL(timeout()), this, SLOT(AnimationThread()));

    ui->textureAButton->setIcon(QIcon(SystemParams::stroke_texture_file.c_str()));
    ui->textureAButton->setIconSize(ui->textureAButton->size());
    ui->textureAButton->setText("");

    ui->textureBButton->setIcon(QIcon(SystemParams::corner_texture_file.c_str()));
    ui->textureBButton->setIconSize(ui->textureBButton->size());
    ui->textureBButton->setText("");
}

MainWindow::~MainWindow()
{
    delete ui;
    delete animTimer;
}

// Stroke Texture
void MainWindow::SetStrokeTexture()
{
    QString qFilename = QFileDialog::getOpenFileName(this,
                                                     "Set Stroke Texture",
                                                     "/home/azer/workspace/cpp/ConformalDecorativeStrokes/decorative_strokes");
    if(qFilename.isEmpty()) return;

    ui->textureAButton->setIcon(QIcon(qFilename));
    ui->textureAButton->setIconSize(ui->textureAButton->size());

    ui->widget->GetGLWidget()->SetStrokeTexture(qFilename);
}

// Corner Texture
void MainWindow::SetCornerTexture()
{
    QString qFilename = QFileDialog::getOpenFileName(this,
                                                     "Set Corner Texture",
                                                     "/home/azer/workspace/cpp/ConformalDecorativeStrokes/decorative_strokes");
    if(qFilename.isEmpty()) return;

    ui->textureBButton->setIcon(QIcon(qFilename));
    ui->textureBButton->setIconSize(ui->textureBButton->size());


    ui->widget->GetGLWidget()->SetCornerTexture(qFilename);
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

        if(this->ui->widget->GetGLWidget()->ShouldStop()  || duration > 300.0f)
        {
            //this->ui->widget->GetGLWidget()->MappingInterpolation();
            //this->ui->widget->GetGLWidget()->repaint();

            ui->runningTimeLabel->setText("Complete in: " + QString::number(duration));
            ui->deltaLabel->setText("Delta: " + QString::number(ui->widget->GetGLWidget()->IterationDelta()));
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
    SystemParams::fixed_separation_constraint = ui->fixedSeparationCheckBox->isChecked();
    SystemParams::enable_conformal_mapping = ui->conformalMappingCheckBox->isChecked();
    SystemParams::mesh_size = ui->quadSizeSpinBox->value();

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

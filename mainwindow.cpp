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
    connect(ui->miterCheckBox,           SIGNAL(stateChanged(int)),       this, SLOT(SetParams()));
    connect(ui->junctionRibsCheckBox,	 SIGNAL(stateChanged(int)),       this, SLOT(SetParams()));
    connect(ui->spinesCheckBox,          SIGNAL(stateChanged(int)),       this, SLOT(SetParams()));
    connect(ui->meshCheckBox,            SIGNAL(stateChanged(int)),       this, SLOT(SetParams()));
    connect(ui->textureCheckBox,         SIGNAL(stateChanged(int)),       this, SLOT(SetParams()));
    connect(ui->quadSizeSpinBox,         SIGNAL(valueChanged(double)),    this, SLOT(SetParams()));
    connect(ui->actionSetStrokeTexture,	 SIGNAL(triggered()),             this, SLOT(SetStrokeTexture()));
    connect(ui->actionSetCornerTexture,	 SIGNAL(triggered()),             this, SLOT(SetCornerTexture()));

    animTimer = new QTimer(this);
    connect(animTimer, SIGNAL(timeout()), this, SLOT(AnimationThread()));

    QString qFilenameA("/home/azer/workspace/cpp/ConformalDecorativeStrokes/decorative_strokes/stroke_02_rectangle.png");
    QPixmap imageA(qFilenameA);
    imageA = imageA.scaled(ui->textureALabel->size());
    ui->textureALabel->setPixmap(imageA);

    QString qFilenameB("/home/azer/workspace/cpp/ConformalDecorativeStrokes/decorative_strokes/stroke_02_kite.png");
    QPixmap imageB(qFilenameB);
    imageB = imageB.scaled(ui->textureBLabel->size());
    ui->textureBLabel->setPixmap(imageB);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete animTimer;
}

// Stroke Texture
void MainWindow::SetStrokeTexture()
{
    QString qFilename = QFileDialog::getOpenFileName(this, "Set Stroke Texture");
    if(qFilename.isEmpty()) return;

    QPixmap image(qFilename);
    image = image.scaled(ui->textureALabel->size());
    ui->textureALabel->setPixmap(image);
    ui->widget->GetGLWidget()->SetStrokeTexture(qFilename);
}

// Corner Texture
void MainWindow::SetCornerTexture()
{
    QString qFilename = QFileDialog::getOpenFileName(this, "Set Corner Texture");
    if(qFilename.isEmpty()) return;

    QPixmap image(qFilename);
    image = image.scaled(ui->textureBLabel->size());
    ui->textureBLabel->setPixmap(image);
    ui->widget->GetGLWidget()->SetCornerTexture(qFilename);
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
    /*
     * Comment these if you don't want conformal mapping
     */

    if(animTimer->isActive())
    {
       animTimer->stop();
    }
    animTimer->start();

}

void MainWindow::SetParams()
{
    SystemParams::miter_joint_constraint = ui->miterCheckBox->isChecked();
    SystemParams::junction_ribs_constraint = ui->junctionRibsCheckBox->isChecked();
    SystemParams::spines_constraint = ui->spinesCheckBox->isChecked();

    SystemParams::show_mesh = ui->meshCheckBox->isChecked();
    SystemParams::show_texture = ui->textureCheckBox->isChecked();
    SystemParams::mesh_size = ui->quadSizeSpinBox->value();

    this->ui->widget->GetGLWidget()->repaint();
}

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include <ctime>

/**
  * Reza Adhitya Saputra
  * radhitya@uwaterloo.ca
  */


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QTimer* animTimer;

    std::clock_t startTime;

private slots:
    /**
     * Pay attention to this function since this is where I implement
     * an iteration approach of Conformal Mapping
     */
    void AnimationThread();
    void AnimationStart();

    void SetParams();
    void SetDisplay();

    void ShowParamsDockWidget();
    void ShowTexturesDockWidget();

    void SetKiteTexture();
    void SetLLegTexture();
    void SetRLegTexture();
    void SetRectilinearTexture();

    void ResetTime();

};

#endif // MAINWINDOW_H

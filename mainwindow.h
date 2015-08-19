#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

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

private slots:
    void AnimationThread();
    void AnimationStart();

    void SetParams();
    void SetDisplay();
    void SetStrokeTexture();
    void SetCornerTexture();
};

#endif // MAINWINDOW_H

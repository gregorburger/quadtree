#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "quadtree.h"
#include "particle.h"

class GraphicsScene;
class QResizeEvent;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(QuadTree<Particle *, ParticleToVector> *tree, QWidget *parent = 0);
    ~MainWindow();
    
    void resizeEvent(QResizeEvent *);

private slots:
    void on_actionDraw_activated();

private:
    Ui::MainWindow *ui;
    GraphicsScene *scene;
    QuadTree<Particle *, ParticleToVector> *tree;
};

#endif // MAINWINDOW_H

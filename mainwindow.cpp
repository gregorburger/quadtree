#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QGraphicsEllipseItem>
#include "quadtree.h"
#include "graphicsscene.h"


MainWindow::MainWindow(QuadTree<Particle, ParticleToVector>  *tree, QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), tree(tree)
{
    ui->setupUi(this);
    this->addAction(ui->actionDraw);

    scene = new GraphicsScene(tree);
    ui->graphicsView->setScene(scene);
    on_actionDraw_activated();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *) {
    ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void draw(QGraphicsScene *scene, Node<Particle, ParticleToVector>  *n) {
    if (!n) return;
    float cx = n->box.center.x;
    float cy = n->box.center.y;
    float hd = n->box.half_dim;
    scene->addRect(cx-hd, cy-hd, 2*hd, 2*hd);
    if (n->northEast) draw(scene, n->northEast);
    if (n->northWest) draw(scene, n->northWest);
    if (n->southEast) draw(scene, n->southEast);
    if (n->southWest) draw(scene, n->southWest);
}


void MainWindow::on_actionDraw_activated() {
    scene->clear();
    const float pdim = 0.001;
    for (Particle v: tree->get()) {
        scene->addEllipse(v.x - pdim, v.y - pdim, 2*pdim, 2*pdim);
    }
    draw(scene, tree->root);
    ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

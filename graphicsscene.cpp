#include "graphicsscene.h"
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QGraphicsView>
#include "quadtree.h"

GraphicsScene::GraphicsScene(QuadTree<Particle, ParticleToVector>  *tree, QObject *parent) :
    QGraphicsScene(parent), tree(tree) {
}

void GraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseReleaseEvent(event);
    float x = event->scenePos().x();
    float y = event->scenePos().y();
    if (event->modifiers() || Qt::ShiftModifier) {
        queryBox(x, y);
    } else {
        queryVector(x, y);
    }
}

void GraphicsScene::queryVector(float x, float y) {
    Node<Particle, ParticleToVector>  *n = tree->query(Vector(x, y));
    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidthF(0.005);
    x = n->box.center.x;
    y = n->box.center.y;
    float hd = n->box.half_dim;
    this->addRect(x-hd, y-hd, 2*hd, 2*hd, pen);
}

void GraphicsScene::queryBox(float x, float y) {
    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidthF(0.005);
    AABB box(Vector(x, y), 0.05);
    std::vector<Particle> points = tree->query(box);
    const float pdim = 0.001;
    for (const Particle &v: points) {
        addEllipse(v.x - pdim, v.y - pdim, 2*pdim, 2*pdim, pen);
    }
    addRect(x - box.half_dim, y - box.half_dim, 2*box.half_dim, 2*box.half_dim, pen);
}

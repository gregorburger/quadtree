#ifndef GRAPHICSSCENE_H
#define GRAPHICSSCENE_H

#include <QGraphicsScene>
#include "quadtree.h"
#include "particle.h"

class GraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit GraphicsScene(QuadTree<Particle *, ParticleToVector>  *tree, QObject *parent = 0);
    
protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
private:
    void queryVector(float x, float y);
    void queryBox(float x, float y);
    QuadTree<Particle *, ParticleToVector>  *tree;
};

#endif // GRAPHICSSCENE_H

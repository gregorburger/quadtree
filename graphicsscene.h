#ifndef GRAPHICSSCENE_H
#define GRAPHICSSCENE_H

#include <QGraphicsScene>

class QuadTree;

class GraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit GraphicsScene(QuadTree *tree, QObject *parent = 0);
    
protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
private:
    void queryVector(float x, float y);
    void queryBox(float x, float y);
    QuadTree *tree;
};

#endif // GRAPHICSSCENE_H

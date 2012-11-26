#ifndef PARTICLE_H
#define PARTICLE_H

#include "quadtree.h"

class Particle {
public:
    Particle() = default;
    Particle(float x, float y) : x(x), y(y){}
    float x, y;
};

struct ParticleToVector {
    Vector operator()(const Particle &p) const {
        return Vector(p.x, p.y);
    }
};

#endif // PARTICLE_H

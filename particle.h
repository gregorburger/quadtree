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
    inline
    Particle &operator()(Particle *p) const {
        return *p;
    }
};

#endif // PARTICLE_H

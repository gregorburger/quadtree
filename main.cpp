#define NOGUI

#ifndef NOGUI
#include "mainwindow.h"
#include <QApplication>
#endif
#include <iostream>
#include <boost/random.hpp>
#include <algorithm>
#include <omp.h>
#include "quadtree.h"
#include "particle.h"

QuadTree<Particle, ParticleToVector> *random(size_t count) {
    boost::random::mt19937 rng;
    boost::random::uniform_real_distribution<> gen(0, 1.0);
    srandom(time(NULL));

    std::vector<Particle> points(count);

    std::generate_n(points.begin(), count, [&]{
        return Particle(gen(rng), gen(rng));
    });

    float before = omp_get_wtime();
    QuadTree<Particle, ParticleToVector> *tree = new QuadTree<Particle, ParticleToVector>(points, Vector(0.5, 0.5), 1.0, ParticleToVector(), 3);
    float after = omp_get_wtime();
    std::cout << "tree generation of " << count << " points took " << (after - before) << std::endl;
    return tree;
}

#ifdef NOGUI
const int size = 10000000;
#else
const int size = 1000;
#endif
const int query_size = 10000000;


void test_aabb() {
    AABB a(Vector(0, 0), 0.5);
    AABB b(Vector(1, 1), 0.5);
    AABB c(Vector(0, 0), 0.5);
    AABB d(Vector(0, 0), 0.6);
    AABB e(Vector(0, 0), 0.3);

    assert(a.overlaps(b));
    assert(a.overlaps(c));
    assert(a.overlaps(d));
    assert(a.overlaps(e));

    AABB f(Vector(1,1), 0.1);
    assert(!a.overlaps(f));

    assert(a.in(a.center));
}

int main(int argc, char **argv) {
    test_aabb();
    QuadTree<Particle, ParticleToVector> *tree = random(size);
#ifdef NOGUI
    std::cout << "tree done" << std::endl;

    boost::random::mt19937 rng;
    boost::random::normal_distribution<> gen_1(0.5, 0.05);

    std::vector<Vector> points(query_size);

    std::generate_n(points.begin(), query_size, [&]{
        return Vector(gen_1(rng), gen_1(rng));
    });

    float before = omp_get_wtime();

#pragma omp parallel for
    for (int i = 0; i < points.size(); ++i) {
        tree->query(points[i]);
    }

    float after = omp_get_wtime();
    std::cout << "quering of " << query_size << " nodes took " << (after - before) << std::endl;
#else
    QApplication a(argc, argv);
    MainWindow w(tree);
    w.show();
    return a.exec();
#endif
}

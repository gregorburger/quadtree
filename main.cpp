#define NOGUI

#ifndef NOGUI
#include "mainwindow.h"
#include <QApplication>
#endif
#include <iostream>
#include <boost/random.hpp>
#include <algorithm>
#include <chrono>
#include "quadtree.h"
#include "particle.h"

#define TIME_IT(f) { \
    auto before = std::chrono::high_resolution_clock::now(); \
    f; \
    auto after = std::chrono::high_resolution_clock::now(); \
    auto took = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1> > >(after - before).count(); \
    std::cout << " took " << took << " seconds" << std::endl; \
    }


QuadTree<Particle *, ParticleToVector, 10> *random(std::vector<Particle *> &points) {
    boost::random::mt19937 rng;
    boost::random::uniform_real_distribution<> gen(0, 1.0);
    srandom(time(NULL));

    std::generate(points.begin(), points.end(), [&]{
        return new Particle(gen(rng), gen(rng));
    });


    QuadTree<Particle *, ParticleToVector, 10> *tree = new QuadTree<Particle *, ParticleToVector, 10>(Vector(0.5, 0.5), 1.0, ParticleToVector());
    std::cout << "tree generation of " << points.size();
    TIME_IT(tree->insert(points));
    return tree;
}

void change_a_little_bit(std::vector<Particle *> &points) {
    boost::random::mt19937 rng;
    boost::random::uniform_real_distribution<> gen(-0.0001, 0.0001);
    boost::random::uniform_int_distribution<> gen_idx(0, points.size());

#pragma omp parallel for private(gen, rng)
    for (size_t i = 0; i < points.size()/10; ++i) {
        Particle *p = points[gen_idx(rng)];
        p->x += gen(rng);
        p->y += gen(rng);
    }
}

#ifdef NOGUI
const int size = 1000000;
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

void bench_query(QuadTree<Particle *, ParticleToVector, 10> *tree, std::vector<Particle *> &points) {
    std::cout << "quering of " << query_size << " nodes";
    std::vector<int> idx(points.size());
    std::generate(idx.begin(), idx.end(), []{static int n = 0; return n++;});
    std::random_shuffle(idx.begin(), idx.end());
#pragma omp parallel for
    for (std::size_t i = 0; i < points.size(); ++i) {
        tree->query(Vector(points[idx[i]]->x, points[idx[i]]->y));
    }
}

int main(int argc, char **argv) {
    //test_aabb();
    std::vector<Particle *> points(size);
    QuadTree<Particle *, ParticleToVector, 10> *tree = random(points);
#ifdef NOGUI
    std::cout << "tree done" << std::endl;

    TIME_IT(bench_query(tree, points))
    change_a_little_bit(points);

    std::vector<Particle *> outs;
    std::cout << "checking";

    TIME_IT(tree->check(outs))

    std::cout << outs.size() << " are out." << std::endl;

    tree->insert(outs);

#else
    QApplication a(argc, argv);
    MainWindow w(tree);
    w.show();
    return a.exec();
#endif
}

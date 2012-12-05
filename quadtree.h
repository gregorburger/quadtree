#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include <assert.h>
#include <array>
#include <algorithm>
#include <iostream>

struct Vector {
    Vector() = default;
    Vector(float x, float y) : x(x), y(y) {}
    float x, y;

    template<class T>
    inline
    bool isNW(const T &other) const {
        return other.x <= x && other.y <= y;
    }

    template<class T>
    inline
    bool isNE(const T &other) const {
        return other.x > x && other.y <= y;
    }

    template<class T>
    inline
    bool isSW(const T &other) const {
        return other.x <= x && other.y > y;
    }

    template<class T>
    inline
    bool isSE(const T &other) const {
        return other.x > x && other.y > y;
    }

    inline
    bool operator==(const Vector &other) const {
        return x == other.x && y == other.y;
    }

    inline
    Vector operator-(const float v) const {
        return Vector(x-v, y-v);
    }

    inline
    Vector operator+(const float v) const {
        return Vector(x+v, y+v);
    }

    inline
    bool operator<=(const Vector &other) const {
        return (x <= other.x && y <= other.y);
    }

    inline
    bool operator>=(const Vector &other) const {
        return (x >= other.x && y >= other.y);
    }
};

struct VectorToVector {
    Vector operator()(const Vector &v) const {
        return Vector(v);
    }
};

struct AABB {
    AABB(Vector c, float half_dim)
        : center(c), half_dim(half_dim) {
    }

    template<class T>
    inline
    bool in(const T &v) const {
        if (v.x < l()) return false;
        if (v.x > r()) return false;
        if (v.y < t()) return false;
        if (v.y > b()) return false;
        return true;
    }

    inline
    bool overlaps(const AABB &other) const {
        if (l() > other.r()) return false;
        if (r() < other.l()) return false;
        if (t() > other.b()) return false;
        if (b() < other.t()) return false;
        return true;
    }

    inline
    float l() const {
        return center.x - half_dim;
    }

    inline
    float r() const {
        return center.x + half_dim;
    }

    inline
    float t() const {
        return center.y - half_dim;
    }

    inline
    float b() const {
        return center.y + half_dim;
    }


    inline
    Vector ul() const {
        return Vector(center.x - half_dim, center.y - half_dim);
    }

    inline
    Vector ur() const {
        return Vector(center.x + half_dim, center.y - half_dim);
    }

    inline
    Vector ll() const {
        return Vector(center.x - half_dim, center.y + half_dim);
    }

    inline
    Vector lr() const {
        return Vector(center.x + half_dim, center.y + half_dim);
    }

    inline
    std::array<Vector, 4> corners() const {
        return std::array<Vector, 4> {{ul(), ur(), ll(), lr()}};
    }


    Vector center;
    float half_dim;
};

template<class T, class Function, std::size_t threshold = 3>
struct Node {
    Node(Vector c, float dim, Function f)
        : northWest(0), northEast(0), southWest(0), southEast(0),
          box(c, dim/2.0f), f(f), num_elems(0) {
    }

    void insert(std::vector<T> &elems) {
        if (elems.size() + num_elems <= threshold) {
            std::copy(elems.begin(), elems.end(), this->elems.begin() + num_elems);
            num_elems += elems.size();
            return;
        }

        elems.insert(elems.end(), this->elems.begin(), this->elems.begin() + num_elems);
        num_elems = 0;

        int pre_allocate = elems.size()/2;

        std::vector<T> nw, ne, se, sw;
        nw.reserve(pre_allocate);
        ne.reserve(pre_allocate);
        se.reserve(pre_allocate);
        sw.reserve(pre_allocate);
        float quad_dim = box.half_dim / 2.0f;

        std::cout << "inserting " << elems.size() << std::endl;

        for (T &v: elems) {
            auto _v = f(v);
            if (box.center.isNE(_v)) ne.push_back(v);
            if (box.center.isNW(_v)) nw.push_back(v);
            if (box.center.isSE(_v)) se.push_back(v);
            if (box.center.isSW(_v)) sw.push_back(v);
        }

#pragma omp parallel sections
{

#pragma omp section
        if (!nw.empty()) {
            northWest = new Node<T, Function, threshold>(Vector(box.center.x-quad_dim, box.center.y-quad_dim), box.half_dim, f);
            northWest->insert(nw);
        }

#pragma omp section
        if (!ne.empty()) {
            northEast = new Node<T, Function, threshold>(Vector(box.center.x+quad_dim, box.center.y-quad_dim), box.half_dim, f);
            northEast->insert(ne);
        }

#pragma omp section
        if (!sw.empty()) {
            southWest = new Node<T, Function, threshold>(Vector(box.center.x-quad_dim, box.center.y+quad_dim), box.half_dim, f);
            southWest->insert(sw);
        }

#pragma omp section
        if (!se.empty()) {
            southEast = new Node<T, Function, threshold>(Vector(box.center.x+quad_dim, box.center.y+quad_dim), box.half_dim, f);
            southEast->insert(se);
        }
}
        assert(elems.size() == (nw.size() + ne.size() + sw.size() + se.size()));
    }

    std::size_t count() const {
        if (!northEast && !northWest && !southEast && !southWest) {
            for (Vector v: elems) {
                assert(v <= box.center+box.half_dim);
                assert(v >= box.center-box.half_dim);
            }
            return elems.size();
        }
        std::size_t count = 0;
        if (northEast)
            count += northEast->count();
        if (northWest)
            count += northWest->count();
        if (southEast)
            count += southEast->count();
        if (southWest)
            count += southWest->count();
        return count;
    }

    std::vector<T> query(const AABB &box) const {
        std::vector<T> points;
        if (!northEast && !northWest && !southEast && !southWest) {
            for (size_t i = 0; i < num_elems; ++i) {
                T v = elems[i];
                if (box.in(f(v))) {
                    points.push_back(v);
                }
            }
            return points;
        }
        if (northEast && northEast->box.overlaps(box)) {
            std::vector<T> tmp = northEast->query(box);
            points.insert(points.end(), tmp.begin(), tmp.end());
        }
        if (northWest && northWest->box.overlaps(box)) {
            std::vector<T> tmp = northWest->query(box);
            points.insert(points.end(), tmp.begin(), tmp.end());
        }
        if (southEast && southEast->box.overlaps(box)) {
            std::vector<T> tmp = southEast->query(box);
            points.insert(points.end(), tmp.begin(), tmp.end());
        }
        if (southWest && southWest->box.overlaps(box)) {
            std::vector<T> tmp = southWest->query(box);
            points.insert(points.end(), tmp.begin(), tmp.end());
        }
        return points;
    }

    Node *query(Vector c) {
        if (box.center.isNE(c)) {
            if (northEast) return northEast->query(c);
            return this;
        }

        if (box.center.isNW(c)) {
            if (northWest) return northWest->query(c);
            return this;
        }

        if (box.center.isSE(c)) {
            if (southEast) return southEast->query(c);
            return this;
        }

        if (box.center.isSW(c)) {
            if (southWest) return southWest->query(c);
            return this;
        }
        return 0;
    }

    void get(std::vector<T> &store) const {
        if (!northEast && !northWest && !southEast && !southWest) {
            store.insert(store.end(), elems.begin(), elems.begin()+num_elems);
            return;
        }
        if (northEast)
            northEast->get(store);
        if (northWest)
            northWest->get(store);
        if (southEast)
            southEast->get(store);
        if (southWest)
            southWest->get(store);
    }

    bool check(std::vector<T> &outs) {
        if (!northEast && !northWest && !southEast && !southWest) {
            auto end = std::remove_if(elems.begin(), elems.begin()+num_elems, [&](T v){
                return !box.in(f(v));
            });

            outs.insert(outs.end(), end, elems.begin()+num_elems);
            num_elems = end - elems.begin();

            return num_elems == 0;
        }
        if (northEast) {
            if (northEast->check(outs)) {
                delete northEast;
                northEast = 0;
                //std::cout << "deleting ne" << std::endl;
            }
        }
        if (northWest) {
            if (northWest->check(outs)) {
                delete northWest;
                northWest = 0;
                //std::cout << "deleting nw" << std::endl;
            }
        }
        if (southEast) {
            if (southEast->check(outs)) {
                delete southEast;
                southEast = 0;
                //std::cout << "deleting se" << std::endl;
            }
        }
        if (southWest) {
            if (southWest->check(outs)) {
                delete southWest;
                southWest = 0;
                //std::cout << "deleting sw" << std::endl;
            }
        }
        return !northEast && !northWest && !southEast && !southWest;
    }

    Node *northWest;
    Node *northEast;
    Node *southWest;
    Node *southEast;
    AABB box;
    Function f;
    std::array<T, threshold> elems;
    size_t num_elems;
};

template<class T, class Function, std::size_t threshold = 3>
class QuadTree {
public:
    QuadTree(Vector c, float dim, Function f) {
        root = new Node<T, Function, threshold>(c, dim, f);
    }

    void insert(std::vector<T> &elems) {
        root->insert(elems);
    }

    std::vector<T> query(const AABB &box) const {
        return root->query(box);
    }

    std::vector<T> query(Vector c, float radius) const {
        return std::vector<Vector>();
    }

    Node<T, Function, threshold> *query(Vector c) {
        return root->query(c);
    }

    Node<T, Function, threshold> *getRoot() const {
        return root;
    }

    std::size_t count() const {
        return root->count();
    }

    std::vector<T> get() const {
        std::vector<T> store;
        root->get(store);
        return store;
    }

    void check(std::vector<T> &outs) {
        root->check(outs);
    }

    Node<T, Function, threshold> *root;
};

#endif // QUADTREE_H

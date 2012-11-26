#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include <assert.h>
#include <array>

struct Vector {
    Vector() = default;
    Vector(float x, float y) : x(x), y(y) {}
    float x, y;

    inline
    bool isNW(const Vector &other) const {
        return other.x <= x && other.y <= y;
    }

    inline
    bool isNE(const Vector &other) const {
        return other.x > x && other.y <= y;
    }

    inline
    bool isSW(const Vector &other) const {
        return other.x <= x && other.y > y;
    }

    inline
    bool isSE(const Vector &other) const {
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

    inline
    bool in(const Vector &v) const {
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

template<class T, class Function>
struct Node {
    Node(std::vector<T> elems, Vector c, float dim, Function f, int threshold = 3)
        : northWest(0), northEast(0), southWest(0), southEast(0),
          box(c, dim/2.0f), f(f) {
        if (elems.size() <= (std::size_t) threshold) {
            this->elems = elems;
            return;
        }
        std::vector<T> nw, ne, se, sw;
        float quad_dim = box.half_dim / 2.0f;


        for (const T &v: elems) {
            if (c.isNE(f(v))) ne.push_back(v);
            if (c.isNW(f(v))) nw.push_back(v);
            if (c.isSE(f(v))) se.push_back(v);
            if (c.isSW(f(v))) sw.push_back(v);
        }

#pragma omp parallel sections
{

#pragma omp section
{
        if (!nw.empty())
            northWest = new Node<T, Function>(nw, Vector(c.x-quad_dim, c.y-quad_dim), box.half_dim, f, threshold);
}
#pragma omp section
{
        if (!ne.empty())
            northEast = new Node<T, Function>(ne, Vector(c.x+quad_dim, c.y-quad_dim), box.half_dim, f, threshold);
}
#pragma omp section
{
        if (!sw.empty())
            southWest = new Node<T, Function>(sw, Vector(c.x-quad_dim, c.y+quad_dim), box.half_dim, f, threshold);
}
#pragma omp section
{
        if (!se.empty())
            southEast = new Node<T, Function>(se, Vector(c.x+quad_dim, c.y+quad_dim), box.half_dim, f, threshold);
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
            for (T v: this->elems) {
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
            store.insert(store.end(), elems.begin(), elems.end());
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

    Node *northWest;
    Node *northEast;
    Node *southWest;
    Node *southEast;
    Function f;
    std::vector<T> elems;
    AABB box;
};

template<class T, class Function>
class QuadTree {
public:
    QuadTree(std::vector<T> elems, Vector c, float dim, Function f, int threshold = 3) {
        root = new Node<T, Function>(elems, c, dim, f, threshold);
    }

    std::vector<T> query(const AABB &box) const {
        return root->query(box);
    }

    std::vector<T> query(Vector c, float radius) const {
        return std::vector<Vector>();
    }

    Node<T, Function> *query(Vector c) {
        return root->query(c);
    }

    Node<T, Function> *getRoot() const {
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

    Node<T, Function> *root;
};

#endif // QUADTREE_H

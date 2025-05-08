#pragma once
#include <iostream>
#include <vector>
#include <map>

using FP = float;

namespace M2PGeo {
    /*
     * While various project file formats allow for much longer names,
     * texture names are ultimately limited by the 15 character limit of WAD.
     */
    constexpr int c_MAX_TEXTURE_NAME = 16;
    constexpr FP c_EPSILON = 1.0f / 1024.0f;
    constexpr int c_MAP_DIGITS_PRECISION = 6;

    struct Vector2
    {
        FP x, y;
    public:
        FP magnitude() const;
        FP dot(const Vector2 &other) const;
        /**
         * Pseudo-cross product
         * @return >0 if other is to the left, <0 if it's on the right
         */
        FP cross(const Vector2 &other) const;
        Vector2 normalised() const;
        bool operator==(const Vector2& other) const;
        bool operator!=(const Vector2& other) const;

        static Vector2 zero();
    };

    class Vector3
    {
    public:
        FP x, y, z;
        FP magnitude() const;
        FP dot(const Vector3& other) const;
        Vector3 cross(const Vector3& other) const;
        Vector3 normalised() const;
        Vector3() : x(0), y(0), z(0) {}
        Vector3(FP _x, FP _y, FP _z) : x(_x), y(_y), z(_z) {}
        Vector3 operator+() const;
        Vector3 operator+(const Vector3& other) const;
        Vector3& operator+=(const Vector3& other);
        Vector3 operator-() const;
        Vector3 operator-(const Vector3& other) const;
        Vector3& operator-=(const Vector3& other);
        Vector3 operator*(const Vector3& other) const;
        Vector3 operator*(const FP other) const;
        Vector3 operator/(const Vector3& other) const;
        Vector3 operator/(const FP other) const;
        Vector3 operator/(const int other) const;
        bool operator==(const Vector3& other) const;
        bool operator!=(const Vector3& other) const;

        friend Vector3 operator*(const FP& lhs, const Vector3& rhs);
        friend std::ostream& operator<<(std::ostream& os, const M2PGeo::Vector3& v);

        static Vector3 zero();
    };

    class Vertex : public Vector3
    {
    public:
        Vector2 uv = Vector2::zero();
        Vector3 normal = Vector3::zero();
        bool flipped = false;
        Vertex() : Vector3() {};
        Vertex(FP _x, FP _y, FP _z) : Vector3(_x, _y, _z) {}
        Vertex(const Vector3& point) { x = point.x; y = point.y; z = point.z; }
        bool operator==(const Vertex &other) const;
        bool operator!=(const Vertex &other) const;

        Vector3 coord() const { return Vector3(x, y, z); };
    };

    struct Triangle
    {
        bool flipped;
        M2PGeo::Vector3 normal;
        std::tuple<Vertex, Vertex, Vertex> vertices;
        std::string textureName;
    };

    struct Texture
    {
        std::string name;
        FP shiftx{}, shifty{}, angle{}, scalex{}, scaley{};
        int width{}, height{};
        Vector3 rightaxis{}, downaxis{};
    public:
        Vector2 uvForPoint(const Vector3& point) const;
    };

    enum PointRelation
    {
        BEHIND = -1,
        ON_PLANE = 0,
        INFRONT = 1
    };

    class HessianPlane
    {
    protected:
        Vector3 m_normal;
        FP m_distance;
    public:
        HessianPlane(const Vector3 normal, FP distance);
        HessianPlane(const Vector3 planePoints[3]);
        HessianPlane() : HessianPlane({}, 0.0f) {};
        Vector3 normal() const;
        FP distance() const;
        FP distanceToPoint(Vector3 point) const;
        PointRelation pointRelation(const Vector3& point) const;
    };

    class Plane : public HessianPlane
    {
        Texture m_texture;
        Vector3 m_planePoints[3];
    public:
        Plane(const Vector3 planePoints[3], const Texture& texture);
        Texture texture() const;
    };

    Vector3 segmentsCross(const Vector3& a, const Vector3& b, const Vector3& c);
    Vector3 segmentsCross(const Vector3 planePoints[3]);

    Vector3 planeNormal(const Vector3 planePoints[3]);

    Vector3 sumVectors(const std::vector<Vector3> &vectors);
    Vector3 sumVertices(const std::vector<Vertex>& vertices);

    Vector3 geometricCenter(const std::vector<Vector3> &vectors);
    Vector3 geometricCenter(const std::vector<Vertex> &vertices);

    void sortVectors(std::vector<Vector3> &vectors, const Vector3 &normal);
    void sortVertices(std::vector<Vertex> &vertices, const Vector3 &normal);

}

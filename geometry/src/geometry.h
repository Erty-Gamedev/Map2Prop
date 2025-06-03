#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <format>
#include <unordered_map>
#include <numbers>

using FP = float;

namespace M2PGeo {
    /*
     * While various project file formats allow for much longer names,
     * texture names are ultimately limited by the 15 character limit of WAD.
     */
    constexpr int c_MAX_TEXTURE_NAME = 16;
    constexpr int c_MAP_DIGITS_PRECISION = 6;
    constexpr FP c_EPSILON = 1. / 1024.;
    constexpr FP c_DEG2RAD = static_cast<FP>(std::numbers::pi / 180.);
    constexpr FP c_RAD2DEG = static_cast<FP>(180. / std::numbers::pi);

    extern bool g_isEager;


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
        Vector3(const FP xyz[3]) : x(xyz[0]), y(xyz[1]), z(xyz[2]) {}
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
        Vector3 operator/(const size_t other) const;
        bool operator==(const Vector3& other) const;
        bool operator!=(const Vector3& other) const;

        friend Vector3 operator*(const FP& lhs, const Vector3& rhs);
        friend std::ostream& operator<<(std::ostream& os, const M2PGeo::Vector3& v);

        static Vector3 zero();
    };

    struct Bounds
    {
        Vector3 min, max;
        static Bounds zero() { return { Vector3::zero(), Vector3::zero() }; }
        bool operator==(const Bounds& other) const { return min == other.min && max == other.max; };

        bool pointInside(Vector3 p) const;
    };

    class Vertex : public Vector3
    {
    public:
        Vector2 uv = Vector2::zero();
        Vector3 normal = Vector3::zero();
        Vertex() : Vector3() {};
        Vertex(FP _x, FP _y, FP _z) : Vector3(_x, _y, _z) {}
        Vertex(const float xyz[3]) : Vector3(xyz) {}
        Vertex(const Vector3& point) { x = point.x; y = point.y; z = point.z; }
        Vertex(const float xyz[3], const float _uv[2], const Vector3 _normal)
        {
            x = xyz[0]; y = xyz[1]; z = xyz[2];
            uv.x = _uv[0]; uv.y = _uv[1];
            normal = _normal;
        }
        bool operator==(const Vertex &other) const;
        bool operator!=(const Vertex &other) const;

        friend bool operator==(const Vector3& lhs, const Vertex& rhs);
        friend bool operator==(const Vertex& lhs, const Vector3& rhs);

        Vector3 coord() const { return Vector3(x, y, z); };
    };

    struct Triangle
    {
        bool flipped;
        M2PGeo::Vector3 normal;
        Vertex vertices[3] = { Vertex::zero(), Vertex::zero(), Vertex::zero() };
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

    FP deg2rad(FP degrees);
    FP rad2deg(FP rad);

    FP clip(FP value, FP minimum, FP maximum);
    FP vectorsAngle(const Vector3& a, const Vector3& b);

    Vector3 segmentsCross(const Vector3& a, const Vector3& b, const Vector3& c);
    Vector3 segmentsCross(const Vector3 planePoints[3]);

    Vector3 planeNormal(const Vector3 planePoints[3]);

    Vector3 sumVectors(const std::vector<Vector3> &vectors);
    Vector3 sumVertices(const std::vector<Vertex>& vertices);

    Vector3 geometricCenter(const std::vector<Vector3> &vectors);
    Vector3 geometricCenter(const Bounds& vectors);
    Vector3 geometricCenter(const std::vector<Vertex> &vertices);

    void sortVectors(std::vector<Vector3> &vectors, const Vector3 &normal);
    void sortVertices(std::vector<Vertex> &vertices, const Vector3 &normal);

    using GroupedVertices = std::unordered_map<std::string, std::vector<std::reference_wrapper<Vertex>>>;
    void averageNormals(GroupedVertices& groupedVertices);
    void averageNearNormals(GroupedVertices& groupedVertices, FP thresholdDegrees);
}

template <>
struct std::formatter<M2PGeo::Vector3> : std::formatter<std::string>
{
    auto format(M2PGeo::Vector3 v, format_context& ctx) const
    {
        return formatter<string>::format(std::format("{:.6g} {:.6g} {:.6g}", v.x, v.y, v.z), ctx);
    }
};

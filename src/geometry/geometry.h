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
    static inline constexpr int c_MAX_TEXTURE_NAME = 16;
    static inline constexpr int c_MAP_DIGITS_PRECISION = 6;
    static inline constexpr FP c_EPSILON = static_cast<FP>(1. / 1024.);
    static inline constexpr FP c_EPSILON_MERGE = static_cast<FP>(1. / 1024.);
    // According to LogicAndTrick, this epsilon matches Valve Hammer Editor (https://github.com/LogicAndTrick/sledge-formats/blob/d97d978d0eebbba3cfa87166eafd69d9db59abec/Sledge.Formats.Map/Objects/Solid.cs#L26)
    static inline constexpr float c_EPSILON_ONPLANE = 0.0075f;
    static inline constexpr FP c_DEG2RAD = static_cast<FP>(std::numbers::pi / 180.);
    static inline constexpr FP c_RAD2DEG = static_cast<FP>(180. / std::numbers::pi);

    extern bool g_isEager;


    struct Vector2
    {
        union { struct { FP x, y; };  FP v[2]; };

        Vector2() = default;
        Vector2(FP _x, FP _y) : x(_x), y(_y) {};

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

        static Vector2 zero() { return Vector2{ 0.0f, 0.0f }; }
    };

    class Vector3
    {
    public:
        union { struct { FP x, y, z; };  FP v[3]; };

        Vector3() : x(0.), y(0.), z(0.) {}
        Vector3(FP _x, FP _y, FP _z) : x(_x), y(_y), z(_z) {}
        Vector3(const float xyz[3]) : x(static_cast<FP>(xyz[0])), y(static_cast<FP>(xyz[1])), z(static_cast<FP>(xyz[2])) {}
        Vector3(const double xyz[3]) : x(static_cast<FP>(xyz[0])), y(static_cast<FP>(xyz[1])), z(static_cast<FP>(xyz[2])) {}

        FP magnitude() const;
        FP dot(const Vector3& other) const;
        Vector3 cross(const Vector3& other) const;
        Vector3 normalised() const;
        FP distance(const Vector3& other) const;
        FP angle(const Vector3& other) const;
        Vector3 operator+() const;
        Vector3 operator+(const Vector3& other) const;
        Vector3& operator+=(const Vector3& other);
        Vector3 operator-() const;
        Vector3 operator-(const Vector3& other) const;
        Vector3& operator-=(const Vector3& other);
        Vector3 operator*(const Vector3& other) const;
        Vector3 operator*(const FP other) const;
        Vector3& operator*=(const FP other);
        Vector3 operator/(const Vector3& other) const;
        Vector3 operator/(const FP other) const;
        Vector3 operator/(const int other) const;
        Vector3 operator/(const size_t other) const;
        bool operator==(const Vector3& other) const;
        bool operator!=(const Vector3& other) const;

        static Vector3 zero();
    };
    Vector3 operator*(const FP& lhs, const Vector3& rhs);
    std::ostream& operator<<(std::ostream& os, const M2PGeo::Vector3& v);

    struct Bounds
    {
        Vector3 min, max;
        static Bounds zero() { return { Vector3::zero(), Vector3::zero() }; }
        bool operator==(const Bounds& other) const { return min == other.min && max == other.max; };

        bool pointInside(Vector3 p) const;
    };

    bool pointInBounds(Vector3 point, const std::vector<Bounds>& bounds);


    class Vertex : public Vector3
    {
    public:
        Vector2 uv = Vector2::zero();
        Vector3 normal = Vector3::zero();
        Vertex() : Vector3() {};
        Vertex(FP _x, FP _y, FP _z) : Vector3(_x, _y, _z) {}
        Vertex(const FP xyz[3]) : Vector3(xyz) {}
        Vertex(const Vector3& point) { x = point.x; y = point.y; z = point.z; }
        Vertex(const Vector3& point, const Vector3& _normal)
        {
            x = point.x; y = point.y; z = point.z;
            normal = _normal;
        }
        Vertex(const float xyz[3], const float _uv[2], const Vector3 _normal)
        {
            x = xyz[0]; y = xyz[1]; z = xyz[2];
            uv.x = _uv[0]; uv.y = _uv[1];
            normal = _normal;
        }
        bool operator==(const Vertex &other) const;
        bool operator!=(const Vertex &other) const;

        Vector3 coord() const { return Vector3(x, y, z); };
    };
    bool operator==(const Vector3& lhs, const Vertex& rhs);
    bool operator==(const Vertex& lhs, const Vector3& rhs);

    struct Triangle
    {
        bool flipped;
        M2PGeo::Vector3 normal;
        Vertex vertices[3] = { Vertex::zero(), Vertex::zero(), Vertex::zero() };
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

    using GroupedVertices = std::unordered_map<Vector3, std::vector<std::reference_wrapper<Vertex>>>;
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

namespace std
{
    template<> struct hash<M2PGeo::Vector3>
    {
        std::size_t operator()(M2PGeo::Vector3 const& v) const noexcept
        {
            return std::hash<std::string>{}(std::format("{:.2g} {:.2g} {:.2g}", v.x, v.y, v.z));
        }
    };
}

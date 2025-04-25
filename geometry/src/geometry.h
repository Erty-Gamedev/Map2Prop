#pragma once
#include <iostream>
#include <vector>
#include <map>

namespace M2PGeo {
    /*
    * While various project file formats allow for much longer names,
    * texture names are ultimately limited by the 15 character limit of WAD.
    */ 
    constexpr int c_MAX_TEXTURE_NAME = 16;
    constexpr float c_EPSILON = 1.0f / 1024.0f;
    constexpr int c_MAP_DIGITS_PRECISION = 6;

    struct Vector2
    {
        float x, y;
    public:
        float magnitude() const;
        float dot(const Vector2& other) const;
        Vector2 normalised() const;
        static Vector2 zero();
    };

    struct Vector3
    {
        float x, y, z;
    public:
        float magnitude() const;
        float dot(const Vector3& other) const;
        Vector3 cross(const Vector3& other) const;
        Vector3 normalised() const;
        Vector3 operator+() const;
        Vector3 operator+(const Vector3& other) const;
        Vector3 operator-() const;
        Vector3 operator-(const Vector3& other) const;
        Vector3 operator*(const Vector3& other) const;
        Vector3 operator*(const float other) const;
        Vector3 operator/(const Vector3& other) const;
        Vector3 operator/(const float other) const;
        bool operator==(const Vector3& other) const;
        bool operator!=(const Vector3& other) const;

        friend Vector3 operator*(const float& lhs, const Vector3& rhs);
        friend std::ostream& operator<<(std::ostream& os, const M2PGeo::Vector3& v);

        static Vector3 zero();
    };

    struct Vertex
    {
        Vector3 coord, normal;
        Vector2 uv;
        bool flipped;
    };

    struct Polygon
    {
        std::string texture_name;
        bool flipped;
        std::vector<Vertex> vertices;
    public:
        Vector3 normal() const;
    };

    struct Texture
    {
        std::string name;
        float shiftx{}, shifty{}, angle{}, scalex{}, scaley{}, width{}, height{};
        Vector3 rightaxis{}, downaxis{};
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
        float m_distance;
    public:
        HessianPlane(Vector3 normal, float distance);
        HessianPlane() : HessianPlane({}, 0.0f) {};
        Vector3 normal() const;
        float distance() const;
        float distanceToPoint(Vector3 point) const;
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

    Vector3 segmentsCross(const Vector3 planePoints[3]);

}

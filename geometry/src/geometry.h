#pragma once
#include <iostream>
#include <vector>

namespace M2PGeo {
    /*
    * While various project file formats allow for much longer names,
    * texture names are ultimately limited by the 15 character limit of WAD.
    */ 
    constexpr int c_MAX_TEXTURE_NAME = 16;
    constexpr float c_EPSILON = 1 / 1024;


    struct Vector2
    {
        float x, y;
    public:
        float magnitude() const;
        float dot(const Vector2&) const;
        Vector2 normalised() const;
        static Vector2 zero();
    };

    struct Vector3
    {
        float x, y, z;
    public:
        float magnitude() const;
        float dot(const Vector3&) const;
        Vector3 cross(const Vector3&) const;
        Vector3 normalised() const;
        Vector3 operator+() const;
        Vector3 operator+(const Vector3&) const;
        Vector3 operator-() const;
        Vector3 operator-(const Vector3&) const;
        Vector3 operator*(const Vector3&) const;
        Vector3 operator*(const float) const;
        Vector3 operator/(const Vector3&) const;
        Vector3 operator/(const float) const;
        bool operator==(const Vector3&) const;
        bool operator!=(const Vector3&) const;

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
        char texture_name[c_MAX_TEXTURE_NAME];
        bool flipped;
        std::vector<Vertex> vertices;
    public:
        Vector3 normal() const;
    };

    struct Texture
    {
        char name[c_MAX_TEXTURE_NAME];
        float shiftx, shifty, angle, scalex, scaley, width, height;
        Vector3 rightaxis, downaxis;
    };

    enum PointRelation
    {
        BEHIND = -1,
        ON_PLANE = 0,
        INFRONT = 1
    };

    class HessianPlane
    {
        Vector3 normal;
        float distance;
    public:
        HessianPlane(Vector3, float);
        HessianPlane() : HessianPlane({}, 0.0f) {};
        float distanceToPoint(Vector3) const;
        PointRelation pointRelation(const Vector3&) const;
    };

    class Plane : HessianPlane
    {
    public:
        Plane(const Vector3[3], const Texture&);
    };

    Vector3 segmentsCross(const Vector3[3]);

}

std::ostream& operator<<(std::ostream&, const M2PGeo::Vector3&);



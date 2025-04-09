#pragma once
#include <iostream>
#include <vector>

namespace M2PGeo {
    /*
    * While various project file formats allow for much longer names,
    * texture names are ultimately limited by the 15 character limit of WAD.
    */ 
    constexpr int MAX_TEXTURE_NAME = 16;
    constexpr float EPSILON = 1 / 1024;


    struct Vector2D
    {
        float x, y;
    public:
        float magnitude() const;
        float dot(const Vector2D& other) const;
        Vector2D normalised() const;
        static Vector2D zero();
    };

    struct Vector3D
    {
        float x, y, z;
    public:
        float magnitude() const;
        float dot(const Vector3D& other) const;
        Vector3D cross(const Vector3D& other);
        Vector3D normalised() const;
        Vector3D operator+();
        Vector3D operator+(const Vector3D& other);
        Vector3D operator-();
        Vector3D operator-(const Vector3D& other);
        Vector3D operator*(const Vector3D& other);
        Vector3D operator*(const float& other);
        Vector3D operator/(const Vector3D& other);
        Vector3D operator/(const float& other);

        static Vector3D zero();
    };

    struct Vertex
    {
        Vector3D coord, normal;
        Vector2D uv;
        bool flipped;
    };

    struct Polygon
    {
        char texture_name[MAX_TEXTURE_NAME];
        bool flipped;
        std::vector<Vertex> vertices;
    public:
        Vector3D normal();
    };

    struct Texture
    {
        char name[MAX_TEXTURE_NAME];
        float shiftx, shifty, angle, scalex, scaley, width, height;
        Vector3D rightaxis, downaxis;
    };

    enum PointRelation
    {
        BEHIND = -1,
        ON_PLANE = 0,
        INFRONT = 1
    };

    class HessianPlane
    {
        Vector3D normal;
        float distance;
    public:
        float distanceToPoint(Vector3D point);
        PointRelation pointRelation(const Vector3D& point);
    };

    class Plane : HessianPlane
    {
        Vector3D planePoints[3];
        Texture texture;
    };

    Vector3D segmentsCross(Vector3D planePoints[3]);

}

std::ostream& operator<<(std::ostream& os, const M2PGeo::Vector3D& v);

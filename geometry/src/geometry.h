#pragma once
#include <iostream>
#include <vector>

namespace M2PGeo {

    struct Vector2D
    {
        float x, y;
    };

    struct Vector3D
    {
        float x, y, z;
    public:
        Vector3D zero();
        float magnitude() const;
        float dot(const Vector3D& b) const;
        Vector3D normalized() const;
        Vector3D cross(const Vector3D& b);
        Vector3D operator+();
        Vector3D operator+(const Vector3D& other);
        Vector3D operator-();
        Vector3D operator-(const Vector3D& other);
        Vector3D operator*(const Vector3D& other);
        Vector3D operator*(const float& other);
        Vector3D operator/(const Vector3D& other);
        Vector3D operator/(const float& other);
    };

    struct Vertex
    {
        Vector3D coord, normal;
        Vector2D uv;
        bool flipped;
    };

    struct Polygon
    {
        char texture_name[16];
        bool flipped;
        std::vector<Vertex> vertices;
    public:
        Vector3D normal();
    };

    struct Texture
    {
        char name[16];
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

std::ostream& operator<<(std::ostream& os, const M2PGeo::Vector3D v);

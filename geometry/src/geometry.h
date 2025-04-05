#pragma once
#include <iostream>
#include <vector>

namespace M2PGeo {

    struct Vector2D {
        float x, y;
    };

    struct Vector3D {
        float x, y, z;
    public:
        Vector3D zero();
        float magnitude() const;
        float dot(Vector3D b) const;
        Vector3D normalized() const;
        Vector3D cross(Vector3D b);
        Vector3D operator+();
        Vector3D operator+(Vector3D& other);
        Vector3D operator-();
        Vector3D operator-(Vector3D& other);
        Vector3D operator*(Vector3D& other);
        Vector3D operator*(float& other);
        Vector3D operator/(Vector3D& other);
        Vector3D operator/(float& other);
    };

    struct Vertex {
        Vector3D coord, normal;
        Vector2D uv;
        bool flipped;
    };

    struct Polygon {
        char texture[16];
        bool flipped;
        std::vector<Vertex> vertices;
    public:
        Vector3D normal();
    };

    Vector3D segmentsCross(Vector3D planePoints[3]);
    Vector3D planeNormal(Vector3D planePoints[3]);

}

std::ostream& operator<<(std::ostream& os, const M2PGeo::Vector3D v);

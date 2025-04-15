#pragma once
#include <vector>
#include <string>
#include "geometry.h"

namespace M2PEntity
{
    struct Face
    {
        std::vector<M2PGeo::Vector3> points;
        std::vector<M2PGeo::Vertex> vertices;
        std::vector<M2PGeo::Polygon> polygons;
        M2PGeo::Texture texture;
        M2PGeo::Vector3 m_normal;
    };

    struct Brush
    {
        std::vector<Face> faces;
        std::vector<M2PGeo::Vector3> allPoints;
        std::vector<M2PGeo::Vector3> allPolygons;
        std::vector<std::string> maskedTextures;
        std::string raw;
    public:
        bool hasToolTexture(const char*);
        bool hasContentWater();
        std::tuple<M2PGeo::Vector3, M2PGeo::Vector3> getBounds();
        M2PGeo::Vector3 getCenter();
    };

    struct Entity
    {
        std::string classname;
        std::map<std::string, std::string> properties;
        std::vector<Brush> brushes;
        std::string raw;
        Entity();
    };
}

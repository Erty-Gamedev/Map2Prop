#pragma once
#include <vector>
#include <string>
#include "geometry.h"

namespace M2PEntity
{
    struct Face
    {
        M2PGeo::Vector3 normal{};
        M2PGeo::Texture texture;
        std::vector<M2PGeo::Vector3> points;
        std::vector<M2PGeo::Vertex> vertices;
    };

    struct Brush
    {
        std::vector<Face> faces;
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


    class BaseReader
    {
    public:
        virtual bool hasMissingTextures() const = 0;
        virtual std::vector<Entity>& getEntities() = 0;
    };
}

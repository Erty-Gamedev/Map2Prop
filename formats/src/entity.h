#pragma once
#include <vector>
#include <string>
#include "geometry.h"
#include "wad3handler.h"

namespace M2PEntity
{
    struct Face
    {
        M2PGeo::Vector3 normal{};
        M2PGeo::Texture texture;
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
        std::vector< std::pair<std::string, std::string>> keyvalues;
        std::vector<Brush> brushes;
        std::string raw;
    public:
        Entity();
        std::string toString() const;
    };


    class BaseReader
    {
    public:
        std::vector<Entity> entities;
        M2PWad3::Wad3Handler wadHandler;

        virtual bool hasMissingTextures() const = 0;
    };
}

#pragma once
#include <vector>
#include <string>
#include "geometry.h"
#include "wad3handler.h"

namespace M2PEntity
{
    enum ToolTexture
    {
        BEVEL,
        BOUNDINGBOX,
        CLIP,
        CLIPBEVEL,
        CONTENTWATER,
        ORIGIN,
    };

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
        bool isToolBrush(ToolTexture toolTexture) const;
        bool hasContentWater() const;
        std::pair<M2PGeo::Vector3, M2PGeo::Vector3> getBounds() const;
        M2PGeo::Vector3 getCenter() const;
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
        bool hasKey(const std::string& key) const;
        std::string getKey(const std::string& key) const;
        void setKey(const std::string& key, const std::string& value);
        int getKeyInt(const std::string& key) const;
        bool getKeyBool(const std::string& key) const;
    };


    class BaseReader
    {
    public:
        std::vector<Entity> entities;
        M2PWad3::Wad3Handler wadHandler;

        virtual bool hasMissingTextures() const = 0;
    };
}

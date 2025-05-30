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

    class Brush
    {
    public:
        std::vector<Face> faces;
        std::string raw;

        bool isToolBrush(ToolTexture toolTexture) const;
        bool hasContentWater() const;
        M2PGeo::Bounds getBounds() const;
        M2PGeo::Vector3 getCenter() const;
        virtual std::string getRaw() const { return raw; }
    };

    class Entity
    {
    public:
        std::string classname;
        std::vector< std::pair<std::string, std::string>> keyvalues;
        std::vector<std::unique_ptr<Brush>> brushes;
        std::string raw;

        Entity();
        bool hasKey(const std::string& key) const;
        std::string getKey(const std::string& key) const;
        void setKey(const std::string& key, const std::string& value);
        int getKeyInt(const std::string& key) const;
        bool getKeyBool(const std::string& key) const;
        virtual std::string toString() const;
    };


    class BaseReader
    {
    public:
        std::vector<std::unique_ptr<Entity>> entities;
        M2PWad3::Wad3Handler wadHandler;

        bool hasMissingTextures() const { return wadHandler.hasMissingTextures(); };
    };
}

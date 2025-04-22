#pragma once

#include <string>
#include <vector>
#include <map>
#include "wad3.h"


namespace M2PWad3
{
    const std::vector< std::string> c_WADSKIPLIST{
        "cached", "decals", "fonts",
        "gfx", "spraypaint", "tempdecal"
    };
    const std::vector< std::string> c_TOOLTEXTURES{
        "bevel", "boundingbox", "clipbevel",
        "clip", "contentwater", "origin"
    };
    const std::vector< std::string> c_SKIPTEXTURES{
        "aaatrigger", "black_hidden", "clipbevelbrush",
        "cliphull1", "cliphull2", "cliphull3",
        "contentempty", "hint", "noclip", "null",
        "skip", "solidhint"
    };


    class ImageInfo
    {
    public:
        int width, height;
        ImageInfo() { width = 16; height = 16; }
        ImageInfo(const std::pair<int, int>&);
        ImageInfo(const std::string&);
        ~ImageInfo();
    private:
        std::ifstream m_file;
    };


    class Wad3Handler
    {
    public:
        ImageInfo& checkTexture(const std::string& textureName);

        bool isSkipTexture(const std::string& textureName);
        bool isToolTexture(const std::string& textureName);
        bool hasMissingTextures() const;

        static ImageInfo& s_getImageInfo(const std::string& textureName);
    private:
        bool m_missingTextures = false;
        std::vector<std::string> m_checked;
        std::map<std::filesystem::path, Wad3Reader> m_wadCache;
        std::vector<std::filesystem::path> m_wadpathList;
        std::vector<std::filesystem::path> m_usedWads;

        std::vector<std::filesystem::path>& getWadList();
        Wad3Reader& getWad3Reader(const std::filesystem::path& wad);
        Wad3Reader* checkWads(const std::string&);

        static inline std::map<std::string, ImageInfo> s_images;
    };
}

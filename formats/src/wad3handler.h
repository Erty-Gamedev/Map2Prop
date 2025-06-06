#pragma once

#include <string>
#include <vector>
#include <array>
#include <format>
#include <map>
#include <unordered_map>
#include "wad3.h"


namespace M2PWad3
{
    static inline const std::array<std::string, 6> c_TOOLTEXTURES{
        "bevel", "boundingbox", "clipbevel",
        "clip", "contentwater", "origin"
    };
    static inline const std::array<std::string, 12> c_SKIPTEXTURES{
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

        friend std::ostream& operator<<(std::ostream& os, const M2PWad3::ImageInfo& info);
    private:
        std::ifstream m_file;
    };

    struct ImageSize
    {
        int width = 16, height = 16;

        friend std::ostream& operator<<(std::ostream& os, const M2PWad3::ImageSize& size);
    };


    class Wad3Handler
    {
    public:
        std::vector<std::filesystem::path> usedWads;

        ImageSize checkTexture(const std::string& textureName);
        bool hasMissingTextures() const;

        static ImageSize s_getImageInfo(const std::string& textureName);
        static bool isSkipTexture(const std::string& textureName);
        static bool isToolTexture(const std::string& textureName);
    private:
        bool m_missingTextures = false;

        Wad3Reader& getWad3Reader(const std::filesystem::path& wad);
        Wad3Reader* checkWads(const std::string&);

        static inline std::map<std::filesystem::path, Wad3Reader> s_wadCache;
        static inline std::unordered_map<std::string, ImageSize> s_images;
    };
}

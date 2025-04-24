#pragma once

#include <cstdint>
#include <fstream>
#include <vector>

namespace M2PBmp
{
    constexpr size_t c_BMPSIZEHEADER = 1078; // sizeof(BMPHeader) + sizeof(BMPInfoHeader) + Palette (256 * 4)
    constexpr size_t c_BMPPALETTESIZE = 256;

#pragma pack(push, 1)
    struct BGRA
    {
        unsigned char b, g, r, a;
    };

    struct BMPHeader
    {
        unsigned char signature[2] = { 0x42, 0x4D };
        std::uint32_t filesize = 0;
        std::uint32_t reserved = 0;
        std::uint32_t dataOffset = 0;
    };

    struct BMPInfoHeader
    {
        std::uint32_t size;
        std::uint32_t width;
        std::uint32_t height;
        std::uint16_t planes;
        std::uint16_t bitsPerPixel;
        std::uint32_t compression;
        std::uint32_t imageSize;
        std::uint32_t horizontalPixelsPerM;
        std::uint32_t verticalPixelsPerM;
        std::uint32_t coloursUsed;
        std::uint32_t importantColours;
    };
#pragma pack(pop)

    class BMP8Bpp
    {
    private:
        BMPHeader m_header;
        BMPInfoHeader m_infoHeader;
        std::ofstream m_file;
    public:
        std::vector<unsigned char> m_data;
        std::vector<unsigned char> m_palette;

        BMP8Bpp(int width, int height);
        ~BMP8Bpp();
        bool save(const std::filesystem::path& filepath);
    };
}

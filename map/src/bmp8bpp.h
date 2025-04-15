#pragma once

#include <cstdint>

namespace M2PBmp
{
    struct BGRA
    {
        unsigned char b, g, r, a;
    };

    constexpr size_t c_BMPSIZEHEADER = 1078;

    struct BMPHeader
    {
        unsigned char signature[2] = { 0x42, 0x4D };
        std::uint32_t filesize;
        std::uint32_t reserved;
        std::uint32_t dataOffset;
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
}

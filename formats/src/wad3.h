#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <map>


namespace M2PWad3
{
    constexpr size_t c_MAXTEXTURENAME = 16;
    constexpr size_t c_MIPLEVELS = 4;
    constexpr size_t c_PALETTESIZE = 768;
    constexpr size_t c_BMPPALETTESIZE = 256;
    constexpr size_t c_MAXMIPTEXSIZE = 16384 * 16384;

#pragma pack(push, 1)
    enum EntryType : std::int8_t
    {
        QPIC = 0x42,
        MIPTEX = 0x43,
        FONT = 0x45,
        SPRAYDECAL = 0x40,
    };

    struct Wad3Header
    {
        char szMagic[4];         // should be WAD3
        std::int32_t nDir;       // number of directory entries
        std::int32_t nDirOffset; // offset into directory
    };

    struct Wad3DirEntry
    {
        std::int32_t nFilePos;         // offset in WAD
        std::int32_t nDiskSize;        // size in file
        std::int32_t nSize;            // uncompressed size
        std::int8_t nType;             // type of entry
        bool bCompression;             // 0 if none
        std::int16_t nDummy;           // not used
        char szName[c_MAXTEXTURENAME]; // must be null terminated
    };

    struct Wad3MipTex
    {
        char szName[c_MAXTEXTURENAME];       // Name of texture
        std::uint32_t nWidth, nHeight;       // Extends of the texture
        std::uint32_t nOffsets[c_MIPLEVELS]; // Offsets to texture mipmaps
    };


    struct MMData
    {
        std::uint32_t width, height;
        unsigned char palette[c_PALETTESIZE];
        std::vector<unsigned char> data;
    };

    //typedef std::map<std::string, std::reference_wrapper<MMData>> textureMap;
    typedef std::map<std::string, MMData> textureMap;


    struct Wad3Miptex
    {
        MMData data;
    public:
        bool save(const std::filesystem::path&) { return true; }
        //ImageInfo getImageInfo() { return ImageInfo(std::pair(16, 16)); }
    };
#pragma pack(pop)


    class Wad3Reader
    {
    public:
        Wad3Reader() {};
        Wad3Reader(const std::filesystem::path&);
        ~Wad3Reader();

        //textureMap& getTextures() const;
        std::string getFilename() const;
        bool containsTexture(const std::string& textureName);
    private:
        std::filesystem::path m_filepath;
        textureMap m_textures;
        std::ifstream m_file;
    };

    bool saveTexture(const MMData&, const std::filesystem::path&);

}

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
#pragma pack(pop)


    class Wad3Reader
    {
    public:
        Wad3Reader() {};
        Wad3Reader(const std::filesystem::path&);
        ~Wad3Reader();

        std::string getFilename() const;
        bool contains(const std::string& textureName);
        Wad3MipTex extract(const std::string& textureName, const std::filesystem::path& filepath);
    private:
        std::filesystem::path m_filepath;
        std::vector<Wad3DirEntry> m_dirEntries;
        std::ifstream m_file;
        std::ofstream m_outfile;

        void open();
        Wad3DirEntry* getDirEntry(const std::string& textureName);
    };
}

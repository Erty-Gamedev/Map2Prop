#pragma once
//#pragma pack(1)

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


    class Wad3Reader
    {
    public:
        Wad3Reader() {};
        Wad3Reader(const std::filesystem::path&);
        ~Wad3Reader();

        //textureMap& getTextures() const;
        std::string getFilename() const;
    private:
        std::filesystem::path m_filepath;
        textureMap m_textures;
        std::ifstream m_file;
    };

    typedef std::map<
        std::string, std::vector<std::reference_wrapper<Wad3Reader*>>
    > matchTextureMap;
    typedef std::map<std::string, matchTextureMap> foundMap;


    bool saveTexture(const MMData&, const std::filesystem::path&);


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
        ImageInfo& checkTexture(const std::string&);

        bool isSkipTexture(const std::string&);
        bool isToolTexture(const std::string&);
        bool hasMissingTextures() const;

        static ImageInfo& s_getImageInfo(const std::string&);
    private:
        bool m_missingTextures = false;
        std::vector<std::string> m_checked;
        std::map<std::filesystem::path, Wad3Reader> m_wads;

        std::vector<std::filesystem::path>& getWadList();
        Wad3Reader& getWad3Reader(const std::filesystem::path& wad);
        Wad3Reader* checkWads(const std::string&);

        static inline std::vector<std::filesystem::path> s_wadList;
        static inline std::map<std::string, ImageInfo> s_images;
    };

}

#pragma once

#include <array>
#include <fstream>
#include <filesystem>
#include "entity.h"

namespace M2PJMF
{
	static inline constexpr std::array<int, 2> c_SUPPORTED_VERSIONS{ 121, 122 };

#pragma pack(push, 1)
	struct JmfBgImage
	{
		double scale;
		std::int32_t luminance;
		std::int32_t filtering;
		std::int32_t invertColors;
		std::int32_t xOffset;
		std::int32_t yOffset;
		std::int32_t padding;
	};
	struct RGBA
	{
		unsigned char r, g, b, a;
	};
	struct JmfGroup
	{
		std::int32_t groupId;
		std::int32_t groupParentId;
		std::int32_t flags;
		std::int32_t childCount;
		RGBA color;
	};
	struct JmfVisgroup
	{
		std::int32_t visgroupId;
		RGBA color;
		unsigned char visible;
	};
	struct JmfCamera
	{
		float eyePosition[3];
		float lookatPosition[3];
		std::int32_t flags;
		RGBA color;
	};
	struct JmfEntityHeader
	{
		float origin[3];
		std::int32_t flags;
		std::int32_t groupId;
		std::int32_t rootGroupId;
		RGBA color;
	};
	struct JmfEntityBody
	{
		std::int32_t spawnflags;
		float angles[3];
		std::int32_t rendering;
		RGBA color;
		std::int32_t renderMode;
		std::int32_t renderFX;
		std::int16_t body;
		std::int16_t skin;
		std::int32_t sequence;
		float framerate;
		float scale;
		float radius;
		char _unknown[28];
		std::int32_t kvCount;
	};
	struct JmfBrushHeader
	{
		std::int32_t curveCount;
		std::int32_t flags;
		std::int32_t groupId;
		std::int32_t rootGroupId;
		RGBA color;
		std::int32_t visgroupCount;
	};
	struct JmfFace
	{
		float rightAxis[3];
		float shiftX;
		float downAxis[3];
		float shiftY;
		float scaleX, scaleY;
		float angle;
		std::int32_t alignment;
		char _unknown[12];
		std::int32_t contentFlags;
		char textureName[64];

		void toTexture(M2PGeo::Texture &texture) const {
			texture.name = textureName;
			texture.shiftx = shiftX;
			texture.shifty = shiftY;
			texture.angle = angle;
			texture.scalex = scaleX;
			texture.scaley = scaleY;
			texture.rightaxis = rightAxis;
			texture.downaxis = downAxis;
		}
	};
	struct JfmCurvePoint
	{
		float position[3];
		float normal[3];
		float uv[2];
		unsigned char selected;
	};
	struct JmfCurve
	{
		std::int32_t width, height;
		JmfFace face;
		char _unknown[4];
		JfmCurvePoint points[1024];
	};
	struct JmfVertex
	{
		float coords[3];
		float uv[2];
		std::int32_t selected;
	};
#pragma pack(pop)

	class JmfBrush : public M2PEntity::Brush
	{
	public:
		std::string getRaw() const override;
	};

	class JmfEntity : public M2PEntity::Entity
	{
	public:
		std::string toString() const override;
	};

}

namespace M2PFormat
{
	class JmfReader : public M2PEntity::BaseReader
	{
	public:
		JmfReader(const std::filesystem::path& filepath, const std::filesystem::path& outputDir);
		~JmfReader();
	private:
		int m_version;
		std::filesystem::path m_filepath;
		std::filesystem::path m_outputDir;
		std::ifstream m_file;

		void parse();

		void readBgImage();
		void readGroup();
		void readVisgroup();
		void readCamera();
		void readPath();
		void readPathNode();
		void readEntity();
		void readBrush(M2PEntity::Entity& parent);
		void readCurve();
		M2PEntity::Face readFace();
	};
}

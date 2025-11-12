#include <format>
#include "entity.h"
#include "utils.h"
#include "wad3handler.h"

using namespace M2PEntity;


bool Brush::isToolBrush(ToolTexture toolTexture) const
{
	std::string textureName;
	if (toolTexture == ToolTexture::BEVEL)
		textureName = "bevel";
	else if (toolTexture == ToolTexture::BOUNDINGBOX)
		textureName = "boundingbox";
	else if (toolTexture == ToolTexture::CLIP)
		textureName = "clip";
	else if (toolTexture == ToolTexture::CLIPBEVEL)
		textureName = "clipbevel";
	else if (toolTexture == ToolTexture::CONTENTWATER)
		textureName = "contentwater";
	else if (toolTexture == ToolTexture::ORIGIN)
		textureName = "origin";

	for (const auto& face : faces)
	{
		if (M2PUtils::toLowerCase(face.texture.name) != textureName)
			return false;
	}
	return true;
}

bool Brush::isToolBrushAny() const
{
	for (const auto& face : faces)
	{
		if (!M2PUtils::contains(c_toolTextures, M2PUtils::toLowerCase(face.texture.name)))
			return false;
	}
	return true;
}

bool Brush::hasContentWater() const
{
	for (const auto& face : faces)
	{
		if (M2PUtils::toLowerCase(face.texture.name) == "contentwater")
			return true;
	}
	return false;
}

M2PGeo::Bounds Brush::getBounds() const
{
	M2PGeo::Vector3 min = faces[0].vertices[0].coord();
	M2PGeo::Vector3 max = faces[0].vertices[0].coord();

	for (const auto& face : faces)
	{
		if (M2PWad3::Wad3Handler::isSkipTexture(face.texture.name))
			continue;

		for (const auto& vertex : face.vertices)
		{
			if (vertex.x < min.x) min.x = vertex.x;
			if (vertex.y < min.y) min.y = vertex.y;
			if (vertex.z < min.z) min.z = vertex.z;

			if (vertex.x > max.x) max.x = vertex.x;
			if (vertex.y > max.y) max.y = vertex.y;
			if (vertex.z > max.z) max.z = vertex.z;
		}
	}

	return { min, max };
}

M2PGeo::Vector3 Brush::getCenter() const
{
	M2PGeo::Bounds bounds = getBounds();
	return (bounds.min + bounds.max) / 2;
}


Entity::Entity()
{
	raw.reserve(1024);
}

std::string Entity::toString() const
{
	std::string str;
	str.reserve(raw.size());
	str = "{\n";

	for (std::pair<std::string, std::string> const& pair : keyvalues)
	{
		str += std::format("\"{}\" \"{}\"\n", pair.first, pair.second);
	}

	if (!brushes.empty())
	{
		for (auto &brush : brushes)
		{
			str += "{\n";
			str += brush->getRaw();
			str += "}\n";
		}
	}

	str += "}\n";

	return str;
}

bool Entity::hasKey(const std::string& key) const
{
	for (const std::pair<std::string, std::string>& kv : keyvalues)
	{
		if (key == kv.first)
			return true;
	}
	return false;
}

std::string Entity::getKey(const std::string& key) const
{
	for (const std::pair<std::string, std::string>& kv : keyvalues)
	{
		if (key == kv.first)
			return kv.second;
	}
	return "";
}

void Entity::setKey(const std::string& key, const std::string& value)
{
	for (int i = 0; i < keyvalues.size(); ++i)
	{
		if (key == keyvalues[i].first)
		{
			keyvalues[i].second = value;
			return;
		}
	}
	keyvalues.emplace_back(key, value);
}

int Entity::getKeyInt(const std::string& key) const
{
	return atoi(getKey(key).c_str());
}

FP Entity::getKeyFloat(const std::string& key) const
{
	return static_cast<FP>(atof(getKey(key).c_str()));
}

bool Entity::getKeyBool(const std::string& key) const
{
	return getKeyInt(key) != 0;
}

M2PGeo::Vector3 Entity::getOrigin() const
{
	std::vector<std::string> parts = M2PUtils::split(getKey("origin"));
	if (parts.size() != 3)
		return M2PGeo::Vector3::zero();
	return { std::stof(parts[0]), std::stof(parts[1]), std::stof(parts[2]) };
}

M2PGeo::Bounds Entity::getBounds() const
{
	if (brushes.empty())
		return M2PGeo::Bounds::zero();

	M2PGeo::Bounds bounds;

	// Find first non-tool brush
	for (const auto& brush : brushes)
	{
		if (!brush->isToolBrushAny())
		{
			bounds = brush->getBounds();
			break;
		}
	}

	for (const auto& brush : brushes)
	{
		if (brush->isToolBrushAny())
			continue;

		M2PGeo::Bounds current = brush->getBounds();
		if (current.min.x < bounds.min.x) bounds.min.x = current.min.x;
		if (current.min.y < bounds.min.y) bounds.min.y = current.min.y;
		if (current.min.z < bounds.min.z) bounds.min.z = current.min.z;

		if (current.max.x > bounds.max.x) bounds.max.x = current.max.x;
		if (current.max.y > bounds.max.y) bounds.max.y = current.max.y;
		if (current.max.z > bounds.max.z) bounds.max.z = current.max.z;
	}

	return bounds;
}

M2PGeo::Bounds Entity::getCustomBounds() const
{
	std::string cMin = getKey("customclip_min"), cMax = getKey("customclip_max");
	if (cMin.empty() || cMax.empty())
		return M2PGeo::Bounds::zero();

	std::vector<std::string> minParts = M2PUtils::split(cMin);
	std::vector<std::string> maxParts = M2PUtils::split(cMax);

	if (minParts.size() != 3 || maxParts.size() != 3)
		return M2PGeo::Bounds::zero();

	M2PGeo::Bounds bounds{
		{ std::stof(minParts[0]), std::stof(minParts[1]), std::stof(minParts[2]) },
		{ std::stof(maxParts[0]), std::stof(maxParts[1]), std::stof(maxParts[2]) }
	};


	M2PGeo::Vector3 cBoundsSize = bounds.getSize() / 2;
	M2PGeo::Vector3 entOrigin = getOrigin();
	M2PGeo::Vector3 boundsCenter;

	if (getKeyBool("customclip_align"))
	{
		M2PGeo::Bounds entBounds = getBounds();
		if (entBounds == M2PGeo::Bounds::zero())
			boundsCenter = entOrigin;
		else
			boundsCenter = (entBounds.min + entBounds.max) / 2;
	}
	else
	{
		boundsCenter = getOrigin();
		boundsCenter.z += cBoundsSize.z;
	}

	return { boundsCenter - cBoundsSize, boundsCenter + cBoundsSize };
}

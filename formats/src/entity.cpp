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
		for (Brush const& brush : brushes)
		{
			str += "{\n";
			str += brush.raw;
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

bool Entity::getKeyBool(const std::string& key) const
{
	return getKeyInt(key) != 0;
}

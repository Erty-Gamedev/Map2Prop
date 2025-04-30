#include <format>
#include "entity.h"

using namespace M2PEntity;

Entity::Entity()
{
	raw.reserve(1024);
}

std::string M2PEntity::Entity::toString() const
{
	std::string str;
	str.reserve(raw.size());
	str = "{\n";

	for (std::pair<std::string, std::string> const& pair : keyvalues)
	{
		str += std::format("\"{}\" \"{}\"\n", pair.first, pair.second);
	}

	if (brushes.size() > 0)
	{
		str += "{\n";
		for (M2PEntity::Brush const& brush : brushes)
		{
			str += brush.raw;
		}
		str += "}\n";
	}

	str += "}\n";

	return str;
}

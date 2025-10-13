#pragma once
#include "geometry.h"

namespace M2PGeo
{
	using Vertex3 = std::tuple<Vertex, Vertex, Vertex>;

	std::vector<Triangle> earClip(
		const std::vector<Vertex> &_polygon,
		const Vector3 &normal
	);
}

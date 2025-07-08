#include "doctest.h"
#include "map_format.h"
#include "geometry.h"

using namespace M2PGeo;
using namespace M2PMAP;


TEST_SUITE("map_format")
{
	TEST_CASE("test intersection 3 planes")
	{
		Vector3 intersection;
		Vector3 expected{ 32, 32, 32 };
		HessianPlane p1 = { Vector3(1, 0, 0), 32.0f };
		HessianPlane p2 = { Vector3(0, 1, 0), 32.0f };
		HessianPlane p3 = { Vector3(0, 0, 1), 32.0f };

		CHECK(intersection3Planes(p1, p2, p3, intersection) == true);
		CHECK(intersection == expected);
	}

	TEST_CASE("test no intersection")
	{
		Vector3 intersection;
		Vector3 expected;
		HessianPlane p1 = { Vector3(1, 0, 0), 32.0f };
		HessianPlane p2 = { Vector3(-1, 0, 0), 32.0f };
		HessianPlane p3 = { Vector3(0, -1, 0), 32.0f };

		CHECK(intersection3Planes(p1, p2, p3, intersection) == false);
		CHECK(intersection == expected);
	}
}

#include "doctest.h"
#include "ear_clip.h"

using namespace M2PGeo;

TEST_SUITE("ear_clip")
{
    TEST_CASE("test triangulation")
    {
        Vertex A{ 16, 16, 0 };
        Vertex B{ -16, 16, 0 };
        Vertex C{ -16, -16, 0 };
        Vertex D{ 16, -16, 0 };

        std::vector<Triangle> triangles = earClip(std::vector<Vertex>{A, B, C, D}, Vertex{ 0, 0, 1 });

        CHECK(triangles[0].vertices[0] == D);
        CHECK(triangles[0].vertices[1] == A);
        CHECK(triangles[0].vertices[2] == B);

        CHECK(triangles[1].vertices[0] == B);
        CHECK(triangles[1].vertices[1] == C);
        CHECK(triangles[1].vertices[2] == D);
    }
}

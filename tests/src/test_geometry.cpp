#include "doctest.h"
#include "geometry.h"

using namespace M2PGeo;


TEST_CASE("test angle between vectors")
{
    Vector3 a{ .8f, .9f, 1.f };
    Vector3 b{ 1.f, 0.f, 0.f };

    FP result = a.angle(b);
    FP expected = 1.0343f;

    CHECK(abs(result - expected) < 0.001);
}

TEST_CASE("test sort vertices")
{
    Vertex a{ 0.0f, -.25f, 1.0f };
    Vertex b{ 0.0f, .25f, 1.0f };
    Vertex c{ -.28236f, -.60355f, .78723f };
    Vertex d{ -.65862f, -.60355f, .50369f };
    Vertex e{ -.94679f, -.27159f, .28654f };
    Vertex f{ -.94725f, .24074f, .2862f };
    Vertex g{ -.66172f, .60355f, .50136f };
    Vertex h{ -.27808f, .59819f, .79045f };

    Vector3 planePoints[3] = { a.coord(), g.coord(), e.coord() };
    HessianPlane plane{ planePoints };

    std::vector<Vertex> vertices{ a, d, c, g, f, h, e, b };
    std::vector<Vertex> expected{ a, b, h, g, f, e, d, c };

    sortVertices(vertices, plane.normal());

    CHECK(vertices == expected);
}

#include "doctest.h"
#include <cmath>
#include "geometry.h"

#pragma warning ( disable: 4305 )

using namespace M2PGeo;


TEST_SUITE("geometry")
{
    TEST_CASE("test angle between vectors")
    {
        Vector3 a{ .8f, .9f, 1.f };
        Vector3 b{ 1.f, 0.f, 0.f };

        FP result = a.angle(b);
        FP expected = 1.0343f;

        CHECK(abs(result - expected) < 0.0001);
    }

    TEST_CASE("test angle between vectors 2")
    {
        Vector3 a{ 0, 0, 1 }; // Normal of a plane flat on the horizontal plane
        FP delta{ 0.0001 };

        SUBCASE("30 degree angle")
        {
            Vector3 b{ cos(deg2rad(60)), 0, sin(deg2rad(60)) };
            CHECK(abs(rad2deg(a.angle(b)) - 30) < delta);
        }

        SUBCASE("60 degree angle")
        {
            Vector3 b{ cos(deg2rad(30)), 0, sin(deg2rad(30)) };
            CHECK(abs(rad2deg(a.angle(b)) - 60) < delta);
        }

        SUBCASE("90 degree angle")
        {
            Vector3 b{ 1, 0, 0 };
            CHECK(abs(rad2deg(a.angle(b)) - 90) < delta);
        }

        SUBCASE("150 degree angle")
        {
            Vector3 b{ cos(deg2rad(300)), 0, sin(deg2rad(300)) };
            CHECK(abs(rad2deg(a.angle(b)) - 150) < delta);
        }
    }

    TEST_CASE("test hessian plane")
    {
        Vector3 points[3]{ Vector3(4.2f, -5.13f, 6.71f), Vector3(3.19f, 0.07f, 0.56f), Vector3(0.99f, 4.14f, 4.67f) };
        HessianPlane plane{ points };
        Vector3 point{ -1.78f, 0.04f, -0.39f };

        CHECK(abs(plane.distance() - 3.05541) < 0.0001);
        CHECK(abs(plane.distanceToPoint(point) - (-4.74378)) < 0.0001);
        CHECK(plane.pointRelation(point) == PointRelation::BEHIND);
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
}

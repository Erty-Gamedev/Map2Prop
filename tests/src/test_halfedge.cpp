#include "doctest.h"
#include "geometry.h"
#include "halfedge.h"
#include <array>

using namespace M2PHalfEdge;


TEST_CASE("walk and smooth fans")
{
    Mesh mesh;
    M2PGeo::Vertex v0{  32,   24, 64 };
    M2PGeo::Vertex v1{ -48,  120,  0 };
    M2PGeo::Vertex v2{  24,  128,  0 };
    M2PGeo::Vertex v3{  96,   96,  0 };
    M2PGeo::Vertex v4{ 128,  -16,  0 };
    M2PGeo::Vertex v5{  96,  -96,  0 };
    M2PGeo::Vertex v6{  -8, -112,  0 };
    M2PGeo::Vertex v7{ -96,  -16,  0 };
    M2PGeo::Vertex v8{ -96,   64,  0 };

    std::array<M2PGeo::Triangle, 14> triangles{
        // topside
        M2PGeo::Triangle{.vertices = { v0, v2, v1 } },
        M2PGeo::Triangle{.vertices = { v0, v3, v2 } },
        M2PGeo::Triangle{.vertices = { v0, v4, v3 } },
        M2PGeo::Triangle{.vertices = { v0, v5, v4 } },
        M2PGeo::Triangle{.vertices = { v0, v6, v5 } },
        M2PGeo::Triangle{.vertices = { v0, v7, v6 } },
        M2PGeo::Triangle{.vertices = { v0, v8, v7 } },
        M2PGeo::Triangle{.vertices = { v0, v1, v8 } },

        // underside
        M2PGeo::Triangle{.vertices = { v8, v1, v2 } },
        M2PGeo::Triangle{.vertices = { v2, v3, v4 } },
        M2PGeo::Triangle{.vertices = { v4, v5, v6 } },
        M2PGeo::Triangle{.vertices = { v6, v7, v8 } },
        M2PGeo::Triangle{.vertices = { v6, v8, v4 } },
        M2PGeo::Triangle{.vertices = { v4, v8, v2 } }
    };
    for (M2PGeo::Triangle& triangle : triangles)
    {
        M2PGeo::Vector3 planepoints[3] = { triangle.vertices[0].coord(), triangle.vertices[1].coord(), triangle.vertices[2].coord() };
        triangle.normal = M2PGeo::planeNormal(planepoints);

        mesh.addTriangle(triangle, triangle.normal, M2PGeo::Texture());
    }

    CHECK(mesh.vertices[0]->coord() == v0.coord());


    std::vector<M2PGeo::Bounds> alwaysSmooth;
    std::vector<M2PGeo::Bounds> neverSmooth{
        M2PGeo::Bounds{{-64, 16, -8}, {40, 144, 72}},
        M2PGeo::Bounds{{24, -104, -8}, {104, 32, 72}}
    };

    mesh.markSmoothEdges(90., alwaysSmooth, neverSmooth);

    SUBCASE("start at v1-v0")
    {
        auto fans = mesh.getSmoothFansByVertex(mesh.vertices[0]);

        CHECK(fans.size() == 3);

        CHECK(fans[0].faces.size() == 4);
        CHECK(fans[1].faces.size() == 1);
        CHECK(fans[2].faces.size() == 3);

        M2PGeo::Vector3 expectedNormals[4] = {
            M2PGeo::Vector3{-0.0578f, 0.52f,  0.852f}.normalised(),
            M2PGeo::Vector3{ 0.482f,  0.155f, 0.862f}.normalised(),
            M2PGeo::Vector3{-0.269f, -0.18f,  0.946f}.normalised()
        };

        CHECK((fans[0].accumulatedNormal / fans[0].faces.size()).normalised() == expectedNormals[2]);
        CHECK((fans[1].accumulatedNormal / fans[1].faces.size()).normalised() == expectedNormals[0]);
        CHECK((fans[2].accumulatedNormal / fans[2].faces.size()).normalised() == expectedNormals[1]);
    }

    SUBCASE("start at v7-v0")
    {
        for (auto& edge : mesh.edges)
        {
            if (edge->origin->coord() == mesh.vertices[0]->coord() && edge->next->origin->coord() == v4.coord())
            {
                mesh.vertices[0]->edge = edge;
                break;
            }
        }

        auto fans = mesh.getSmoothFansByVertex(mesh.vertices[0]);

        CHECK(fans.size() == 3);

        CHECK(fans[0].faces.size() == 3);
        CHECK(fans[1].faces.size() == 4);
        CHECK(fans[2].faces.size() == 1);
    }

}

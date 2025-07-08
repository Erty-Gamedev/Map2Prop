#include "doctest.h"
#include <array>
#include "geometry.h"
#include "halfedge.h"

using namespace M2PHalfEdge;


TEST_SUITE("half_edge")
{
    TEST_CASE("walk and smooth fans")
    {
        Mesh mesh;
        M2PGeo::Vertex v0{ 32,   24, 64 };
        M2PGeo::Vertex v1{ -48,  120,  0 };
        M2PGeo::Vertex v2{ 24,  128,  0 };
        M2PGeo::Vertex v3{ 96,   96,  0 };
        M2PGeo::Vertex v4{ 128,  -16,  0 };
        M2PGeo::Vertex v5{ 96,  -96,  0 };
        M2PGeo::Vertex v6{ -8, -112,  0 };
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

        mesh.markSmoothEdges(60., alwaysSmooth, neverSmooth);


        SUBCASE("all bottom faces should have downwards normals")
        {
            for (auto& vertex : mesh.vertices)
                mesh.getSmoothFansByVertex(vertex);

            M2PGeo::Vector3 expected{ 0, 0, -1 };

            for (int i = 8; i < 14; ++i)
            {
                const auto& face = mesh.faces[i];
                for (auto& v : face->vertices)
                    CHECK(v->normal == expected);
            }
        }

        SUBCASE("check expected edges are sharp")
        {

            std::vector<std::pair<M2PGeo::Vertex, M2PGeo::Vertex>> expectedSharp{
                {v0, v1}, {v0, v2}, {v0, v5}, // internal sharps
                {v2, v1}, {v3, v2}, {v4, v3}, {v5, v4}, {v6, v5}, {v7, v6}, {v8, v7}, {v1, v8}, // border sharps
            };

            for (const auto& edge : mesh.edges)
            {
                bool shouldBeSharp = false;

                for (const std::pair<M2PGeo::Vertex, M2PGeo::Vertex>& expected : expectedSharp)
                {
                    if ((edge->origin->coord() == expected.first && edge->next->origin->coord() == expected.second)
                        || (edge->origin->coord() == expected.second && edge->next->origin->coord() == expected.first))
                    {
                        shouldBeSharp = true;
                        break;
                    }
                }
                CAPTURE(edge->origin->coord());
                CAPTURE(edge->next->origin->coord());
                CAPTURE(edge->face->index);
                CAPTURE(edge->twin->face->index);

                CAPTURE(M2PGeo::rad2deg(edge->face->normal.angle(edge->twin->face->normal)));

                CHECK(edge->sharp == shouldBeSharp);
            }
        }

        SUBCASE("start at v1-v0")
        {
            auto fans = mesh.getSmoothFansByVertex(mesh.vertices[0]);

            CHECK(fans.size() == 3);

            CHECK(fans[0].faces.size() == 4);
            CHECK(fans[1].faces.size() == 1);
            CHECK(fans[2].faces.size() == 3);

            M2PGeo::Vector3 expectedNormals[3] = {
                M2PGeo::Vector3{-0.269f, -0.18f,  0.946f}.normalised(),
                M2PGeo::Vector3{-0.0578f, 0.52f,  0.852f}.normalised(),
                M2PGeo::Vector3{ 0.482f,  0.155f, 0.862f}.normalised()
            };

            CHECK(fans[0].accumulatedNormal.normalised() == expectedNormals[0]);
            CHECK(fans[1].accumulatedNormal.normalised() == expectedNormals[1]);
            CHECK(fans[2].accumulatedNormal.normalised() == expectedNormals[2]);
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

        SUBCASE("check edge vertex")
        {
            auto fans = mesh.getSmoothFansByVertex(mesh.vertices[6]);

            CHECK(fans.size() == 2);

            CHECK(fans[0].faces.size() == 3);
            CHECK(fans[1].faces.size() == 2);

            CHECK(fans[0].accumulatedNormal.normalised() == M2PGeo::Vector3{ 0, 0, -1 });
        }

    }

}

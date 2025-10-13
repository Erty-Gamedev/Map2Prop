#include "doctest.h"
#include <array>
#include "geometry.h"
#include "halfedge.h"

#pragma warning ( disable: 4305 )

using namespace M2PHalfEdge;


TEST_SUITE("half_edge")
{
    TEST_CASE("walk and smooth fans (irregular spike)")
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
            // Topside
            M2PGeo::Triangle{.vertices = { v0, v2, v1 } },
            M2PGeo::Triangle{.vertices = { v0, v3, v2 } },
            M2PGeo::Triangle{.vertices = { v0, v4, v3 } },
            M2PGeo::Triangle{.vertices = { v0, v5, v4 } },
            M2PGeo::Triangle{.vertices = { v0, v6, v5 } },
            M2PGeo::Triangle{.vertices = { v0, v7, v6 } },
            M2PGeo::Triangle{.vertices = { v0, v8, v7 } },
            M2PGeo::Triangle{.vertices = { v0, v1, v8 } },

            // Underside
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

            mesh.addTriangle(triangle, M2PGeo::Texture());
        }

        mesh.markSmoothEdges(60., {}, {
            M2PGeo::Bounds{ {-64, 16, -8}, {40, 144, 72} },
            M2PGeo::Bounds{ {24, -104, -8}, {104, 32, 72} }
        });


        SUBCASE("all bottom faces should have downwards normals")
        {
            for (auto& pVertex : mesh.coords)
                mesh.getSmoothFansByVertex(*pVertex);

            M2PGeo::Vector3 expected{ 0, 0, -1 };

            for (int i = 8; i < 14; ++i)
            {
                const auto& face = mesh.faces[i];
                for (auto& v : face->vertices)
                    CHECK(v.normal == expected);
            }
        }

        SUBCASE("check expected edges are sharp")
        {
            std::vector<std::pair<M2PGeo::Vertex, M2PGeo::Vertex>> expectedSharp{
                // Internal sharp edges
                {v0, v1}, {v0, v2}, {v0, v5},

                // Border edges
                {v2, v1}, {v3, v2}, {v4, v3}, {v5, v4}, {v6, v5}, {v7, v6}, {v8, v7}, {v1, v8}
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

                CHECK(edge->sharp == shouldBeSharp);
            }
        }

        SUBCASE("check fans at v0")
        {
            auto fans = mesh.getSmoothFansByVertex(*mesh.coords[0]);

            CHECK(fans.size() == 3);

            CHECK(fans[0].faces.size() == 4);
            CHECK(fans[1].faces.size() == 1);
            CHECK(fans[2].faces.size() == 3);

            M2PGeo::Vector3 expectedNormals[3]{
                M2PGeo::Vector3{-0.269f, -0.18f,  0.946f}.normalised(),
                M2PGeo::Vector3{-0.0578f, 0.52f,  0.852f}.normalised(),
                M2PGeo::Vector3{ 0.482f,  0.155f, 0.862f}.normalised()
            };

            for (int i = 0; i < 3; ++i)
                CHECK(fans[i].accumulatedNormal.normalised() == expectedNormals[i]);
        }

        SUBCASE("start at edge v7-v0")
        {
            for (auto& edge : mesh.edges)
            {
                if (edge->origin->coord() == mesh.coords[0]->coord() && edge->next->origin->coord() == v4.coord())
                {
                    mesh.coords[0]->edge = edge.get();
                    break;
                }
            }

            auto fans = mesh.getSmoothFansByVertex(*mesh.coords[0]);

            CHECK(fans.size() == 3);

            CHECK(fans[0].faces.size() == 3);
            CHECK(fans[1].faces.size() == 4);
            CHECK(fans[2].faces.size() == 1);
        }

        SUBCASE("check edge vertex")
        {
            auto fans = mesh.getSmoothFansByVertex(*mesh.coords[6]);

            CHECK(fans.size() == 2);

            CHECK(fans[0].faces.size() == 3);
            CHECK(fans[1].faces.size() == 2);

            CHECK(fans[0].accumulatedNormal.normalised() == M2PGeo::Vector3{ 0, 0, -1 });
        }

    }

    TEST_CASE("smooth cylinder open top face")
    {
        Mesh mesh;
        M2PGeo::Vertex v0{  -6,  6,  0 };
        M2PGeo::Vertex v1{   0,  8,  0 };
        M2PGeo::Vertex v2{   6,  6,  0 };
        M2PGeo::Vertex v3{   8,  0,  0 };
        M2PGeo::Vertex v4{   6, -6,  0 };
        M2PGeo::Vertex v5{   0, -8,  0 };
        M2PGeo::Vertex v6{  -6, -6,  0 };
        M2PGeo::Vertex v7{  -8,  0,  0 };
        M2PGeo::Vertex v8{  -6,  6, 72 };
        M2PGeo::Vertex v9{   0,  8, 72 };
        M2PGeo::Vertex v10{ -8,  0, 72 };
        M2PGeo::Vertex v11{ -6, -6, 72 };
        M2PGeo::Vertex v12{  0, -8, 72 };
        M2PGeo::Vertex v13{  6, -6, 72 };
        M2PGeo::Vertex v14{  8,  0, 72 };
        M2PGeo::Vertex v15{  6,  6, 72 };

        std::array<M2PGeo::Triangle, 22> triangles{
            M2PGeo::Triangle{.vertices = {  v0,  v1,  v2 } },
            M2PGeo::Triangle{.vertices = {  v2,  v3,  v4 } },
            M2PGeo::Triangle{.vertices = {  v4,  v5,  v6 } },
            M2PGeo::Triangle{.vertices = {  v6,  v7,  v0 } },
            M2PGeo::Triangle{.vertices = {  v0,  v2,  v4 } },
            M2PGeo::Triangle{.vertices = {  v4,  v6,  v0 } },
            M2PGeo::Triangle{.vertices = {  v0,  v8,  v9 } },
            M2PGeo::Triangle{.vertices = {  v9,  v1,  v0 } },
            M2PGeo::Triangle{.vertices = {  v7, v10,  v8 } },
            M2PGeo::Triangle{.vertices = {  v8,  v0,  v7 } },
            M2PGeo::Triangle{.vertices = {  v6, v11, v10 } },
            M2PGeo::Triangle{.vertices = { v10,  v7,  v6 } },
            M2PGeo::Triangle{.vertices = {  v5, v12, v11 } },
            M2PGeo::Triangle{.vertices = { v11,  v6,  v5 } },
            M2PGeo::Triangle{.vertices = {  v4, v13, v12 } },
            M2PGeo::Triangle{.vertices = { v12,  v5,  v4 } },
            M2PGeo::Triangle{.vertices = {  v3, v14, v13 } },
            M2PGeo::Triangle{.vertices = { v13,  v4,  v3 } },
            M2PGeo::Triangle{.vertices = {  v2, v15, v14 } },
            M2PGeo::Triangle{.vertices = { v14,  v3,  v2 } },
            M2PGeo::Triangle{.vertices = {  v1,  v9, v15 } },
            M2PGeo::Triangle{.vertices = { v15,  v2,  v1 } }
        };

        for (M2PGeo::Triangle& triangle : triangles)
        {
            M2PGeo::Vector3 planepoints[3] = { triangle.vertices[0].coord(), triangle.vertices[1].coord(), triangle.vertices[2].coord() };
            triangle.normal = M2PGeo::planeNormal(planepoints);

            mesh.addTriangle(triangle, M2PGeo::Texture());
        }

        mesh.markSmoothEdges(60., {}, {
            M2PGeo::Bounds{{-40, -40, 96}, {40, 40, 136}}
        });


        SUBCASE("check expected edges are sharp")
        {
            std::vector<std::pair<M2PGeo::Vertex, M2PGeo::Vertex>> expectedSharp{
                // Top
                {v9, v8}, {v8, v10}, {v10, v11}, {v11, v12}, {v12, v13}, {v13, v14}, {v14, v15}, {v15, v9},
                
                // Bottom
                {v1, v2}, {v2, v3}, {v3, v4}, {v4, v5}, {v5, v6}, {v6, v7}, {v7, v0}, {v0, v1}
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

                CHECK(edge->sharp == shouldBeSharp);
            }
        }

        SUBCASE("check top vertices (open)")
        {
            std::array<int, 8> topVertices{ 8, 9, 10, 11, 12, 13, 14, 15 };
            M2PGeo::Vector3 up{ 0, 0, 1 };

            for (int i : topVertices)
            {
                auto fans = mesh.getSmoothFansByVertex(*mesh.coords[i]);
                // We should only have one fan, each with 3 faces, and the normal should not be pointing up

                CHECK(fans.size() == 1);
                CHECK(fans[0].faces.size() == 3);
                CHECK(fans[0].accumulatedNormal.normalised() != up);
            }
        }

        SUBCASE("check bottom vertices")
        {
            std::array<int, 8> bottomVertices{ 0, 1, 2, 3, 4, 5, 6, 7 };
            M2PGeo::Vector3 down{ 0, 0, -1 };

            for (int i : bottomVertices)
            {
                auto fans = mesh.getSmoothFansByVertex(*mesh.coords[i]);
                // We should have two fans, one for the side and one for the bottom

                CHECK(fans.size() == 2);

                bool hasBottomFan{ false };
                bool hasSideFan{ false };

                for (const auto& fan : fans)
                {
                    if (fan.accumulatedNormal.normalised() == down)
                        hasBottomFan = true;
                    else
                    {
                        hasSideFan = true;
                        CHECK(fan.faces.size() == 3);
                    }
                }
                CHECK((hasBottomFan && hasSideFan) == true);
            }
        }
    }

    TEST_CASE("two meshes with intersecting faces")
    {
        Mesh mesh;
        M2PGeo::Vertex v0{    0, -16,  -8 };
        M2PGeo::Vertex v1{    0,  16,  -8 };
        M2PGeo::Vertex v2{    0,  16,   8 };
        M2PGeo::Vertex v3{    0, -16,   8 };
        M2PGeo::Vertex v4{   -8, -16, -24 };
        M2PGeo::Vertex v5{   -8, -16,  24 };
        M2PGeo::Vertex v6{  -24, -16,  32 };
        M2PGeo::Vertex v7{  -40, -16,  24 };
        M2PGeo::Vertex v8{  -48, -16,  -0 };
        M2PGeo::Vertex v9{  -40, -16, -24 };
        M2PGeo::Vertex v10{ -24, -16, -32 };
        M2PGeo::Vertex v11{ -24,  16, -32 };
        M2PGeo::Vertex v12{  -8,  16, -24 };
        M2PGeo::Vertex v13{ -24,  16,  32 };
        M2PGeo::Vertex v14{  -8,  16,  24 };
        M2PGeo::Vertex v15{ -40,  16,  24 };
        M2PGeo::Vertex v16{ -48,  16,  -0 };
        M2PGeo::Vertex v17{ -40,  16, -24 };
        M2PGeo::Vertex v18{   8,  16, -24 };
        M2PGeo::Vertex v19{   8,  16,  24 };
        M2PGeo::Vertex v20{  24,  16,  32 };
        M2PGeo::Vertex v21{  40,  16,  24 };
        M2PGeo::Vertex v22{  48,  16,   0 };
        M2PGeo::Vertex v23{  40,  16, -24 };
        M2PGeo::Vertex v24{  24,  16, -32 };
        M2PGeo::Vertex v25{  24, -16, -32 };
        M2PGeo::Vertex v26{   8, -16, -24 };
        M2PGeo::Vertex v27{  24, -16,  32 };
        M2PGeo::Vertex v28{   8, -16,  24 };
        M2PGeo::Vertex v29{  40, -16,  24 };
        M2PGeo::Vertex v30{  48, -16,   0 };
        M2PGeo::Vertex v31{  40, -16, -24 };

        std::array<M2PGeo::Triangle, 50> triangles{
            M2PGeo::Triangle{.vertices = { v0, v1, v2 } },
            M2PGeo::Triangle{.vertices = { v2, v3, v0 } },
            M2PGeo::Triangle{.vertices = { v4, v0, v3 } },
            M2PGeo::Triangle{.vertices = { v3, v5, v6 } },
            M2PGeo::Triangle{.vertices = { v7, v8, v9 } },
            M2PGeo::Triangle{.vertices = { v10, v4, v3 } },
            M2PGeo::Triangle{.vertices = { v6, v7, v9 } },
            M2PGeo::Triangle{.vertices = { v3, v6, v9 } },
            M2PGeo::Triangle{.vertices = { v3, v9, v10 } },
            M2PGeo::Triangle{.vertices = { v10, v11, v12 } },
            M2PGeo::Triangle{.vertices = { v12, v4, v10 } },
            M2PGeo::Triangle{.vertices = { v13, v6, v5 } },
            M2PGeo::Triangle{.vertices = { v5, v14, v13 } },
            M2PGeo::Triangle{.vertices = { v5, v3, v2 } },
            M2PGeo::Triangle{.vertices = { v2, v14, v5 } },
            M2PGeo::Triangle{.vertices = { v15, v7, v6 } },
            M2PGeo::Triangle{.vertices = { v6, v13, v15 } },
            M2PGeo::Triangle{.vertices = { v4, v12, v1 } },
            M2PGeo::Triangle{.vertices = { v1, v0, v4 } },
            M2PGeo::Triangle{.vertices = { v16, v8, v7 } },
            M2PGeo::Triangle{.vertices = { v7, v15, v16 } },
            M2PGeo::Triangle{.vertices = { v9, v17, v11 } },
            M2PGeo::Triangle{.vertices = { v11, v10, v9 } },
            M2PGeo::Triangle{.vertices = { v17, v9, v8 } },
            M2PGeo::Triangle{.vertices = { v8, v16, v17 } },
            M2PGeo::Triangle{.vertices = { v1, v0, v3 } },
            M2PGeo::Triangle{.vertices = { v3, v2, v1 } },
            M2PGeo::Triangle{.vertices = { v18, v1, v2 } },
            M2PGeo::Triangle{.vertices = { v2, v19, v20 } },
            M2PGeo::Triangle{.vertices = { v21, v22, v23 } },
            M2PGeo::Triangle{.vertices = { v24, v18, v2 } },
            M2PGeo::Triangle{.vertices = { v20, v21, v23 } },
            M2PGeo::Triangle{.vertices = { v2, v20, v23 } },
            M2PGeo::Triangle{.vertices = { v2, v23, v24 } },
            M2PGeo::Triangle{.vertices = { v24, v25, v26 } },
            M2PGeo::Triangle{.vertices = { v26, v18, v24 } },
            M2PGeo::Triangle{.vertices = { v27, v20, v19 } },
            M2PGeo::Triangle{.vertices = { v19, v28, v27 } },
            M2PGeo::Triangle{.vertices = { v19, v2, v3 } },
            M2PGeo::Triangle{.vertices = { v3, v28, v19 } },
            M2PGeo::Triangle{.vertices = { v29, v21, v20 } },
            M2PGeo::Triangle{.vertices = { v20, v27, v29 } },
            M2PGeo::Triangle{.vertices = { v18, v26, v0 } },
            M2PGeo::Triangle{.vertices = { v0, v1, v18 } },
            M2PGeo::Triangle{.vertices = { v30, v22, v21 } },
            M2PGeo::Triangle{.vertices = { v21, v29, v30 } },
            M2PGeo::Triangle{.vertices = { v23, v31, v25 } },
            M2PGeo::Triangle{.vertices = { v25, v24, v23 } },
            M2PGeo::Triangle{.vertices = { v31, v23, v22 } },
            M2PGeo::Triangle{.vertices = { v22, v30, v31 } }
        };


        for (M2PGeo::Triangle& triangle : triangles)
        {
            M2PGeo::Vector3 planepoints[3] = { triangle.vertices[0].coord(), triangle.vertices[1].coord(), triangle.vertices[2].coord() };
            triangle.normal = M2PGeo::planeNormal(planepoints);

            mesh.addTriangle(triangle, M2PGeo::Texture());
        }

        mesh.markSmoothEdges(60., {}, {});


        auto fans = mesh.getSmoothFansByVertex(*mesh.coords[0]);

        CHECK(fans.size() == 3);


        SUBCASE("check expected edges are sharp")
        {
            std::vector<std::pair<M2PGeo::Vertex, M2PGeo::Vertex>> expectedSharp{
                // Internal sharp edges
                {v0, v1}, {v1, v2}, {v2, v3}, {v3, v0},

                // Border edges front left
                {v0, v3}, {v3, v5}, {v5, v6}, {v6, v7}, {v7, v8}, {v8, v9}, {v9, v10}, {v10, v4}, {v4, v0},
                // Border edges front right
                {v3, v0}, {v0, v26}, {v26, v25}, {v25, v31}, {v31, v30}, {v30, v29}, {v29, v27}, {v27, v28}, {v28, v3},

                // Border edges back left
                {v1, v2}, {v2, v19}, {v19, v20}, {v20, v21}, {v21, v22}, {v22, v23}, {v23, v24}, {v24, v18}, {v18, v1},
                // Border edges back right
                {v2, v1}, {v1, v12}, {v12, v11}, {v11, v17}, {v17, v16}, {v16, v15}, {v15, v13}, {v13, v14}, {v14, v2}
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

                CAPTURE(edge->origin->index);
                CAPTURE(edge->next->origin->index);
                CHECK(edge->sharp == shouldBeSharp);
            }
        }
    }
}

#include "ear_clip.h"
#include "utils.h"

namespace M2PGeo { bool g_isEager = false; }
using M2PGeo::g_isEager;
using namespace M2PGeo;


static inline bool pointInsideTriangle(const Vector3 &point, const Vertex3 &triangle)
{
    Vector3 a = std::get<0>(triangle).coord() - point;
    Vector3 b = std::get<1>(triangle).coord() - point;
    Vector3 c = std::get<2>(triangle).coord() - point;

    Vector3 u = b.cross(c);
    Vector3 v = c.cross(a);
    Vector3 w = a.cross(b);

    if (u.dot(v) < 0.0 || u.dot(w) < 0.0)
        return false;

    return true;
}

static inline bool anyPointsInsideTriangle(const std::vector<Vertex>& vertices, const Vertex3 &triangle)
{
    for (const Vertex& vertex : vertices)
    {
        Vector3 point = vertex.coord();

        // Skip checking triangle points
        if (point == std::get<0>(triangle).coord() || point == std::get<1>(triangle).coord() || point == std::get<2>(triangle).coord())
            continue;

        if (pointInsideTriangle(point, triangle))
            return true;
    }
    return false;
}

static inline int findOptimalEar(const std::vector<Vertex>& polygon, const std::vector<Vertex>& fullPolygon, Vector3 normal)
{
    size_t maxIndex = polygon.size();

    int optimalIndex = -1;
    FP currentOptimal = 0;
    FP optimalDot = .5;

    for (int i = 0; i < maxIndex; ++i)
    {
        Vector3 point = polygon[i].coord();
        Vector3 pPrev = M2PUtils::getCircular(polygon, i - 1).coord();
        Vector3 pNext = M2PUtils::getCircular(polygon, i + 1).coord();

        Vector3 cross = (pPrev - point).normalised().cross((pNext - point).normalised());
        FP normalDot = -normal.dot(cross);

        if (normalDot > 0.)
        {
            if (anyPointsInsideTriangle(fullPolygon, std::make_tuple(pPrev, point, pNext)))
                continue;

            if (g_isEager)
                return i;

            FP delta = abs(optimalDot - normalDot);
            if (optimalIndex == -1 || delta < currentOptimal)
            {
                optimalIndex = i;
                currentOptimal = delta;
                continue;
            }
        }
    }

    if (optimalIndex < 0)
        throw std::runtime_error("Triangulation failed");

    return optimalIndex;
}

std::vector<Triangle> M2PGeo::earClip(const std::vector<Vertex>& _polygon, const Vector3& normal)
{
    size_t numVertices = _polygon.size();

    if (numVertices == 3)
        return std::vector<Triangle>{Triangle{
            .flipped = false,
            .normal = normal,
            .vertices = {_polygon[0], _polygon[1], _polygon[2]}
        }};
    if (numVertices < 3)
        throw std::runtime_error("Polygon with less than 3 sides");

    std::vector<Vertex> polygon(_polygon);  // Make a modifiable copy

    std::vector<Triangle> triangles;
    triangles.reserve(numVertices - 2);

    while (polygon.size() > 3)
    {
        int i = findOptimalEar(polygon, _polygon, normal);

        triangles.push_back(Triangle{
            .flipped = false,
            .normal = normal,
            .vertices = {
                M2PUtils::getCircular(polygon, i - 1),
                polygon[i],
                M2PUtils::getCircular(polygon, i + 1)
            }
        });

        polygon.erase(polygon.begin() + i);
    }

    triangles.push_back(Triangle{
        .flipped = false,
        .normal = normal,
        .vertices = {polygon[0], polygon[1], polygon[2]}
    });

    return triangles;
}

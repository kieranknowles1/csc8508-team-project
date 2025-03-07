#include <random>
#include "NavMesh.h"
#include "stdio.h"

NavMesh::NavMesh(btDiscreteDynamicsWorld* bulletWorld) :
    world(bulletWorld) {}

bool NavMesh::LoadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open NavMesh file: " << filename << std::endl;
        return false;
    }

    int numVertices, numIndices;
    file >> numVertices >> numIndices;

    int numTriangles = numIndices / 3; // Convert indices to actual triangles

    // Load vertices
    vertices.resize(numVertices);
    for (int i = 0; i < numVertices; ++i) {
        float x, y, z;
        file >> x >> y >> z;
        vertices[i] = btVector3(x * scale, y * scale, z * scale);
    }

    // Load triangle indices
    triangles.resize(numTriangles);
    for (int i = 0; i < numTriangles; ++i) {
        file >> triangles[i].v1 >> triangles[i].v2 >> triangles[i].v3;

        // Validate indices
        if (triangles[i].v1 < 0 || triangles[i].v1 >= numVertices ||
            triangles[i].v2 < 0 || triangles[i].v2 >= numVertices ||
            triangles[i].v3 < 0 || triangles[i].v3 >= numVertices) {
            std::cerr << "ERROR: Triangle " << i << " has invalid vertex indices: ("
                << triangles[i].v1 << ", " << triangles[i].v2 << ", " << triangles[i].v3 << ")" << std::endl;
        }
    }

    // Load neighbor indices
    for (int i = 0; i < numTriangles; ++i) {
        file >> triangles[i].neighbor1 >> triangles[i].neighbor2 >> triangles[i].neighbor3;

        // Validate neighbors (should be -1 or a valid triangle index)
        if ((triangles[i].neighbor1 < -1 || triangles[i].neighbor1 >= numTriangles) ||
            (triangles[i].neighbor2 < -1 || triangles[i].neighbor2 >= numTriangles) ||
            (triangles[i].neighbor3 < -1 || triangles[i].neighbor3 >= numTriangles)) {
            std::cerr << "WARNING: Triangle " << i << " has invalid neighbors: ("
                << triangles[i].neighbor1 << ", " << triangles[i].neighbor2 << ", " << triangles[i].neighbor3 << ")" << std::endl;
        }
    }

    // Debug Output
    std::cout << "NavMesh Loaded: " << numVertices << " vertices, " << numTriangles << " triangles" << std::endl;

    return true;
}

bool NavMesh::PointInTriangle(const btVector3& p, const btVector3& a, const btVector3& b, const btVector3& c) {
    btVector3 v0 = c - a;
    btVector3 v1 = b - a;
    btVector3 v2 = p - a;

    float dot00 = v0.dot(v0);
    float dot01 = v0.dot(v1);
    float dot02 = v0.dot(v2);
    float dot11 = v1.dot(v1);
    float dot12 = v1.dot(v2);

    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    return (u >= 0) && (v >= 0) && (u + v <= 1);
}

// Find which triangle contains a given point
int NavMesh::GetTriangleContainingPoint(const btVector3& point) {

    for (size_t i = 0; i < triangles.size(); ++i) {
        const Triangle& tri = triangles[i];

        const btVector3& a = vertices[tri.v1];
        const btVector3& b = vertices[tri.v2];
        const btVector3& c = vertices[tri.v3];

        if (PointInTriangle(point, a, b, c)) {
            return i;
        }
    }

    return -1;
}

float NavMesh::GetYFromPoint(float x, float z) {
    btVector3 pointXZ(x, 0, z);  // Ignore Y for triangle search

    for (size_t i = 0; i < triangles.size(); ++i) {
        const Triangle& tri = triangles[i];

        // Project triangle vertices onto the XZ plane
        const btVector3& A = vertices[tri.v1];
        const btVector3& B = vertices[tri.v2];
        const btVector3& C = vertices[tri.v3];

        btVector3 aXZ(A.x(), 0, A.z());
        btVector3 bXZ(B.x(), 0, B.z());
        btVector3 cXZ(C.x(), 0, C.z());

        if (PointInTriangle(pointXZ, aXZ, bXZ, cXZ)) {
            return GetBarycentricInterpolatedY(A, B, C, x, z);
        }
    }

    std::cerr << "WARNING: No triangle found for (" << x << ", " << z << "). Returning default Y = 0.\n";
    return 0.0f;  // Return 0 or a default height if no triangle is found
}

// Get the neighbors of a given triangle
std::vector<int> NavMesh::GetNeighbors(int triangleIndex) {
    std::vector<int> neighbors;
    if (triangleIndex < 0 || triangleIndex >= (int)triangles.size()) {
        std::cerr << "ERROR: GetNeighbors() called with invalid triangleIndex: " << triangleIndex << std::endl;
        return neighbors;
    }

    const Triangle& tri = triangles[triangleIndex];
    if (tri.neighbor1 != -1) neighbors.push_back(tri.neighbor1);
    if (tri.neighbor2 != -1) neighbors.push_back(tri.neighbor2);
    if (tri.neighbor3 != -1) neighbors.push_back(tri.neighbor3);

    if (neighbors.empty()) {
        std::cerr << "WARNING: Triangle " << triangleIndex << " has NO neighbors!" << std::endl;
    }

    return neighbors;
}

void NavMesh::VisualiseNavMesh() {
    if (!world || !world->getDebugDrawer()) {
        return; // Ensure the debug drawer is available
    }

    btIDebugDraw* debugDrawer = world->getDebugDrawer();
    btVector3 color(0, 1, 0); // Green for NavMesh triangles
    int i = 0;
    for (const Triangle& tri : triangles) {

        const btVector3& a = vertices[tri.v1];
        const btVector3& b = vertices[tri.v2];
        const btVector3& c = vertices[tri.v3];

        // Draw triangle edges
        debugDrawer->drawLine(a, b, color);
        debugDrawer->drawLine(b, c, color);
        debugDrawer->drawLine(c, a, color);
    }
}

std::vector<btVector3> NavMesh::FindPath(const btVector3& start, const btVector3& end) {
    int startTri = GetTriangleContainingPoint(start);
    int endTri = GetTriangleContainingPoint(end);

    if (startTri == -1 || endTri == -1) {
        std::cerr << "Start or End point not inside NavMesh!" << std::endl;
        return {};
    }

    std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> openSet;
    std::unordered_map<int, int> cameFrom;
    std::unordered_map<int, float> gScore;
    std::unordered_map<int, float> fScore;

    openSet.push({ startTri, 0.0f, vertices[startTri].distance(vertices[endTri]) });
    gScore[startTri] = 0.0f;
    fScore[startTri] = vertices[startTri].distance(vertices[endTri]);

    while (!openSet.empty()) {
        AStarNode current = openSet.top();
        openSet.pop();

        if (current.triangleIndex == endTri) {
            break;
        }

        for (int neighbor : GetNeighbors(current.triangleIndex)) {
            float tentativeG = gScore[current.triangleIndex] + vertices[current.triangleIndex].distance(vertices[neighbor]);

            if (!gScore.count(neighbor) || tentativeG < gScore[neighbor]) {
                cameFrom[neighbor] = current.triangleIndex;
                gScore[neighbor] = tentativeG;
                fScore[neighbor] = tentativeG + vertices[neighbor].distance(vertices[endTri]);

                openSet.push({ neighbor, gScore[neighbor], fScore[neighbor] });
            }
        }
    }

    // Reconstruct triangle path
    std::vector<int> trianglePath;
    int current = endTri;
    while (cameFrom.count(current)) {
        trianglePath.push_back(current);
        current = cameFrom[current];
    }
    trianglePath.push_back(startTri);
    std::reverse(trianglePath.begin(), trianglePath.end());

    // Extract portals (shared edges)
    std::vector<std::pair<btVector3, btVector3>> portals = ExtractPortals(trianglePath);

    // Apply Funnel Algorithm to optimize the path
    return ApplyFunnelAlgorithm(start, end, portals);
}


void NavMesh::DebugDrawPath(const std::vector<btVector3>& path) {
    if (!world || !world->getDebugDrawer()) {
        return;
    }

    btIDebugDraw* debugDrawer = world->getDebugDrawer();
    btVector3 color(1, 0, 0); // Red for path

    for (size_t i = 0; i < path.size() - 1; ++i) {
        debugDrawer->drawLine(path[i], path[i + 1], color);
    }
}

btVector3 NavMesh::GetRandomPointInNavMesh() {
    if (triangles.empty()) {
        std::cerr << "Error: No triangles in NavMesh!" << std::endl;
        return btVector3(0, 0, 0);
    }

    // Step 1: Pick a random triangle
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> triangleDist(0, triangles.size() - 1);
    int randTriangleIndex = triangleDist(gen);

    const Triangle& tri = triangles[randTriangleIndex];

    const btVector3& A = vertices[tri.v1];
    const btVector3& B = vertices[tri.v2];
    const btVector3& C = vertices[tri.v3];

    // Step 2: Generate random Barycentric coordinates
    std::uniform_real_distribution<float> barycentricDist(0.0f, 1.0f);
    float r1 = barycentricDist(gen);
    float r2 = barycentricDist(gen);

    // Ensure point remains inside triangle
    if (r1 + r2 > 1.0f) {
        r1 = 1.0f - r1;
        r2 = 1.0f - r2;
    }

    // Compute final random point using Barycentric interpolation
    btVector3 randomPoint = A + r1 * (B - A) + r2 * (C - A);

    return randomPoint;
}

std::vector<std::pair<btVector3, btVector3>> NavMesh::ExtractPortals(const std::vector<int>& trianglePath) {
    std::vector<std::pair<btVector3, btVector3>> portals;

    for (size_t i = 0; i < trianglePath.size() - 1; ++i) {
        const Triangle& currentTri = triangles[trianglePath[i]];
        const Triangle& nextTri = triangles[trianglePath[i + 1]];

        // Find the shared edge between the two triangles
        std::vector<btVector3> sharedVertices;
        btVector3 verticesCurrent[3] = { vertices[currentTri.v1], vertices[currentTri.v2], vertices[currentTri.v3] };
        btVector3 verticesNext[3] = { vertices[nextTri.v1], vertices[nextTri.v2], vertices[nextTri.v3] };

        for (int a = 0; a < 3; ++a) {
            for (int b = 0; b < 3; ++b) {
                if (verticesCurrent[a] == verticesNext[b]) {
                    sharedVertices.push_back(verticesCurrent[a]);
                }
            }
        }

        if (sharedVertices.size() == 2) { // A valid edge should have exactly 2 shared vertices
            portals.push_back({ sharedVertices[0], sharedVertices[1] });
        }
    }
    return portals;
}

bool NavMesh::IsLeftOf(const btVector3& a, const btVector3& b, const btVector3& c) {
    return ((b.x() - a.x()) * (c.z() - a.z()) - (b.z() - a.z()) * (c.x() - a.x())) > 0;
}

std::vector<btVector3> NavMesh::ApplyFunnelAlgorithm(const btVector3& start, const btVector3& end,
    const std::vector<std::pair<btVector3, btVector3>>& portals) {
    std::vector<btVector3> smoothPath;
    smoothPath.push_back(start);  // Start at the actual starting position

    if (portals.empty()) {
        smoothPath.push_back(end);
        return smoothPath;
    }

    btVector3 left = portals[0].first;
    btVector3 right = portals[0].second;
    size_t apexIndex = 0, leftIndex = 0, rightIndex = 0;

    btVector3 apex = start;

    for (size_t i = 1; i < portals.size(); ++i) {
        btVector3 newLeft = portals[i].first;
        btVector3 newRight = portals[i].second;

        // Check if the left edge should be updated
        if (IsLeftOf(apex, right, newLeft)) {
            if (IsLeftOf(apex, left, newLeft)) {
                smoothPath.push_back(left);
                apex = left;
                apexIndex = leftIndex;
                right = apex;
                left = newLeft;
                leftIndex = i;
                rightIndex = i;
                continue;
            }
            left = newLeft;
            leftIndex = i;
        }

        // Check if the right edge should be updated
        if (!IsLeftOf(apex, left, newRight)) {
            if (!IsLeftOf(apex, right, newRight)) {
                smoothPath.push_back(right);
                apex = right;
                apexIndex = rightIndex;
                left = apex;
                right = newRight;
                leftIndex = i;
                rightIndex = i;
                continue;
            }
            right = newRight;
            rightIndex = i;
        }
    }

    smoothPath.push_back(end);  // End at the actual end position
    return smoothPath;
}

float NavMesh::GetBarycentricInterpolatedY(const btVector3& A, const btVector3& B, const btVector3& C, float x, float z) {
    btVector3 P(x, 0, z); // XZ input, ignore Y

    // Compute Barycentric coordinates
    btVector3 v0 = C - A;
    btVector3 v1 = B - A;
    btVector3 v2 = P - A;

    float d00 = v0.dot(v0);
    float d01 = v0.dot(v1);
    float d11 = v1.dot(v1);
    float d20 = v2.dot(v0);
    float d21 = v2.dot(v1);

    float denom = d00 * d11 - d01 * d01;
    float u = (d11 * d20 - d01 * d21) / denom;
    float v = (d00 * d21 - d01 * d20) / denom;
    float w = 1 - u - v;

    return w * A.y() + u * C.y() + v * B.y();
}
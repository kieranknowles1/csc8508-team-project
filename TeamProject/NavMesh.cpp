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
        vertices[i] = btVector3(x, y, z);
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

    // Reconstruct path
    std::vector<btVector3> path;
    int current = endTri;
    while (cameFrom.count(current)) {
        const Triangle& tri = triangles[current];

        // Add triangle center to path
        path.push_back((vertices[tri.v1] + vertices[tri.v2] + vertices[tri.v3]) / 3);

        current = cameFrom[current];
    }
    path.push_back(start);  // Add start point
    std::reverse(path.begin(), path.end());

    return path;
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
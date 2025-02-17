#include "NavMesh.h"
#include "stdio.h"

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

// Find which triangle contains a given point
int NavMesh::GetTriangleContainingPoint(const btVector3& point) {
    for (size_t i = 0; i < triangles.size(); ++i) {
        const Triangle& tri = triangles[i];

        const btVector3& a = vertices[tri.v1];
        const btVector3& b = vertices[tri.v2];
        const btVector3& c = vertices[tri.v3];

        // Simple point-in-triangle check using cross products (ignoring height)
        btVector3 ab = b - a;
        btVector3 ac = c - a;
        btVector3 ap = point - a;

        float d1 = ab.x() * ap.y() - ab.y() * ap.x();
        float d2 = ac.x() * ap.y() - ac.y() * ap.x();

        if ((d1 >= 0 && d2 <= 0) || (d1 <= 0 && d2 >= 0)) {
            return i; // Found the triangle
        }
    }
    return -1; // Not inside any triangle
}

// Get the neighbors of a given triangle
std::vector<int> NavMesh::GetNeighbors(int triangleIndex) {
    std::vector<int> neighbors;
    if (triangleIndex < 0 || triangleIndex >= (int)triangles.size()) return neighbors;

    const Triangle& tri = triangles[triangleIndex];
    if (tri.neighbor1 != -1) neighbors.push_back(tri.neighbor1);
    if (tri.neighbor2 != -1) neighbors.push_back(tri.neighbor2);
    if (tri.neighbor3 != -1) neighbors.push_back(tri.neighbor3);
    return neighbors;
}

void NavMesh::VisualiseNavMesh(btDiscreteDynamicsWorld* world) {
    if (!world || !world->getDebugDrawer()) {
        return; // Ensure the debug drawer is available
    }

    btIDebugDraw* debugDrawer = world->getDebugDrawer();
    btVector3 color(0, 1, 0); // Green for NavMesh triangles
    int i = 0;
    for (const Triangle& tri : triangles) {
        std::cout << i++ << std::endl;

        const btVector3& a = vertices[tri.v1];
        const btVector3& b = vertices[tri.v2];
        const btVector3& c = vertices[tri.v3];

        // Draw triangle edges
        debugDrawer->drawLine(a, b, color);
        debugDrawer->drawLine(b, c, color);
        debugDrawer->drawLine(c, a, color);
    }
}
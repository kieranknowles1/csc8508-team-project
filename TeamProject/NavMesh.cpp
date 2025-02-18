#include "NavMesh.h"
#include "stdio.h"

bool NavMesh::LoadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open NavMesh file: " << filename << std::endl;
        return false;
    }

    int numVertices, numTriangles;
    file >> numVertices;
    file >> numTriangles;

    // Load vertices
    vertices.resize(numVertices);
    for (int i = 0; i < numVertices; ++i) {
        float x, y, z;
        file >> x >> y >> z;
        vertices[i] = btVector3(x, y, z);
    }

    // Load triangles
    triangles.resize(numTriangles);
    for (int i = 0; i < numTriangles; ++i) {
        file >> triangles[i].v1 >> triangles[i].v2 >> triangles[i].v3;
    }

    // Load neighbors
    for (int i = 0; i < numTriangles; ++i) {
        file >> triangles[i].neighbor1 >> triangles[i].neighbor2 >> triangles[i].neighbor3;
    }

    std::cout << "Vertices: " << vertices.size() << ", Triangles: " << triangles.size() << std::endl;
    for (const Triangle& tri : triangles) {
        std::cout << "Triangle: " << tri.v1 << " " << tri.v2 << " " << tri.v3 << std::endl;
    }


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

        if (tri.v1 < 0 || tri.v2 < 0 || tri.v3 < 0 ||
            tri.v1 >= vertices.size() || tri.v2 >= vertices.size() || tri.v3 >= vertices.size()) {
            std::cerr << "Skipping invalid triangle: " << i << " ("
                << tri.v1 << ", " << tri.v2 << ", " << tri.v3 << ")" << std::endl;
            continue;
        }

        const btVector3& a = vertices[tri.v1];
        const btVector3& b = vertices[tri.v2];
        const btVector3& c = vertices[tri.v3];

        // Draw triangle edges
        debugDrawer->drawLine(a, b, color);
        debugDrawer->drawLine(b, c, color);
        debugDrawer->drawLine(c, a, color);
    }
}
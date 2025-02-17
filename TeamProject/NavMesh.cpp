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

// A* Pathfinding Algorithm
std::vector<btVector3> NavMesh::FindPath(const btVector3& start, const btVector3& end) {
    int startTri = GetTriangleContainingPoint(start);
    int endTri = GetTriangleContainingPoint(end);


    if (startTri == -1 || endTri == -1) {
        std::cerr << "Start or End point not inside NavMesh!" << std::endl;
        return {};
    }

    struct Node {
        int triIndex;
        float gCost, hCost;
        float TotalCost() const { return gCost + hCost; }
    };

    struct CompareNode {
        bool operator()(const Node& a, const Node& b) {
            return a.TotalCost() > b.TotalCost();
        }
    };

    std::priority_queue<Node, std::vector<Node>, CompareNode> openSet;
    std::unordered_map<int, int> cameFrom;
    std::unordered_map<int, float> gScore;
    std::unordered_map<int, float> fScore;

    openSet.push({ startTri, 0.0f, vertices[startTri].distance(vertices[endTri]) });
    gScore[startTri] = 0.0f;
    fScore[startTri] = vertices[startTri].distance(vertices[endTri]);

    while (!openSet.empty()) {
        Node current = openSet.top();
        openSet.pop();

        if (current.triIndex == endTri) {
            return ReconstructPath(cameFrom, endTri, startTri);
        }

        for (int neighbor : GetNeighbors(current.triIndex)) {
            float tentativeG = gScore[current.triIndex] + vertices[current.triIndex].distance(vertices[neighbor]);

            if (!gScore.count(neighbor) || tentativeG < gScore[neighbor]) {
                cameFrom[neighbor] = current.triIndex;
                gScore[neighbor] = tentativeG;
                fScore[neighbor] = tentativeG + vertices[neighbor].distance(vertices[endTri]);

                openSet.push({ neighbor, gScore[neighbor], fScore[neighbor] });
            }
        }
    }

    return {}; // No path found
}

// Reconstructs path from A* search
std::vector<btVector3> NavMesh::ReconstructPath(std::unordered_map<int, int>& cameFrom, int current, int start) {
    std::vector<btVector3> path;
    while (current != start) {
        const Triangle& tri = triangles[current];
        path.push_back(vertices[tri.v1]);
        path.push_back(vertices[tri.v2]);
        path.push_back(vertices[tri.v3]);

        current = cameFrom[current];
    }
    std::reverse(path.begin(), path.end());
    return path;
}

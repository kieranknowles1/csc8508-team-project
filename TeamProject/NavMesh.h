#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <queue>
#include <cmath>
#include <btBulletDynamicsCommon.h>

struct Triangle {
    int v1, v2, v3;  // Indices of vertices
    int neighbor1, neighbor2, neighbor3;  // Neighboring triangle indices (-1 if none)

    bool ContainsVertex(int vertex) const {
        return v1 == vertex || v2 == vertex || v3 == vertex;
    }
};

class NavMesh {
public:
    bool LoadFromFile(const std::string& filename);
    std::vector<btVector3> FindPath(const btVector3& start, const btVector3& end);

private:
    std::vector<btVector3> vertices;
    std::vector<Triangle> triangles;

    int GetTriangleContainingPoint(const btVector3& point);
    std::vector<int> GetNeighbors(int triangleIndex);
    std::vector<btVector3> ReconstructPath(std::unordered_map<int, int>& cameFrom, int current, int start);
};

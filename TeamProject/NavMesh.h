#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <queue>
#include <cmath>
#include <btBulletDynamicsCommon.h>
#include "BulletDebug.h"



struct Triangle {
    int v1, v2, v3;  // Indices of vertices
    int neighbor1, neighbor2, neighbor3;  // Neighboring triangle indices (-1 if none)

    bool ContainsVertex(int vertex) const {
        return v1 == vertex || v2 == vertex || v3 == vertex;
    }
};

class NavMesh {
public:
    NavMesh(btDiscreteDynamicsWorld* bulletWorld);
    bool LoadFromFile(const std::string& filename);
    void VisualiseNavMesh();

//private:
    std::vector<btVector3> vertices;
    std::vector<Triangle> triangles;
    btDiscreteDynamicsWorld* world;

    int GetTriangleContainingPoint(const btVector3& point);
    std::vector<int> GetNeighbors(int triangleIndex);
};

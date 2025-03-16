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

struct AStarNode {
    int triangleIndex;
    float gCost, hCost;

    float TotalCost() const { return gCost + hCost; }

    bool operator>(const AStarNode& other) const {
        return TotalCost() > other.TotalCost();
    }
};

class NavMesh {
public:
    NavMesh(btDiscreteDynamicsWorld* bulletWorld);
    bool LoadFromFile(const std::string& filename);
    void VisualiseNavMesh();

    std::vector<btVector3> FindPath(const btVector3& start, const btVector3& end);
    void DebugDrawPath(const std::vector<btVector3>& path);
    btVector3 GetRandomPointInNavMesh();
    float GetYFromPoint(float x, float z);

private:
    std::vector<btVector3> vertices;
    std::vector<Triangle> triangles;
    btDiscreteDynamicsWorld* world;

    int GetTriangleContainingPoint(const btVector3& point);
    std::vector<int> GetNeighbors(int triangleIndex);
    bool PointInTriangle(const btVector3& p, const btVector3& a, const btVector3& b, const btVector3& c);
    float scale = 20.0f;
    std::vector<std::pair<btVector3, btVector3>> ExtractPortals(const std::vector<int>& trianglePath);
    bool IsLeftOf(const btVector3& a, const btVector3& b, const btVector3& c);
    std::vector<btVector3> ApplyFunnelAlgorithm(const btVector3& start, const btVector3& end, const std::vector<std::pair<btVector3, btVector3>>& portals);
    float GetBarycentricInterpolatedY(const btVector3& A, const btVector3& B, const btVector3& C, float x, float z);
};

// C:\important\quiet\n\mimita-public\mimita-public\src\world\world.h
// dec 16 2025
/**
 * purpose
 * make chunks 
 * helper for chunks
 * so we can do multi pass
 * so collisions work 
 * so i can walk on spheres and triangles and cones and whatever 
 * not just boring csgo flat land walls maaaaube some  surfing NO
 * mi game is COOL and works with BELNDER 
 * ...biiiiiiitch
 */

#pragma once
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include "world-types.h"
#include "map/map_common.h"

// Hash function for glm::ivec3 to use as key in unordered_map
namespace std {
    template<>
    struct hash<glm::ivec3> {
        size_t operator()(const glm::ivec3& v) const {
            size_t h1 = std::hash<int>()(v.x);
            size_t h2 = std::hash<int>()(v.y);
            size_t h3 = std::hash<int>()(v.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}

struct Chunk
{
    std::vector<Triangle> tris;
};

struct World
{
    float chunkSize = 8.0f;
    std::unordered_map<glm::ivec3, Chunk> chunks;

    void buildFromMesh(const Mesh& mesh);
    void getNearbyTriangles(glm::vec3 pos, std::vector<Triangle>& out) const;
};

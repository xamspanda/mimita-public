// physics.cpp
#include "physics.h"
#include "../camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cstdio>

// -------------------------------------------------
// Tunables
// -------------------------------------------------
const float GRAVITY         = -25.0f;
const float MOVE_SPEED     = 15.0f;
const float JUMP_SPEED     = 10.0f;

const float CAPSULE_RADIUS = 0.25f;
const float CAPSULE_HEIGHT = 1.8f;

const float MAX_SLOPE_DEG  = 50.0f;
const float FLOOR_DOT_MIN  =
    glm::cos(glm::radians(90.0f - MAX_SLOPE_DEG));

const float SKIN = 0.01f; // 1cm

// hack fix need something cooler later dec 12 2025 
const float TRI_CULL_DIST = 25.0f; // only check triangles near player

// -------------------------------------------------
// Small helpers
// -------------------------------------------------
static inline glm::vec3 slide(const glm::vec3& v, const glm::vec3& n)
{
    return v - n * glm::dot(v, n);
}

static inline float planeDist(
    const glm::vec3& p,
    const glm::vec3& n,
    const glm::vec3& p0)
{
    return glm::dot(p - p0, n);
}

static inline bool pointInTri(
    const glm::vec3& p,
    const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec3& c,
    const glm::vec3& n)
{
    return
        glm::dot(n, glm::cross(b - a, p - a)) >= 0 &&
        glm::dot(n, glm::cross(c - b, p - b)) >= 0 &&
        glm::dot(n, glm::cross(a - c, p - c)) >= 0;
}

// dont fall thru dec 12 2025
static void pushOutSphere(
    glm::vec3& center,
    float radius,
    const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec3& c,
    glm::vec3& outN,
    bool& outFloorHit)
{
    glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));
    if (glm::dot(n, n) < 1e-6f) return;

    // ---- FORCE NORMAL UPWARD ----
    if (n.y < 0) n = -n;

    float d = planeDist(center, n, a);

    // if sphere center is within radius (or behind plane), it's overlapping
    if (d < radius)
    {
        glm::vec3 proj = center - n * d;
        if (!pointInTri(proj, a, b, c, n)) return;

        float push = (radius - d);
        center += n * push;

        outN = n;
        if (n.y > FLOOR_DOT_MIN) outFloorHit = true;
    }
}

// -------------------------------------------------
// Sphere sweep vs triangle
// -------------------------------------------------
static bool sweepSphereTri(
    const glm::vec3& center,
    float radius,
    const glm::vec3& move,
    const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec3& c,
    float& bestT,
    glm::vec3& bestN)
{
    glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));
    if (glm::dot(n, n) < 1e-6f) return false;

    // ---- FORCE NORMAL UPWARD ----
    if (n.y < 0) n = -n;

    float d0 = planeDist(center, n, a);
    float d1 = planeDist(center + move, n, a);

    float r = radius + SKIN;
    if (d0 >= r && d1 <= r)
    {
        float t = (d0 - r) / (d0 - d1);
        if (t < bestT)
        {
            glm::vec3 hit = center + move * t;
            glm::vec3 proj = hit - n * planeDist(hit, n, a);

            if (pointInTri(proj, a, b, c, n))
            {
                bestT = t;
                bestN = n;
                return true;
            }
        }
    }
    return false;
}

// -------------------------------------------------
// Main physics update
// -------------------------------------------------
void updatePhysics(
    Player& p,
    const Mesh& world,
    GLFWwindow* win,
    float dt,
    const Camera& cam)
{
    // debug stuff bc keep falling thru floors 
    static bool printed = false;
    if (!printed) {
        fprintf(stderr, "PHYSICS TRI COUNT = %zu\n", world.verts.size() / 3);
        if (!world.verts.empty()) {
            auto& v = world.verts[0].pos;
            fprintf(stderr, "FIRST TRI VERT = (%f, %f, %f)\n", v.x, v.y, v.z);
        }
        printed = true;
    }

    // ---------------- movement input ----------------
    glm::vec3 forward(cam.front.x, 0, cam.front.z);
    if (glm::dot(forward, forward) > 0.0f)
        forward = glm::normalize(forward);

    glm::vec3 right = glm::cross(forward, glm::vec3(0,1,0));
    glm::vec3 wish(0);

    if (glfwGetKey(win, GLFW_KEY_W)) wish += forward;
    if (glfwGetKey(win, GLFW_KEY_S)) wish -= forward;
    if (glfwGetKey(win, GLFW_KEY_A)) wish -= right;
    if (glfwGetKey(win, GLFW_KEY_D)) wish += right;

    if (glm::dot(wish, wish) > 0.0f)
        wish = glm::normalize(wish);

    p.vel.x = wish.x * MOVE_SPEED;
    p.vel.z = wish.z * MOVE_SPEED;

    // ---------------- gravity / jump ----------------
    p.vel.y += GRAVITY * dt;

    if (p.onGround && glfwGetKey(win, GLFW_KEY_SPACE))
    {
        p.vel.y = JUMP_SPEED;
        p.onGround = false;
    }

    // ---------------- movement vector ----------------
    glm::vec3 move = p.vel * dt;
    glm::vec3 pos  = p.pos;

    p.onGround = false;

    // --- resolve overlap at current position (spawn safety) ---
    {
        glm::vec3 p0 = pos + glm::vec3(0, CAPSULE_RADIUS, 0);
        glm::vec3 p1 = pos + glm::vec3(0, CAPSULE_HEIGHT - CAPSULE_RADIUS, 0);

        glm::vec3 m1 = glm::mix(p0, p1, 0.25f);
        glm::vec3 m2 = glm::mix(p0, p1, 0.50f);
        glm::vec3 m3 = glm::mix(p0, p1, 0.75f);

        bool floorHit = false;
        glm::vec3 n(0);

        for (size_t i = 0; i < world.verts.size(); i += 3)
        {
            const glm::vec3& a = world.verts[i+0].pos;
            const glm::vec3& b = world.verts[i+1].pos;
            const glm::vec3& c = world.verts[i+2].pos;

            glm::vec3 triCenter = (a + b + c) * (1.0f / 3.0f);
            // ---- TRI CULL (XZ ONLY) ----
            // This keeps checking blocks below/above you.
            float dx = triCenter.x - pos.x;
            float dz = triCenter.z - pos.z;
            float dist2 = dx*dx + dz*dz;
            if (dist2 > TRI_CULL_DIST * TRI_CULL_DIST) continue;

            pushOutSphere(p0, CAPSULE_RADIUS, a,b,c, n, floorHit);
            pushOutSphere(m1, CAPSULE_RADIUS, a,b,c, n, floorHit);
            pushOutSphere(m2, CAPSULE_RADIUS, a,b,c, n, floorHit);
            pushOutSphere(m3, CAPSULE_RADIUS, a,b,c, n, floorHit);
            pushOutSphere(p1, CAPSULE_RADIUS, a,b,c, n, floorHit);
        }

        pos = p0 - glm::vec3(0, CAPSULE_RADIUS, 0);

        if (floorHit)
        {
            p.onGround = true;
            if (p.vel.y < 0) p.vel.y = 0;
        }
    }


    // ---------------- substeps (anti-tunneling) ------
    float maxStep = CAPSULE_RADIUS * 0.5f;
    int steps = glm::clamp(
        (int)glm::ceil(glm::length(move) / maxStep),
        1, 2);

    glm::vec3 stepMove = move / float(steps);

    // ---------------- sweep & slide ------------------
    for (int s = 0; s < steps; s++)
    {
        glm::vec3 step = stepMove;

        for (int iter = 0; iter < 4; iter++)
        {
            float bestT = 1.0f;
            glm::vec3 bestN(0);
            bool hit = false;

            for (size_t i = 0; i < world.verts.size(); i += 3)
            {
                const glm::vec3& a = world.verts[i+0].pos;
                const glm::vec3& b = world.verts[i+1].pos;
                const glm::vec3& c = world.verts[i+2].pos;

                glm::vec3 triCenter = (a + b + c) * (1.0f / 3.0f);
                // ---- TRI CULL (XZ ONLY) ----
                // This keeps checking blocks below/above you.
                float dx = triCenter.x - pos.x;
                float dz = triCenter.z - pos.z;
                float dist2 = dx*dx + dz*dz;
                if (dist2 > TRI_CULL_DIST * TRI_CULL_DIST) continue;

                glm::vec3 p0 = pos + glm::vec3(0, CAPSULE_RADIUS, 0);
                glm::vec3 p1 = pos + glm::vec3(0, CAPSULE_HEIGHT - CAPSULE_RADIUS, 0);

                glm::vec3 m1 = glm::mix(p0, p1, 0.25f);
                glm::vec3 m2 = glm::mix(p0, p1, 0.50f);
                glm::vec3 m3 = glm::mix(p0, p1, 0.75f);

                hit |= sweepSphereTri(p0, CAPSULE_RADIUS, step, a,b,c, bestT, bestN);
                hit |= sweepSphereTri(m1, CAPSULE_RADIUS, step, a,b,c, bestT, bestN);
                hit |= sweepSphereTri(m2, CAPSULE_RADIUS, step, a,b,c, bestT, bestN);
                hit |= sweepSphereTri(m3, CAPSULE_RADIUS, step, a,b,c, bestT, bestN);
                hit |= sweepSphereTri(p1, CAPSULE_RADIUS, step, a,b,c, bestT, bestN);
            }

            if (!hit)
            {
                pos += step;
                break;
            }

            pos += step * bestT;

            if (bestN.y > FLOOR_DOT_MIN)
            {
                p.onGround = true;
                p.vel.y = 0;
            }

            step = slide(step * (1.0f - bestT), bestN);
        }
    }

    p.pos = pos;
}

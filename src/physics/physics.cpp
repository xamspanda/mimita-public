// physics.cpp
#include "physics.h"
#include "../camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

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

    float d0 = planeDist(center, n, a);
    float d1 = planeDist(center + move, n, a);

    if (d0 > radius && d1 < radius)
    {
        float t = (d0 - radius) / (d0 - d1);
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

                glm::vec3 bottom = pos + glm::vec3(0, CAPSULE_RADIUS, 0);
                glm::vec3 top    = pos + glm::vec3(0, CAPSULE_HEIGHT - CAPSULE_RADIUS, 0);

                hit |= sweepSphereTri(bottom, CAPSULE_RADIUS, step, a,b,c, bestT, bestN);
                hit |= sweepSphereTri(top,    CAPSULE_RADIUS, step, a,b,c, bestT, bestN);
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

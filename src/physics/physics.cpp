/**
 * dec 12 2025 i dont get this so make it super super dumb
 * and add bit by bit only the pieces i can comfortably explain jus 
 * offthe ttop of my head
 * like 
 * yeah gravity pulls you down every frame dt = last frame i think
 * yeah move speed is your move speed
 * idgaf about what 30.0f is or 30f or what the f is 
 * or what voidi means or what stdio i dont want definitions i want functionality
 * if i can explain what it DOES off top my head then i get it and i can add it
 * if i dont get it dont add it
 * and that sounds slow but it avoids bloat that makes sutpid bullshit happen later 
 * and file size dont matter line count dont matter, its just do i understand it or not
 */

#include "physics.h"
#include "physics/config.h"
#include "../camera.h"
#include <glm/glm.hpp>

// ---------------- world constants ----------------
// lives in config.h never hardcode values in a file again 
// ---------------- helper functions start ----------------

// “Gives me the direction the triangle is facing.""
glm::vec3 triangleNormal(
    const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec3& c)
{
    return glm::normalize(glm::cross(b - a, c - a));
}

// “Given a point and a triangle, tell me the closest spot on the triangle.”
glm::vec3 closestPointOnTriangle(
    const glm::vec3& p,
    const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec3& c)
{
    // edges
    glm::vec3 ab = b - a;
    glm::vec3 ac = c - a;
    glm::vec3 ap = p - a;

    float d1 = glm::dot(ab, ap);
    float d2 = glm::dot(ac, ap);
    if (d1 <= 0.0f && d2 <= 0.0f) return a;

    glm::vec3 bp = p - b;
    float d3 = glm::dot(ab, bp);
    float d4 = glm::dot(ac, bp);
    if (d3 >= 0.0f && d4 <= d3) return b;

    float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
        return a + ab * (d1 / (d1 - d3));

    glm::vec3 cp = p - c;
    float d5 = glm::dot(ab, cp);
    float d6 = glm::dot(ac, cp);
    if (d6 >= 0.0f && d5 <= d6) return c;

    float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
        return a + ac * (d2 / (d2 - d6));

    float va = d3 * d6 - d5 * d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
        return b + (c - b) * ((d4 - d3) / ((d4 - d3) + (d5 - d6)));

    float denom = 1.0f / (va + vb + vc);
    float v = vb * denom;
    float w = vc * denom;
    return a + ab * v + ac * w;
}

// ---------------- helper functions end ----------------

// ---------------- main update ----------------
void updatePhysics(
    Player& p,
    const Mesh& world,     
    GLFWwindow* win,
    float dt,
    const Camera& cam)
{

    // debug stuff so i get not stuck
    // teleport up
    if (glfwGetKey(win, GLFW_KEY_T) == GLFW_PRESS)
    {
        p.pos.y += 1.0f;
        p.vel = glm::vec3(0.0f);
    }

    // teleport forward (where player is facing)
    if (glfwGetKey(win, GLFW_KEY_G) == GLFW_PRESS)
    {
        glm::vec3 dir = cam.front;
        dir.y = 0.0f; // stay horizontal
        if (glm::length(dir) > 0.0001f)
            dir = glm::normalize(dir);

        p.pos += dir * 1.0f;
        p.vel = glm::vec3(0.0f);
    }

    // player spheres
    // I KNOW its not a rectangle like roblox but just get it working first fix later
    glm::vec3 capsuleBottom = p.pos + glm::vec3(0, PLAYER_RADIUS, 0);
    glm::vec3 capsuleTop    = p.pos + glm::vec3(0,
        PLAYER_RADIUS + PLAYER_CAPSULE_HALF * 2.0f,
        0);

    // ---- movement input ----
    glm::vec3 move(0);

    // camera forward, flattened (no flying)
    glm::vec3 forward = cam.front;
    forward.y = 0;

    // if we're not looking straight down/up, make it usable
    if (glm::length(forward) > 0.0001f)
    {
        forward = glm::normalize(forward);
    }

    // camera right
    glm::vec3 right = glm::cross(forward, glm::vec3(0, 1, 0));

    if (glfwGetKey(win, GLFW_KEY_W)) move += forward;
    if (glfwGetKey(win, GLFW_KEY_S)) move -= forward;
    if (glfwGetKey(win, GLFW_KEY_A)) move -= right;
    if (glfwGetKey(win, GLFW_KEY_D)) move += right;

    p.pos += move * PHYS.moveSpeed * dt;

    // ---- jump ----
    if (glfwGetKey(win, GLFW_KEY_SPACE) && p.onGround)
    {
        p.vel.y = PHYS.jumpStrength;
        p.onGround = false;
    }

    // ---- gravity ----

    p.vel.y += PHYS.gravity * dt;
    p.pos.y += p.vel.y * dt;

    // ---- collisions ----
    p.onGround = false;

    // do a few passes so we fully escape geometry
    for (int pass = 0; pass < 2; ++pass)
    {
        for (size_t i = 0; i + 2 < world.verts.size(); i += 3)
        {
            glm::vec3 a = world.verts[i + 0].pos;
            glm::vec3 b = world.verts[i + 1].pos;
            glm::vec3 c = world.verts[i + 2].pos;

            // closest point on triangle
            glm::vec3 closest =
                closestPointOnTriangle(capsuleBottom, a, b, c);

            // project closest point onto capsule segment
            glm::vec3 ab = capsuleTop - capsuleBottom;
            float t = glm::dot(closest - capsuleBottom, ab) / glm::dot(ab, ab);
            t = glm::clamp(t, 0.0f, 1.0f);

            glm::vec3 capsulePoint = capsuleBottom + ab * t;

            glm::vec3 delta = capsulePoint - closest;
            float dist = glm::length(delta);

            if (dist < PLAYER_RADIUS && dist > 0.0001f)
            {
                glm::vec3 normal = delta / dist;
                float push = PLAYER_RADIUS - dist;

                p.pos += normal * push;

                if (normal.y > 0.5f)
                {
                    p.onGround = true;
                    if (p.vel.y < 0.0f)
                        p.vel.y = 0.0f;
                }
                else
                {
                    p.vel -= normal * glm::dot(p.vel, normal);
                }
            }
        }
    }
}
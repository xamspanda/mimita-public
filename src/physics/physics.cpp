/**
 * dec 12 2025 i dont get this so make it super super dumb
 * and add bit by bit only the pieces i can comfortably explain jus 
 * offthe ttop of my head
 * like 
 * yeah gravity pulls you down every frame
 * yeah move speed is your move speed
 * idgaf about what 30.0f is or 30f or what the f is 
 * or what voidi means or what stdio i dont want definitions i want functionality
 * if i can explain what it DOES off top my head then i get it and i can add it
 * if i dont get it dont add it
 * and that sounds slow but it avoids bloat that makes sutpid bullshit happen later 
 * and file size dont matter line count dont matter, its just do i understand it or not
 */

#include "physics.h"
#include "camera.h"
#include <glm/glm.hpp>

// ---------------- constants ----------------
const float GRAVITY     = -30.0f;
const float MOVE_SPEED = 12.0f;
const float FLOOR_Y    = 50.0f;

// ---------------- main update ----------------
void updatePhysics(
    Player& p,
    const Mesh& world,      // unused for now
    GLFWwindow* win,
    float dt,
    const Camera& cam)
{
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

    p.pos += move * MOVE_SPEED * dt;
    // ---- gravity ----

    p.vel.y += GRAVITY * dt;
    p.pos.y += p.vel.y * dt;

    // ---- floor ----
    if (p.pos.y < FLOOR_Y)
    {
        p.pos.y = FLOOR_Y;
        p.vel.y = 0;
        p.onGround = true;
    }
    else
    {
        p.onGround = false;
    }
}

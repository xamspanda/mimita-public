// C:\important\go away v5\s\mimita-v5\src\physics\physics.h

#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "entities/player.h"
#include "map/map_common.h"

void updatePhysics(Player& player, const Mesh& map, GLFWwindow* win, float dt, const Camera& cam);

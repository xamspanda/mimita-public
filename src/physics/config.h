// C:\important\go away v5\s\mimita-v5\src\physics\config.h

#pragma once

struct PhysicsConfig {
    float gravity      = -25.0f; // m/s^2
    float moveSpeed    = 20.0f;  // m/s
    float jumpStrength = 12.0f;  // m/s
};

inline PhysicsConfig PHYS;

// dec 16 2025 i think for some reason 1 meter = 0.5f so just mult by 2
// for example player height could be 1.8f but it might be 3.6f to be actual 1.8m? idk

// capsule definition
constexpr float PLAYER_HEIGHT = 1.8f;   // meters
constexpr float PLAYER_RADIUS = 0.35f;  // meters

// derived
constexpr float PLAYER_CAPSULE_HALF =
    (PLAYER_HEIGHT * 0.5f) - PLAYER_RADIUS;

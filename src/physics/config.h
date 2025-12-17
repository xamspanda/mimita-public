// C:\important\go away v5\s\mimita-v5\src\physics\config.h
// the physics.json file at 
// C:\important\quiet\n\mimita-public\mimita-public\config\physics.json
// this has hot reload values that i think are more accurate

#pragma once

struct PhysicsConfig {
    float gravity;
    float moveSpeed;
    float jumpStrength;
};

inline PhysicsConfig PHYS = {
    -25.0f,
    20.0f,
    12.0f
};

// Player dimensions (RAW FLOATS ONLY)
// i know, 1.8f = 1.8m is nice and pretty but i dont htink cpp is doing that so the
// numbers are bigger 

// defaults are
/*
0.5f for width
1.8f for height
0.2f for depth
0.35f for radius
*/

inline float PHYS_MULT = 2.0f;  
inline float PLAYER_WIDTH  = 0.5f * PHYS_MULT;  
inline float PLAYER_HEIGHT = 1.8f * PHYS_MULT;   
inline float PLAYER_DEPTH  = 0.2f * PHYS_MULT;  
inline float PLAYER_RADIUS = 0.35f * PHYS_MULT;

// do we use this idk
// i put it jsut so errors stop 

inline float MAX_SLOPE_ANGLE = 0.5f;

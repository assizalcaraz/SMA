#pragma once

#include "ofMain.h"

class Particle {
public:
    ofVec2f home;        // Posición de reposo
    ofVec2f pos;         // Posición actual
    ofVec2f vel;         // Velocidad
    float mass;          // Masa (default 1.0)
    int id;             // Identificador único
    float lastHitTime;  // Para cooldown (no usado en esta fase)
    
    Particle();
    Particle(int _id, ofVec2f _home);
    
    void update(float dt, float k_home, float k_drag);
    void reset();
};

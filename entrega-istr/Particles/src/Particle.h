#pragma once

#include "ofMain.h"

class Particle {
public:
    ofVec2f home;        // Posición de reposo
    ofVec2f pos;         // Posición actual
    ofVec2f vel;         // Velocidad
    float mass;          // Masa (default 1.0)
    int id;             // Identificador único
    float lastHitTime;  // Para cooldown
    ofVec2f vel_pre;    // Velocidad PRE-colisión (para cálculo de energía)
    float last_hit_distance;  // Distancia recorrida desde último hit
    int last_surface;    // Última superficie impactada (para detectar scrapes)
    
    Particle();
    Particle(int _id, ofVec2f _home);
    
    void update(float dt, float k_home, float k_drag);
    void bounce(int surface, float restitution, float width, float height);
    void reset();
};

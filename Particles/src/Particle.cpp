#include "Particle.h"

//--------------------------------------------------------------
Particle::Particle() {
    home = ofVec2f(0, 0);
    pos = ofVec2f(0, 0);
    vel = ofVec2f(0, 0);
    mass = 1.0f;
    id = 0;
    lastHitTime = 0.0f;
}

//--------------------------------------------------------------
Particle::Particle(int _id, ofVec2f _home) {
    id = _id;
    home = _home;
    pos = _home;
    vel = ofVec2f(0, 0);
    mass = 1.0f;
    lastHitTime = 0.0f;
}

//--------------------------------------------------------------
void Particle::update(float dt, float k_home, float k_drag) {
    // Fuerza de retorno al origen (F_home)
    ofVec2f F_home = k_home * (home - pos);
    
    // Fuerza de drag (F_drag)
    ofVec2f F_drag = -k_drag * vel;
    
    // Fuerza total
    ofVec2f F = F_home + F_drag;
    
    // Integración semi-implícita Euler
    vel += (F / mass) * dt;
    pos += vel * dt;
}

//--------------------------------------------------------------
void Particle::reset() {
    pos = home;
    vel = ofVec2f(0, 0);
    lastHitTime = 0.0f;
}

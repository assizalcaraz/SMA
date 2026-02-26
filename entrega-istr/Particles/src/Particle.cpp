#include "Particle.h"

//--------------------------------------------------------------
Particle::Particle() {
    home = ofVec2f(0, 0);
    pos = ofVec2f(0, 0);
    vel = ofVec2f(0, 0);
    mass = 1.0f;
    id = 0;
    lastHitTime = 0.0f;
    vel_pre = ofVec2f(0, 0);
    last_hit_distance = 0.0f;
    last_surface = -1;
}

//--------------------------------------------------------------
Particle::Particle(int _id, ofVec2f _home) {
    id = _id;
    home = _home;
    pos = _home;
    vel = ofVec2f(0, 0);
    mass = 1.0f;
    lastHitTime = 0.0f;
    vel_pre = ofVec2f(0, 0);
    last_hit_distance = 0.0f;
    last_surface = -1;
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
void Particle::bounce(int surface, float restitution, float width, float height) {
    // Aplicar rebote según superficie
    if (surface == 0 || surface == 1) {
        // Bordes horizontales (L/R)
        vel.x *= -restitution;
        pos.x = ofClamp(pos.x, 0.0f, width);
    } else if (surface == 2 || surface == 3) {
        // Bordes verticales (T/B)
        vel.y *= -restitution;
        pos.y = ofClamp(pos.y, 0.0f, height);
    }
}

//--------------------------------------------------------------
void Particle::reset() {
    pos = home;
    vel = ofVec2f(0, 0);
    lastHitTime = 0.0f;
    vel_pre = ofVec2f(0, 0);
    last_hit_distance = 0.0f;
    last_surface = -1;
}

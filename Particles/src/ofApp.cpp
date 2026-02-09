#include "ofApp.h"
#include <sstream>

//--------------------------------------------------------------
void ofApp::setup(){
    // Configurar parámetros iniciales
    // NOTA: Valores calibrados para facilitar que partículas lleguen a bordes con gestos normales
    // - k_gesture alto: más fuerza de gesto para empujar partículas
    // - k_drag bajo: menos fricción para que partículas mantengan movimiento
    // - k_home bajo: menos fuerza de retorno para permitir que partículas se alejen más
    k_home = 1.0f;           // Reducido de 2.0 para permitir más movimiento libre
    k_drag = 0.5f;           // Reducido de 1.0 para menos fricción
    k_gesture = 150.0f;      // Aumentado de 50.0 para más fuerza de gesto
    sigma = 200.0f;          // Aumentado de 150.0 para mayor área de influencia
    speed_ref = 300.0f;      // Reducido de 500.0 para que gestos normales se consideren más rápidos
    smooth_alpha = 0.15f;    // Factor de suavizado
    int initialN = 2000;
    
    // Parámetros de colisiones
    restitution = 0.6f;      // Coeficiente de restitución (0.2-0.85)
    hit_cooldown_ms = 60.0f; // Cooldown por partícula en ms (30-120)
    
    // Parámetros de energía
    // NOTA: Valores ajustados para hacer más factible y frecuente que partículas lleguen al margen
    vel_ref = 300.0f;        // Reducido de 500.0 para que velocidades normales generen más energía
    dist_ref = 40.0f;        // Reducido de 50.0 para que distancias menores generen más energía
    energy_a = 0.7f;         // Peso de velocidad en energía (0.5-0.9)
    energy_b = 0.3f;         // Peso de distancia en energía (0.1-0.5)
    
    // Parámetros de rate limiting
    max_hits_per_second = 200.0f;  // Máximo de hits por segundo (50-500)
    burst = 300.0f;                // Burst máximo (100-500)
    max_hits_per_frame = 10;        // Máximo de hits por frame (5-20)
    
    // Inicializar rate limiter
    rate_limiter.tokens = burst;
    rate_limiter.rate = max_hits_per_second;
    rate_limiter.burst = burst;
    rate_limiter.max_per_frame = max_hits_per_frame;
    rate_limiter.hits_this_frame = 0;
    
    // Inicializar contadores de debug
    hits_per_second = 0.0f;
    hits_discarded_rate = 0;
    hits_discarded_cooldown = 0;
    hits_this_second = 0;
    time_accumulator = 0.0f;
    
    // Inicializar mouse
    mouse.pos = ofVec2f(0.5f, 0.5f);
    mouse.pos_prev = mouse.pos;
    mouse.pos_smooth = mouse.pos;
    mouse.vel = ofVec2f(0, 0);
    mouse.active = false;
    
    // Configurar GUI
    gui.setup("Parameters");
    gui.add(nParticlesSlider.setup("N Particles", initialN, 500, 8000));
    gui.add(kHomeSlider.setup("k_home", k_home, 0.5f, 6.0f));
    gui.add(kDragSlider.setup("k_drag", k_drag, 0.5f, 3.0f));
    gui.add(kGestureSlider.setup("k_gesture", k_gesture, 0.0f, 200.0f));
    gui.add(sigmaSlider.setup("sigma", sigma, 50.0f, 500.0f));
    gui.add(speedRefSlider.setup("speed_ref", speed_ref, 100.0f, 2000.0f));
    
    // Sliders de colisiones
    gui.add(restitutionSlider.setup("restitution", restitution, 0.2f, 0.85f));
    gui.add(hitCooldownSlider.setup("hit_cooldown (ms)", hit_cooldown_ms, 30.0f, 120.0f));
    
    // Sliders de energía
    gui.add(velRefSlider.setup("vel_ref", vel_ref, 300.0f, 1000.0f));
    gui.add(distRefSlider.setup("dist_ref", dist_ref, 20.0f, 100.0f));
    gui.add(energyASlider.setup("energy_a", energy_a, 0.5f, 0.9f));
    gui.add(energyBSlider.setup("energy_b", energy_b, 0.1f, 0.5f));
    
    // Sliders de rate limiting
    gui.add(maxHitsPerSecondSlider.setup("max_hits/s", max_hits_per_second, 50.0f, 500.0f));
    gui.add(burstSlider.setup("burst", burst, 100.0f, 500.0f));
    gui.add(maxHitsPerFrameSlider.setup("max_hits/frame", max_hits_per_frame, 5, 20));
    
    // Inicializar partículas
    initializeParticles(initialN);
}

//--------------------------------------------------------------
void ofApp::update(){
    // Actualizar parámetros desde sliders
    k_home = kHomeSlider;
    k_drag = kDragSlider;
    k_gesture = kGestureSlider;
    sigma = sigmaSlider;
    speed_ref = speedRefSlider;
    
    // Actualizar parámetros de colisiones
    restitution = restitutionSlider;
    hit_cooldown_ms = hitCooldownSlider;
    
    // Actualizar parámetros de energía
    vel_ref = velRefSlider;
    dist_ref = distRefSlider;
    energy_a = energyASlider;
    energy_b = energyBSlider;
    
    // Actualizar parámetros de rate limiting
    max_hits_per_second = maxHitsPerSecondSlider;
    burst = burstSlider;
    max_hits_per_frame = maxHitsPerFrameSlider;
    rate_limiter.rate = max_hits_per_second;
    rate_limiter.burst = burst;
    rate_limiter.max_per_frame = max_hits_per_frame;
    
    // Detectar cambio en número de partículas
    int targetN = nParticlesSlider;
    if (targetN != (int)particles.size()) {
        resizeParticles(targetN);
    }
    
    // Actualizar input del mouse
    updateMouseInput();
    
    // Aplicar fuerza de gesto a las partículas
    applyGestureForce();
    
    // Actualizar física de partículas
    float dt = ofGetLastFrameTime();
    if (dt <= 0.0f) dt = 0.016f; // Fallback a 60fps
    
    // Actualizar tracking de distancia recorrida
    for (auto& p : particles) {
        p.last_hit_distance += p.vel.length() * dt;
    }
    
    // Actualizar física de partículas
    for (auto& p : particles) {
        p.update(dt, k_home, k_drag);
    }
    
    // Limpiar eventos del frame anterior
    pending_hits.clear();
    validated_hits.clear();
    
    // Actualizar rate limiter
    updateRateLimiter(dt);
    
    // Detectar y manejar colisiones
    checkCollisions();
    
    // Procesar eventos pendientes con rate limiting
    processPendingHits();
    
    // Actualizar contadores de debug
    time_accumulator += dt;
    if (time_accumulator >= 1.0f) {
        hits_per_second = (float)hits_this_second;
        hits_this_second = 0;
        time_accumulator = 0.0f;
        // Resetear contadores de hits descartados cada segundo para mostrar tasa actual
        hits_discarded_rate = 0;
        hits_discarded_cooldown = 0;
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    // Fondo oscuro
    ofBackground(10, 10, 15);
    
    // Render de partículas como puntos
    ofSetColor(255, 255, 255); // Blanco/azulado metálico
    glPointSize(2.0f);
    glEnable(GL_POINT_SMOOTH);
    glBegin(GL_POINTS);
    for (const auto& p : particles) {
        glVertex2f(p.pos.x, p.pos.y);
    }
    glEnd();
    glDisable(GL_POINT_SMOOTH);
    
    // Debug overlay
    drawDebugOverlay();
    
    // GUI
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::exit(){

}

//--------------------------------------------------------------
void ofApp::initializeParticles(int n) {
    particles.clear();
    particles.reserve(n);
    
    // Calcular grid
    int cols = (int)sqrt(n);
    int rows = (int)ceil((float)n / cols);
    
    float margin = 50.0f;
    // Usar dimensiones de ventana o valores por defecto si no está lista
    float winWidth = ofGetWidth() > 0 ? ofGetWidth() : 1024.0f;
    float winHeight = ofGetHeight() > 0 ? ofGetHeight() : 768.0f;
    float width = winWidth - 2 * margin;
    float height = winHeight - 2 * margin;
    
    float cellWidth = width / cols;
    float cellHeight = height / rows;
    
    // Crear partículas con grid + jitter
    int id = 0;
    for (int i = 0; i < rows && id < n; i++) {
        for (int j = 0; j < cols && id < n; j++) {
            // Posición base del grid
            float baseX = margin + j * cellWidth + cellWidth * 0.5f;
            float baseY = margin + i * cellHeight + cellHeight * 0.5f;
            
            // Aplicar jitter aleatorio
            float jitterX = ofRandom(-cellWidth * 0.3f, cellWidth * 0.3f);
            float jitterY = ofRandom(-cellHeight * 0.3f, cellHeight * 0.3f);
            
            ofVec2f homePos(baseX + jitterX, baseY + jitterY);
            
            // Asegurar que esté dentro de los límites
            homePos.x = ofClamp(homePos.x, margin, winWidth - margin);
            homePos.y = ofClamp(homePos.y, margin, winHeight - margin);
            
            particles.push_back(Particle(id, homePos));
            id++;
        }
    }
}

//--------------------------------------------------------------
void ofApp::resizeParticles(int newN) {
    // Reinicializar todas las partículas para mantener distribución grid+jitter consistente
    // Esto asegura que el grid se recalcule correctamente para cualquier N
    initializeParticles(newN);
}

//--------------------------------------------------------------
void ofApp::updateMouseInput() {
    float dt = ofGetLastFrameTime();
    if (dt <= 0.0f) dt = 0.016f; // Fallback a 60fps
    
    // Obtener posición del mouse en pixels
    int mouseX = ofGetMouseX();
    int mouseY = ofGetMouseY();
    
    // Normalizar posición (0..1)
    float winWidth = ofGetWidth();
    float winHeight = ofGetHeight();
    
    if (winWidth > 0 && winHeight > 0) {
        mouse.pos.x = ofClamp((float)mouseX / winWidth, 0.0f, 1.0f);
        mouse.pos.y = ofClamp((float)mouseY / winHeight, 0.0f, 1.0f);
        
        // Suavizado con lerp
        mouse.pos_smooth = mouse.pos_smooth * (1.0f - smooth_alpha) + mouse.pos * smooth_alpha;
        
        // Calcular velocidad (en pixels/s)
        ofVec2f pos_pixels = ofVec2f(mouse.pos_smooth.x * winWidth, mouse.pos_smooth.y * winHeight);
        ofVec2f pos_prev_pixels = ofVec2f(mouse.pos_prev.x * winWidth, mouse.pos_prev.y * winHeight);
        mouse.vel = (pos_pixels - pos_prev_pixels) / dt;
        
        // Actualizar posición anterior
        mouse.pos_prev = mouse.pos_smooth;
        
        // Mouse activo si está dentro de la ventana
        mouse.active = (mouseX >= 0 && mouseX < winWidth && mouseY >= 0 && mouseY < winHeight);
    }
}

//--------------------------------------------------------------
void ofApp::applyGestureForce() {
    if (!mouse.active || mouse.vel.length() < 1.0f) {
        return; // Mouse inactivo o sin movimiento
    }
    
    float winWidth = ofGetWidth();
    float winHeight = ofGetHeight();
    
    // Posición del mouse en pixels
    ofVec2f mousePosPixels = ofVec2f(mouse.pos_smooth.x * winWidth, mouse.pos_smooth.y * winHeight);
    
    // Normalizar velocidad
    float vel_magnitude = mouse.vel.length();
    
    // Velocidad normalizada (0..1)
    float speed = ofClamp(vel_magnitude / speed_ref, 0.0f, 1.0f);
    
    // Aplicar fuerza a cada partícula
    float dt = ofGetLastFrameTime();
    if (dt <= 0.0f) dt = 0.016f;
    
    for (auto& p : particles) {
        // Distancia desde partícula al mouse (en pixels)
        ofVec2f particlePosPixels = ofVec2f(p.pos.x, p.pos.y);
        ofVec2f diff = particlePosPixels - mousePosPixels;
        float r = diff.length();
        
        // Calcular dirección radial desde mouse hacia partícula (push radial)
        // Esto empuja las partículas alejándolas del mouse
        ofVec2f push_dir;
        if (r > 0.001f) {
            push_dir = diff.normalized(); // Dirección desde mouse hacia partícula
        } else {
            continue; // Partícula exactamente en el mouse, saltar
        }
        
        // Influencia gaussiana por distancia: w = exp(-(r²)/(2*sigma²))
        // Usa sigma como radio de influencia (más grande = más alcance)
        float w = exp(-(r * r) / (2.0f * sigma * sigma));
        
        // Solo aplicar fuerza si la influencia es significativa (evitar cálculos innecesarios)
        if (w < 0.01f) {
            continue; // Influencia demasiado pequeña, saltar
        }
        
        // Fuerza de gesto: push radial que empuja partículas alejándolas del mouse
        // La fuerza es proporcional a la velocidad del mouse y la cercanía (gaussiana)
        ofVec2f F_gesture = k_gesture * w * speed * push_dir;
        
        // Aplicar fuerza directamente a la velocidad (impulso)
        p.vel += (F_gesture / p.mass) * dt;
    }
}

//--------------------------------------------------------------
void ofApp::checkCollisions() {
    float width = ofGetWidth();
    float height = ofGetHeight();
    
    for (auto& p : particles) {
        int surface = -1;
        bool collided = false;
        
        // Guardar velocidad PRE-colisión
        p.vel_pre = p.vel;
        
        // Detectar colisión con bordes
        if (p.pos.x < 0.0f) {
            surface = 0; // Borde izquierdo
            collided = true;
        } else if (p.pos.x > width) {
            surface = 1; // Borde derecho
            collided = true;
        } else if (p.pos.y < 0.0f) {
            surface = 2; // Borde superior
            collided = true;
        } else if (p.pos.y > height) {
            surface = 3; // Borde inferior
            collided = true;
        }
        
        if (collided) {
            // Aplicar rebote físico
            p.bounce(surface, restitution, width, height);
            
            // Generar evento de hit
            generateHitEvent(p, surface);
        }
    }
}

//--------------------------------------------------------------
float ofApp::calculateHitEnergy(Particle& p, int surface) {
    // Velocidad normalizada: speed_norm = |vel_pre| / vel_ref
    float speed_norm = ofClamp(p.vel_pre.length() / vel_ref, 0.0f, 1.0f);
    
    // Distancia normalizada: dist_norm = last_hit_distance / dist_ref
    float dist_norm = ofClamp(p.last_hit_distance / dist_ref, 0.0f, 1.0f);
    
    // Energía combinada: energy = clamp(a * speed_norm + b * dist_norm, 0..1)
    float energy = ofClamp(energy_a * speed_norm + energy_b * dist_norm, 0.0f, 1.0f);
    
    // Validación mínima: descartar ruido numérico extremo
    if (energy < 0.001f) {
        return 0.0f;
    }
    
    return energy;
}

//--------------------------------------------------------------
void ofApp::generateHitEvent(Particle& p, int surface) {
    float timeNow = ofGetElapsedTimef();
    
    // Verificar cooldown por partícula
    float timeSinceLastHit = timeNow - p.lastHitTime;
    float cooldown_seconds = hit_cooldown_ms / 1000.0f;
    
    if (timeSinceLastHit < cooldown_seconds) {
        hits_discarded_cooldown++;
        return; // Partícula en cooldown
    }
    
    // Calcular energía del impacto
    float energy = calculateHitEnergy(p, surface);
    
    if (energy <= 0.0f) {
        return; // Energía demasiado baja (ruido numérico)
    }
    
    // Crear evento de hit
    HitEvent event;
    event.id = p.id;
    event.x = ofClamp(p.pos.x / ofGetWidth(), 0.0f, 1.0f);
    event.y = ofClamp(p.pos.y / ofGetHeight(), 0.0f, 1.0f);
    event.energy = energy;
    event.surface = surface;
    
    // Agregar a eventos pendientes
    pending_hits.push_back(event);
    
    // Actualizar estado de partícula
    p.lastHitTime = timeNow;
    p.last_hit_distance = 0.0f;
    p.last_surface = surface;
}

//--------------------------------------------------------------
void ofApp::updateRateLimiter(float dt) {
    // Token bucket: tokens += rate * dt
    rate_limiter.tokens += rate_limiter.rate * dt;
    
    // Limitar tokens al burst máximo
    rate_limiter.tokens = ofMin(rate_limiter.tokens, rate_limiter.burst);
    
    // Resetear contador de hits este frame
    rate_limiter.hits_this_frame = 0;
}

//--------------------------------------------------------------
bool ofApp::canEmitHit() {
    // Verificar que hay tokens disponibles
    if (rate_limiter.tokens < 1.0f) {
        return false;
    }
    
    // Verificar límite por frame
    if (rate_limiter.hits_this_frame >= rate_limiter.max_per_frame) {
        return false;
    }
    
    return true;
}

//--------------------------------------------------------------
void ofApp::consumeToken() {
    rate_limiter.tokens -= 1.0f;
    rate_limiter.hits_this_frame++;
}

//--------------------------------------------------------------
void ofApp::processPendingHits() {
    for (const auto& event : pending_hits) {
        if (canEmitHit()) {
            // Evento válido, agregar a validated_hits
            validated_hits.push_back(event);
            consumeToken();
            hits_this_second++;
        } else {
            // Evento descartado por rate limiting
            hits_discarded_rate++;
        }
    }
}

//--------------------------------------------------------------
void ofApp::drawDebugOverlay() {
    ofSetColor(255, 255, 255);
    stringstream ss;
    ss << "FPS: " << ofGetFrameRate() << endl;
    ss << "Particles: " << particles.size() << endl;
    ss << "k_home: " << k_home << endl;
    ss << "k_drag: " << k_drag << endl;
    ss << "k_gesture: " << k_gesture << endl;
    ss << "Mouse vel: " << mouse.vel.length() << " px/s" << endl;
    ss << "---" << endl;
    ss << "Hits/sec: " << hits_per_second << endl;
    ss << "Discarded (rate): " << hits_discarded_rate << endl;
    ss << "Discarded (cooldown): " << hits_discarded_cooldown << endl;
    ss << "Tokens: " << rate_limiter.tokens << endl;
    ss << "Pending: " << pending_hits.size() << endl;
    ss << "Validated: " << validated_hits.size() << endl;
    
    // Dibujar en la parte inferior izquierda para no superponerse con la GUI
    // La GUI típicamente está en la esquina superior derecha
    float x = 20.0f;
    float lineHeight = 15.0f; // Altura aproximada de cada línea
    int lineCount = 13; // Número de líneas en el overlay
    float y = ofGetHeight() - (lineCount * lineHeight) - 20.0f;
    
    // Asegurar que no se salga de la pantalla
    if (y < 20.0f) {
        y = 20.0f;
    }
    
    ofDrawBitmapString(ss.str(), x, y);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    // Recalcular posiciones home cuando cambia el tamaño de ventana
    if (particles.size() > 0) {
        initializeParticles(particles.size());
    }
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

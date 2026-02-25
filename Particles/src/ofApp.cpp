#include "ofApp.h"
#include <sstream>
#include <cmath>
#include <algorithm>

const float ofApp::REST_SPEED_EPSILON_FACTOR = 0.01f;
static const float ENERGY_FLOOR = 0.01f;  // Suelo perceptible; descartes por debajo (Fase 2)

//--------------------------------------------------------------
bool ofApp::isExternalForceActive() const {
    return (mouse.active && mouse.vel.length() >= 1.0f) || (plateAmp >= 0.01f);
}

//--------------------------------------------------------------
void ofApp::setup(){
    // Inicializar contadores de debug
    particles_rendered_this_frame = 0;
    particleSize = 2.0f;  // Tamaño inicial
    
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
    particleSize = 2.0f;      // Tamaño inicial de partículas (píxeles)
    cameraZoom = 1.0f;        // Sin zoom inicial
    cameraRotation = 0.0f;    // Sin rotación inicial
    int initialN = 2000;
    
    // Parámetros de colisiones
    restitution = 0.6f;      // Coeficiente de restitución (0.2-0.85)
    hit_cooldown_ms = 40.0f; // Cooldown por partícula en ms (20-120) - reducido para más densidad
    particle_radius = 5.0f;  // Radio de colisión entre partículas (píxeles)
    enable_particle_collisions = true;  // Habilitar colisiones partícula-partícula por defecto
    
    // Parámetros de energía
    // NOTA: Valores ajustados para hacer más factible y frecuente que partículas lleguen al margen
    vel_ref = 300.0f;        // Reducido de 500.0 para que velocidades normales generen más energía
    dist_ref = 40.0f;        // Reducido de 50.0 para que distancias menores generen más energía
    energy_a = 0.7f;         // Peso de velocidad en energía (0.5-0.9)
    energy_b = 0.3f;         // Peso de distancia en energía (0.1-0.5)
    
    // Parámetros de rate limiting
    max_hits_per_second = 800.0f;
    burst = 1000.0f;
    max_hits_per_frame = 50;
    max_hits_border_per_second = 200.0f;   // Fase 4: borde más estricto que p2p
    max_hits_pp_per_second = 600.0f;

    // Inicializar rate limiter global (solo max_per_frame y hits_this_frame)
    rate_limiter.tokens = burst;
    rate_limiter.rate = max_hits_per_second;
    rate_limiter.burst = burst;
    rate_limiter.max_per_frame = max_hits_per_frame;
    rate_limiter.hits_this_frame = 0;

    rate_limiter_border.tokens = burst;
    rate_limiter_border.rate = max_hits_border_per_second;
    rate_limiter_border.burst = burst;
    rate_limiter_border.max_per_frame = 0;
    rate_limiter_border.hits_this_frame = 0;

    rate_limiter_pp.tokens = burst;
    rate_limiter_pp.rate = max_hits_pp_per_second;
    rate_limiter_pp.burst = burst;
    rate_limiter_pp.max_per_frame = 0;
    rate_limiter_pp.hits_this_frame = 0;
    
    // Inicializar contadores de debug
    hits_per_second = 0.0f;
    hits_discarded_rate = 0;
    hits_discarded_cooldown = 0;
    hits_this_second = 0;
    time_accumulator = 0.0f;
    hits_candidate_border = 0;
    hits_candidate_p2p = 0;
    hits_added_pending = 0;
    hits_validated = 0;
    hits_sent_osc = 0;
    hits_discarded_low_energy = 0;
    validated_this_frame = 0;
    sent_this_frame = 0;
    dropped_rate_this_frame = 0;

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
    gui.add(hitCooldownSlider.setup("hit_cooldown (ms)", hit_cooldown_ms, 20.0f, 120.0f));
    gui.add(particleRadiusSlider.setup("particle_radius", particle_radius, 2.0f, 20.0f));
    gui.add(enableParticleCollisionsToggle.setup("enable_particle_collisions", enable_particle_collisions));
    
    // Sliders de energía
    gui.add(velRefSlider.setup("vel_ref", vel_ref, 300.0f, 1000.0f));
    gui.add(distRefSlider.setup("dist_ref", dist_ref, 20.0f, 100.0f));
    gui.add(energyASlider.setup("energy_a", energy_a, 0.5f, 0.9f));
    gui.add(energyBSlider.setup("energy_b", energy_b, 0.1f, 0.5f));
    
    // Sliders de rate limiting
    gui.add(maxHitsPerSecondSlider.setup("max_hits/s", max_hits_per_second, 50.0f, 1000.0f));
    gui.add(burstSlider.setup("burst", burst, 100.0f, 1000.0f));
    gui.add(maxHitsPerFrameSlider.setup("max_hits/frame", max_hits_per_frame, 5, 50));
    
    // Slider de tamaño de partículas
    gui.add(particleSizeSlider.setup("particle_size", particleSize, 1.0f, 10.0f));
    
    // Sliders de cámara
    gui.add(cameraZoomSlider.setup("camera_zoom", cameraZoom, 0.1f, 5.0f));
    gui.add(cameraRotationSlider.setup("camera_rotation", cameraRotation, -180.0f, 180.0f));
    
    // Plate Controller sliders
    plateFreq = 440.0f;  // Frecuencia inicial (A4)
    plateAmp = 0.0f;    // Amplitud inicial (silencio)
    plateMode = 0;      // Modo inicial
    plateSendInterval = 0.05f;  // 20 Hz
    plateSendTimer = 0.0f;
    plateForceStrength = 50.0f;  // Intensidad de fuerza del plate (ajustable)
    gui.add(plateFreqSlider.setup("plate_freq (Hz)", plateFreq, 20.0f, 2000.0f));
    gui.add(plateAmpSlider.setup("plate_amp", plateAmp, 0.0f, 1.0f));
    gui.add(plateModeSlider.setup("plate_mode", plateMode, 0, 7));
    
    // v0.3: Inicializar Chladni State
    chladniState = false;
    k_home_previous = k_home;  // Guardar valor inicial
    plateShakerStrength = 30.0f;  // Constante para fuerza del shaker

    // Campo plate precomputado (plan 4.1)
    gridPlateW = 128;
    gridPlateH = 128;
    plateDirty = true;
    lastPlateRebuildTime = 0.0f;
    lastBuiltPlateMode = -1;
    lastBuiltPlateWidth = 0.0f;
    lastBuiltPlateHeight = 0.0f;
    update_total_ms = 0.0f;
    p2p_collision_ms = 0.0f;
    plate_force_ms = 0.0f;
    draw_ms = 0.0f;
    
    // Inicializar partículas
    initializeParticles(initialN);
    
    // VBO preasignado para partículas (plan 4.2)
    particlesMesh.setMode(OF_PRIMITIVE_POINTS);
    particlesMesh.getVertices().resize(kMaxParticles);
    for (int i = 0; i < kMaxParticles; i++) {
        particlesMesh.getVertices()[i] = glm::vec3(0, 0, 0);
    }
    particlesMesh.getVbo().setVertexData(particlesMesh.getVertices().data(), kMaxParticles, GL_DYNAMIC_DRAW);
    
    if (!pointsShader.load("points.vert", "points.frag")) {
        ofLogWarning("ofApp") << "Points shader not loaded; point size may use legacy glPointSize";
    }
    
    // Configurar OSC
    setupOSC();
}

//--------------------------------------------------------------
void ofApp::update(){
    // dt centralizado: una sola fuente por frame, clamp para estabilidad
    const float kDtMax = 1.0f / 30.0f;
    float raw_dt = ofGetLastFrameTime();
    if (raw_dt <= 0.0f) raw_dt = 0.016f;
    dt_sec = (raw_dt > kDtMax) ? kDtMax : raw_dt;

    float t_update_start = ofGetElapsedTimef();

    // Actualizar parámetros desde sliders
    // v0.3: Chladni State logic - manejar k_home según estado
    if (chladniState) {
        // Chladni ON: forzar k_home a 0 (ignorar slider)
        // NO guardar k_home_previous aquí (se guarda solo en keyPressed() al activar)
        k_home = 0.01f;  // Valor muy bajo para estabilidad numérica
    } else {
        // Chladni OFF: usar valor del slider normalmente (comportamiento v0.2)
        k_home = kHomeSlider;
    }
    k_drag = kDragSlider;
    k_gesture = kGestureSlider;
    sigma = sigmaSlider;
    speed_ref = speedRefSlider;
    
    // Actualizar parámetros de colisiones
    restitution = restitutionSlider;
    hit_cooldown_ms = hitCooldownSlider;
    particle_radius = particleRadiusSlider;
    enable_particle_collisions = enableParticleCollisionsToggle;
    
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
    rate_limiter_border.rate = max_hits_border_per_second;
    rate_limiter_border.burst = burst;
    rate_limiter_pp.rate = max_hits_pp_per_second;
    rate_limiter_pp.burst = burst;

    // Actualizar tamaño de partículas
    particleSize = particleSizeSlider;
    
    // Actualizar parámetros de cámara
    cameraZoom = cameraZoomSlider;
    cameraRotation = cameraRotationSlider;
    
    // Actualizar parámetros de Plate Controller
    plateFreq = plateFreqSlider;
    plateAmp = plateAmpSlider;
    plateMode = plateModeSlider;
    if (plateMode != lastBuiltPlateMode || ofGetWidth() != lastBuiltPlateWidth || ofGetHeight() != lastBuiltPlateHeight) {
        plateDirty = true;
    }
    
    // Detectar cambio en número de partículas
    int targetN = nParticlesSlider;
    if (targetN != (int)particles.size()) {
        resizeParticles(targetN);
    }
    
    // Actualizar input del mouse
    updateMouseInput();
    
    // Aplicar fuerza de gesto a las partículas
    applyGestureForce();
    
    float t_plate_start = ofGetElapsedTimef();
    // Aplicar fuerza del Plate Controller a las partículas
    applyPlateForce();
    plate_force_ms = (ofGetElapsedTimef() - t_plate_start) * 1000.0f;
    
    // Actualizar física de partículas (usa dt_sec centralizado)
    // Actualizar tracking de distancia recorrida
    for (auto& p : particles) {
        p.last_hit_distance += p.vel.length() * dt_sec;
    }
    
    // Actualizar física de partículas
    for (auto& p : particles) {
        p.update(dt_sec, k_home, k_drag);
    }
    
    // Limpiar eventos del frame anterior
    pending_hits.clear();
    validated_hits.clear();
    validated_this_frame = 0;
    sent_this_frame = 0;
    dropped_rate_this_frame = 0;

    // Actualizar rate limiter (refill antes de selección/consumo)
    updateRateLimiter(dt_sec);
    
    // Detectar y manejar colisiones
    checkCollisions();
    
    float t_p2p_start = ofGetElapsedTimef();
    // Detectar colisiones entre partículas (si está habilitado)
    if (enable_particle_collisions) {
        checkParticleCollisions();
    } else {
        narrow_phase_pairs_checked = 0;
        collisions_resolved = 0;
    }
    p2p_collision_ms = (ofGetElapsedTimef() - t_p2p_start) * 1000.0f;

    // Fase 3: priorizar por energía (O(n) selección, sin sort completo)
    static const size_t max_pending_cap = 500;
    auto greater_energy = [](const HitEvent& a, const HitEvent& b) { return a.energy > b.energy; };
    if (pending_hits.size() > max_pending_cap) {
        std::nth_element(pending_hits.begin(), pending_hits.begin() + (max_pending_cap - 1), pending_hits.end(), greater_energy);
        pending_hits.resize(max_pending_cap);
    }
    if (pending_hits.size() > (size_t)rate_limiter.max_per_frame) {
        std::nth_element(pending_hits.begin(), pending_hits.begin() + (rate_limiter.max_per_frame - 1), pending_hits.end(), greater_energy);
        std::partial_sort(pending_hits.begin(), pending_hits.begin() + rate_limiter.max_per_frame, pending_hits.end(), greater_energy);
    }

    // Procesar eventos pendientes con rate limiting
    processPendingHits();
    
    // Enviar eventos OSC validados (hits_sent_osc solo al enviar realmente)
    if (oscEnabled) {
        for (const auto& event : validated_hits) {
            sendHitEvent(event);
            hits_sent_osc++;
            sent_this_frame++;
        }
        
        // Enviar mensaje /state periódicamente (10 Hz durante actividad)
        stateSendTimer += dt_sec;
        if (stateSendTimer >= stateSendInterval) {
            sendStateMessage();
            stateSendTimer = 0.0f;
        }
        
        // Enviar mensaje /plate con rate limiting (20-30 Hz)
        plateSendTimer += dt_sec;
        if (plateSendTimer >= plateSendInterval) {
            sendPlateMessage();
            plateSendTimer = 0.0f;
        }
    }
    
    // Actualizar contadores de debug
    time_accumulator += dt_sec;
    if (time_accumulator >= 1.0f) {
        hits_per_second = (float)hits_this_second;
        hits_this_second = 0;
        time_accumulator = 0.0f;
        // Resetear contadores de hits descartados y audit cada segundo para mostrar tasa actual
        hits_discarded_rate = 0;
        hits_discarded_cooldown = 0;
        hits_candidate_border = 0;
        hits_candidate_p2p = 0;
        hits_added_pending = 0;
        hits_validated = 0;
        hits_sent_osc = 0;
        hits_discarded_low_energy = 0;
    }
    update_total_ms = (ofGetElapsedTimef() - t_update_start) * 1000.0f;
}

//--------------------------------------------------------------
void ofApp::draw(){
    float t_draw_start = ofGetElapsedTimef();
    // Fondo oscuro
    ofBackground(10, 10, 15);
    
    // Aplicar transformaciones de cámara
    ofPushMatrix();
    float centerX = ofGetWidth() / 2.0f;
    float centerY = ofGetHeight() / 2.0f;
    ofTranslate(centerX, centerY);
    ofScale(cameraZoom, cameraZoom);
    ofRotateDeg(cameraRotation);
    ofTranslate(-centerX, -centerY);
    
    // Actualizar posiciones en el VBO (plan 4.2: updateVertexData, sin clear+addVertex)
    size_t n = particles.size();
    if (n > (size_t)kMaxParticles) n = (size_t)kMaxParticles;
    for (size_t i = 0; i < n; i++) {
        particlesMesh.getVertices()[i] = glm::vec3(particles[i].pos.x, particles[i].pos.y, 0.0f);
    }
    if (n > 0) {
        particlesMesh.getVbo().updateVertexData(particlesMesh.getVertices().data(), (int)n);
    }
    
    ofSetColor(255, 255, 255);
    if (pointsShader.isLoaded()) {
        pointsShader.begin();
        pointsShader.setUniform1f("uPointSize", particleSize);
        // MVP = projection * modelview (OpenGL column-major)
        glm::mat4 mvp = ofGetCurrentMatrix(OF_MATRIX_PROJECTION) * ofGetCurrentMatrix(OF_MATRIX_MODELVIEW);
        pointsShader.setUniformMatrix4f("modelViewProjectionMatrix", mvp);
        particlesMesh.getVbo().draw(GL_POINTS, 0, (int)n);
        pointsShader.end();
    } else {
        glPointSize(particleSize);
        glEnable(GL_POINT_SMOOTH);
        particlesMesh.getVbo().draw(GL_POINTS, 0, (int)n);
        glDisable(GL_POINT_SMOOTH);
    }
    
    ofPopMatrix();
    
    particles_rendered_this_frame = (int)n;
    draw_ms = (ofGetElapsedTimef() - t_draw_start) * 1000.0f;
    
    // Debug overlay
    drawDebugOverlay();
    
    // GUI
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::exit(){
    // Limpieza opcional si se añaden recursos (plan Iter 6: higiene)
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
    // Usar dt_sec centralizado (asignado al inicio de update())
    int mouseX = ofGetMouseX();
    int mouseY = ofGetMouseY();
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
        mouse.vel = (pos_pixels - pos_prev_pixels) / dt_sec;
        
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
    
    // Aplicar fuerza a cada partícula (usa dt_sec centralizado)
    for (auto& p : particles) {
        // Distancia desde partícula al mouse (en pixels)
        ofVec2f particlePosPixels = ofVec2f(p.pos.x, p.pos.y);
        ofVec2f diff = particlePosPixels - mousePosPixels;
        float r = diff.length();
        
        // Calcular dirección radial desde mouse hacia partícula (push radial)
        // Esto empuja las partículas alejándolas del mouse
        ofVec2f push_dir;
        if (r > 0.001f) {
            push_dir = diff.getNormalized(); // Dirección desde mouse hacia partícula
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
        p.vel += (F_gesture / p.mass) * dt_sec;
    }
}

//--------------------------------------------------------------
void ofApp::getModeCoefficients(int mode, int& m, int& n, float& a, float& b) {
    // Mapeo determinístico de mode → (m,n) y coeficientes (a,b)
    // Para modos degenerados (m != n): a² + b² = 1, a y b fijos por mode
    // Para modos simétricos (m == n): a=1, b=0 (comportamiento original)
    
    switch (mode) {
        case 0: m = 1; n = 1; a = 1.0f; b = 0.0f; break;  // Modo fundamental (simétrico)
        case 1: m = 1; n = 2; a = 0.707106781f; b = 0.707106781f; break;  // 1x2 (degenerado, mezcla 50/50)
        case 2: m = 2; n = 1; a = 0.707106781f; b = 0.707106781f; break;  // 2x1 (degenerado, mezcla 50/50)
        case 3: m = 2; n = 2; a = 1.0f; b = 0.0f; break;  // 2x2 (simétrico)
        case 4: m = 3; n = 1; a = 0.707106781f; b = 0.707106781f; break;  // 3x1 (degenerado)
        case 5: m = 1; n = 3; a = 0.707106781f; b = 0.707106781f; break;  // 1x3 (degenerado)
        case 6: m = 3; n = 2; a = 0.707106781f; b = 0.707106781f; break;  // 3x2 (degenerado)
        case 7: m = 2; n = 3; a = 0.707106781f; b = 0.707106781f; break;  // 2x3 (degenerado)
        default: m = 1; n = 1; a = 1.0f; b = 0.0f; break;
    }
}

//--------------------------------------------------------------
void ofApp::rebuildPlateField(int mode, float plateSizeX, float plateSizeY) {
    const float PI_VAL = 3.14159265358979323846f;
    int m, n;
    float a, b;
    getModeCoefficients(mode, m, n, a, b);
    float norm_factor = 1.0f / (float)(m + n);

    size_t numCells = (size_t)gridPlateW * (size_t)gridPlateH;
    plateGradE.resize(numCells);
    plateU.resize(numCells);

    for (int iy = 0; iy < gridPlateH; ++iy) {
        for (int ix = 0; ix < gridPlateW; ++ix) {
            float xHat = (ix + 0.5f) / (float)gridPlateW;
            float yHat = (iy + 0.5f) / (float)gridPlateH;

            float U1 = sin(m * PI_VAL * xHat) * sin(n * PI_VAL * yHat);
            float U2 = sin(n * PI_VAL * xHat) * sin(m * PI_VAL * yHat);
            float U_field = a * U1 + b * U2;

            float dU1_dxHat = m * PI_VAL * cos(m * PI_VAL * xHat) * sin(n * PI_VAL * yHat);
            float dU1_dyHat = n * PI_VAL * sin(m * PI_VAL * xHat) * cos(n * PI_VAL * yHat);
            float dU2_dxHat = n * PI_VAL * cos(n * PI_VAL * xHat) * sin(m * PI_VAL * yHat);
            float dU2_dyHat = m * PI_VAL * sin(n * PI_VAL * xHat) * cos(m * PI_VAL * yHat);
            float dU_dxHat = a * dU1_dxHat + b * dU2_dxHat;
            float dU_dyHat = a * dU1_dyHat + b * dU2_dyHat;

            float U_norm = U_field * norm_factor;
            float dU_dxHat_norm = dU_dxHat * norm_factor;
            float dU_dyHat_norm = dU_dyHat * norm_factor;
            float dE_dxHat = 2.0f * U_norm * dU_dxHat_norm;
            float dE_dyHat = 2.0f * U_norm * dU_dyHat_norm;

            float gradX_world = dE_dxHat / plateSizeX;
            float gradY_world = dE_dyHat / plateSizeY;

            size_t idx = (size_t)iy * (size_t)gridPlateW + (size_t)ix;
            plateGradE[idx] = ofVec2f(gradX_world, gradY_world);
            plateU[idx] = U_norm;
        }
    }
    lastBuiltPlateMode = mode;
    lastBuiltPlateWidth = plateSizeX;
    lastBuiltPlateHeight = plateSizeY;
}

//--------------------------------------------------------------
void ofApp::applyPlateForce() {
    if (plateAmp < 0.01f) {
        return;
    }

    float winWidth = ofGetWidth();
    float winHeight = ofGetHeight();
    float plateCenterX = winWidth * 0.5f;
    float plateCenterY = winHeight * 0.5f;
    float plateSizeX = winWidth;
    float plateSizeY = winHeight;

    // Rebuild con debounce (máx cada 50 ms mientras plateDirty; plan 4.1.1)
    float now = ofGetElapsedTimef();
    if (plateDirty && (now - lastPlateRebuildTime >= 0.05f)) {
        rebuildPlateField(plateMode, plateSizeX, plateSizeY);
        lastPlateRebuildTime = now;
        plateDirty = false;
    }

    if (plateGradE.empty() || plateU.empty()) return;

    float freqNorm = ofClamp((plateFreq - 20.0f) / (2000.0f - 20.0f), 0.0f, 1.0f);
    float excitationIntensity = 0.5f + freqNorm * 0.5f;
    float forceIntensityBase = plateForceStrength * plateAmp * excitationIntensity;

    const float F_MAX = 100.0f;
    const float THRESHOLD_NODE = 0.1f;
    const float EXTRA_DAMPING = 0.3f;
    const float SIGMA_SPATIAL = 0.4f;

    int gw = gridPlateW;
    int gh = gridPlateH;
    float u_scale = (gw > 1) ? (float)(gw - 1) : 1.0f;
    float v_scale = (gh > 1) ? (float)(gh - 1) : 1.0f;

    for (auto& p : particles) {
        float xLocal = p.pos.x - plateCenterX;
        float yLocal = p.pos.y - plateCenterY;
        float xHat = ofClamp(0.5f + (xLocal / plateSizeX) * 0.5f, 0.0f, 1.0f);
        float yHat = ofClamp(0.5f + (yLocal / plateSizeY) * 0.5f, 0.0f, 1.0f);

        // Muestreo bilinear de ∇E y U (sin trig por partícula)
        float u = xHat * u_scale;
        float v = yHat * v_scale;
        int i0 = ofClamp((int)u, 0, gw - 1);
        int j0 = ofClamp((int)v, 0, gh - 1);
        int i1 = ofMin(i0 + 1, gw - 1);
        int j1 = ofMin(j0 + 1, gh - 1);
        float fu = u - i0;
        float fv = v - j0;

        ofVec2f g00 = plateGradE[(size_t)j0 * gw + i0];
        ofVec2f g10 = plateGradE[(size_t)j0 * gw + i1];
        ofVec2f g01 = plateGradE[(size_t)j1 * gw + i0];
        ofVec2f g11 = plateGradE[(size_t)j1 * gw + i1];
        ofVec2f gradE(g00.x * (1.0f - fu) * (1.0f - fv) + g10.x * fu * (1.0f - fv) + g01.x * (1.0f - fu) * fv + g11.x * fu * fv,
                      g00.y * (1.0f - fu) * (1.0f - fv) + g10.y * fu * (1.0f - fv) + g01.y * (1.0f - fu) * fv + g11.y * fu * fv);

        float U00 = plateU[(size_t)j0 * gw + i0];
        float U10 = plateU[(size_t)j0 * gw + i1];
        float U01 = plateU[(size_t)j1 * gw + i0];
        float U11 = plateU[(size_t)j1 * gw + i1];
        float U_norm = U00 * (1.0f - fu) * (1.0f - fv) + U10 * fu * (1.0f - fv) + U01 * (1.0f - fu) * fv + U11 * fu * fv;

        float gradMag = gradE.length();
        if (gradMag < 0.0001f) continue;

        float dist_norm = sqrt((xHat - 0.5f) * (xHat - 0.5f) + (yHat - 0.5f) * (yHat - 0.5f));
        float spatial_weight = exp(-dist_norm * dist_norm / (2.0f * SIGMA_SPATIAL * SIGMA_SPATIAL));
        float forceIntensity = forceIntensityBase * spatial_weight;

        ofVec2f F_plate = -gradE * forceIntensity;
        float F_mag = F_plate.length();
        if (F_mag > F_MAX) F_plate *= (F_MAX / F_mag);

        p.vel += (F_plate / p.mass) * dt_sec;

        float U_abs = fabs(U_norm);
        if (U_abs < THRESHOLD_NODE) {
            float damping_strength = EXTRA_DAMPING * (1.0f - U_abs / THRESHOLD_NODE);
            float damping_factor = 1.0f / (1.0f + damping_strength);
            p.vel *= damping_factor;
        }

        if (chladniState && plateAmp >= 0.01f) {
            float E_clamped = ofClamp(U_norm * U_norm, 0.0f, 1.0f);
            float E_shaped = pow(E_clamped, 2.0f);
            float shaker_magnitude = plateShakerStrength * plateAmp * E_shaped;

            float noise_scale = 0.01f;
            float time_scale = 0.5f;
            float offset = 100.0f;
            float time = ofGetElapsedTimef();
            float dir_x = ofSignedNoise(p.pos.x * noise_scale, p.pos.y * noise_scale, time * time_scale);
            float dir_y = ofSignedNoise(p.pos.x * noise_scale + offset, p.pos.y * noise_scale + offset, time * time_scale);

            ofVec2f direction(dir_x, dir_y);
            float dir_mag = direction.length();
            if (dir_mag > 0.001f) direction /= dir_mag;
            else direction = ofVec2f(1.0f, 0.0f);

            ofVec2f F_shaker = direction * shaker_magnitude;
            const float F_SHAKER_MAX = 0.5f * F_MAX;
            float F_shaker_mag = F_shaker.length();
            if (F_shaker_mag > F_SHAKER_MAX) F_shaker *= (F_SHAKER_MAX / F_shaker_mag);
            p.vel += (F_shaker / p.mass) * dt_sec;
        }
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
void ofApp::checkParticleCollisions() {
    float simWidth = ofGetWidth();
    float simHeight = ofGetHeight();
    // Geometría unívoca: r = particle_radius, collision_distance = 2*r, cell_size = collision_distance
    float collision_distance = particle_radius * 2.0f;
    float cell_size = collision_distance;

    // Dimensiones del grid fijo acotado a la simulación
    gridW = (simWidth > 0 && cell_size > 0) ? (int)std::ceil(simWidth / cell_size) : 0;
    gridH = (simHeight > 0 && cell_size > 0) ? (int)std::ceil(simHeight / cell_size) : 0;
    if (gridW < 1 || gridH < 1) {
        narrow_phase_pairs_checked = 0;
        collisions_resolved = 0;
        return;
    }

    size_t numCells = (size_t)gridW * (size_t)gridH;
    if (grid.size() != numCells) {
        grid.resize(numCells);
    }
    for (auto& cell : grid) {
        cell.clear();
    }

    // Insertar partículas en el grid (clamp a celdas borde si fuera de límites)
    float inv_cs = 1.0f / cell_size;
    for (size_t i = 0; i < particles.size(); ++i) {
        int cx = (int)std::floor(particles[i].pos.x * inv_cs);
        int cy = (int)std::floor(particles[i].pos.y * inv_cs);
        cx = ofClamp(cx, 0, gridW - 1);
        cy = ofClamp(cy, 0, gridH - 1);
        size_t idx = (size_t)cy * (size_t)gridW + (size_t)cx;
        grid[idx].push_back(i);
    }

    // Guardar velocidades PRE-colisión para todas las partículas (solo para energía de evento)
    for (auto& p : particles) {
        p.vel_pre = p.vel;
    }

    narrow_phase_pairs_checked = 0;
    collisions_resolved = 0;

    // Límite de corrección posicional por partícula por frame (plan 3.4)
    if (correction_used.size() != particles.size()) {
        correction_used.resize(particles.size());
    }
    std::fill(correction_used.begin(), correction_used.end(), 0.0f);
    const float slop = 0.15f * particle_radius;
    const float correction_percent = 0.4f;
    const float e_clamped = ofClamp(restitution, 0.0f, 1.0f);

    // Para cada partícula, solo comprobar vecindad 3×3
    for (size_t i = 0; i < particles.size(); ++i) {
        Particle& p1 = particles[i];
        int cx = (int)std::floor(p1.pos.x * inv_cs);
        int cy = (int)std::floor(p1.pos.y * inv_cs);
        cx = ofClamp(cx, 0, gridW - 1);
        cy = ofClamp(cy, 0, gridH - 1);

        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int nx = cx + dx;
                int ny = cy + dy;
                if (nx < 0 || nx >= gridW || ny < 0 || ny >= gridH) continue;
                size_t nkey = (size_t)ny * (size_t)gridW + (size_t)nx;
                for (size_t j : grid[nkey]) {
                    if (j <= i) continue;
                    Particle& p2 = particles[j];

                    narrow_phase_pairs_checked++;

                    ofVec2f diff = p1.pos - p2.pos;
                    float dist = diff.length();
                    // Épsilon mínimo de distancia (plan 3.4)
                    if (dist < 1e-6f) continue;
                    ofVec2f n = diff / dist;
                    // Resolución física con velocidades ACTUALES (p.vel)
                    float v_n = (p1.vel - p2.vel).dot(n);
                    if (v_n >= 0.0f) continue;  // separándose

                    // Impulso: j_impulse = -(1+e)*v_n/2 (masas iguales); n de p2 a p1
                    float j_impulse = -(1.0f + e_clamped) * v_n * 0.5f;
                    p1.vel += n * j_impulse;
                    p2.vel -= n * j_impulse;

                    // Corrección posicional: slop + percent + límite por frame (plan 3.4)
                    float overlap = collision_distance - dist;
                    if (overlap > slop) {
                        float corr_each = (overlap - slop) * correction_percent * 0.5f;
                        float max_left_i = collision_distance - correction_used[i];
                        float max_left_j = collision_distance - correction_used[j];
                        corr_each = ofMin(corr_each, ofMin(max_left_i, max_left_j));
                        if (corr_each > 0.0f) {
                            p1.pos += n * corr_each;
                            p2.pos -= n * corr_each;
                            correction_used[i] += corr_each;
                            correction_used[j] += corr_each;
                        }
                    }

                    // Evento de hit (energía usa vel_pre; no modifica velocidades)
                    ofVec2f collisionPoint = (p1.pos + p2.pos) * 0.5f;
                    generateParticleHitEvent(p1, p2, collisionPoint);
                    collisions_resolved++;
                }
            }
        }
    }
}

//--------------------------------------------------------------
float ofApp::calculateHitEnergy(Particle& p, int surface) {
    // Velocidad normalizada: speed_norm = |vel_pre| / vel_ref
    float speed_norm = ofClamp(p.vel_pre.length() / vel_ref, 0.0f, 1.0f);
    
    // Distancia normalizada: dist_norm = last_hit_distance / dist_ref
    float dist_norm = ofClamp(p.last_hit_distance / dist_ref, 0.0f, 1.0f);
    
    // Energía: dist_norm ponderado por speed_norm para no generar energía en reposo
    // energy = energy_a * speed_norm + energy_b * dist_norm * speed_norm (clamp 0..1)
    float energy = ofClamp(energy_a * speed_norm + energy_b * dist_norm * speed_norm, 0.0f, 1.0f);
    
    // Validación mínima: descartar ruido numérico extremo
    if (energy < 0.001f) {
        return 0.0f;
    }
    
    return energy;
}

//--------------------------------------------------------------
float ofApp::calculateParticleCollisionEnergy(Particle& p1, Particle& p2) {
    // Calcular velocidad relativa
    ofVec2f relVel = p1.vel_pre - p2.vel_pre;
    float relSpeed = relVel.length();
    
    // Normalizar velocidad relativa usando vel_ref
    float speed_norm = ofClamp(relSpeed / vel_ref, 0.0f, 1.0f);
    
    // Usar distancia promedio desde último hit
    float avg_distance = (p1.last_hit_distance + p2.last_hit_distance) * 0.5f;
    float dist_norm = ofClamp(avg_distance / dist_ref, 0.0f, 1.0f);
    
    // Energía: dist_norm ponderado por speed_norm (clamp 0..1)
    float energy = ofClamp(energy_a * speed_norm + energy_b * dist_norm * speed_norm, 0.0f, 1.0f);
    
    // Validación mínima: descartar ruido numérico extremo
    if (energy < 0.001f) {
        return 0.0f;
    }
    
    return energy;
}

//--------------------------------------------------------------
void ofApp::generateParticleHitEvent(Particle& p1, Particle& p2, ofVec2f collisionPoint) {
    // Rest gate (Fase 1): velocidad normal de colisión; no tocar lastHitTime/last_hit_distance si no pasamos
    float rest_epsilon = REST_SPEED_EPSILON_FACTOR * vel_ref;
    ofVec2f diff = p1.pos - p2.pos;
    float dist = diff.length();
    ofVec2f collisionNormal;
    if (dist >= 1e-6f) {
        collisionNormal = diff / dist;
    }
    ofVec2f vrel = p1.vel_pre - p2.vel_pre;
    float vn = (dist >= 1e-6f) ? std::abs(vrel.dot(collisionNormal)) : vrel.length();
    if (vn < rest_epsilon && !isExternalForceActive()) {
        return;
    }

    hits_candidate_p2p++;

    // Calcular energía del impacto
    float energy = calculateParticleCollisionEnergy(p1, p2);
    if (energy < ENERGY_FLOOR) {
        hits_discarded_low_energy++;
        return;
    }

    float timeNow = ofGetElapsedTimef();
    float cooldown_seconds = hit_cooldown_ms / 1000.0f;
    float timeSinceLastHit1 = timeNow - p1.lastHitTime;
    float timeSinceLastHit2 = timeNow - p2.lastHitTime;
    if (timeSinceLastHit1 < cooldown_seconds && timeSinceLastHit2 < cooldown_seconds) {
        hits_discarded_cooldown++;
        return;
    }

    // Crear evento de hit usando la partícula con mayor energía o la primera
    Particle& p = (p1.vel_pre.length() > p2.vel_pre.length()) ? p1 : p2;
    HitEvent event;
    event.id = p.id;
    event.x = ofClamp(collisionPoint.x / ofGetWidth(), 0.0f, 1.0f);
    event.y = ofClamp(collisionPoint.y / ofGetHeight(), 0.0f, 1.0f);
    event.energy = energy;
    event.surface = -1;

    pending_hits.push_back(event);
    hits_added_pending++;

    p1.lastHitTime = timeNow;
    p1.last_hit_distance = 0.0f;
    p1.last_surface = -1;
    p2.lastHitTime = timeNow;
    p2.last_hit_distance = 0.0f;
    p2.last_surface = -1;
}

//--------------------------------------------------------------
void ofApp::generateHitEvent(Particle& p, int surface) {
    // Rest gate (Fase 1): componente normal de velocidad al borde
    float rest_epsilon = REST_SPEED_EPSILON_FACTOR * vel_ref;
    ofVec2f borderNormal;
    if (surface == 0) borderNormal.set(-1.0f, 0.0f);       // L
    else if (surface == 1) borderNormal.set(1.0f, 0.0f);     // R
    else if (surface == 2) borderNormal.set(0.0f, -1.0f);  // T
    else borderNormal.set(0.0f, 1.0f);                      // B
    float vn = std::abs(p.vel_pre.dot(borderNormal));
    if (vn < rest_epsilon && !isExternalForceActive()) {
        return;
    }

    hits_candidate_border++;

    float energy = calculateHitEnergy(p, surface);
    if (energy < ENERGY_FLOOR) {
        hits_discarded_low_energy++;
        return;
    }

    float timeNow = ofGetElapsedTimef();
    float cooldown_seconds = hit_cooldown_ms / 1000.0f;
    if (timeNow - p.lastHitTime < cooldown_seconds) {
        hits_discarded_cooldown++;
        return;
    }

    HitEvent event;
    event.id = p.id;
    event.x = ofClamp(p.pos.x / ofGetWidth(), 0.0f, 1.0f);
    event.y = ofClamp(p.pos.y / ofGetHeight(), 0.0f, 1.0f);
    event.energy = energy;
    event.surface = surface;

    pending_hits.push_back(event);
    hits_added_pending++;

    p.lastHitTime = timeNow;
    p.last_hit_distance = 0.0f;
    p.last_surface = surface;
}

//--------------------------------------------------------------
void ofApp::updateRateLimiter(float dt) {
    rate_limiter_border.tokens += rate_limiter_border.rate * dt;
    rate_limiter_border.tokens = ofMin(rate_limiter_border.tokens, rate_limiter_border.burst);
    rate_limiter_pp.tokens += rate_limiter_pp.rate * dt;
    rate_limiter_pp.tokens = ofMin(rate_limiter_pp.tokens, rate_limiter_pp.burst);
    rate_limiter.hits_this_frame = 0;
}

//--------------------------------------------------------------
bool ofApp::canEmitHit(const HitEvent& event) {
    bool is_border = event.surface >= 0;
    RateLimiter& bucket = is_border ? rate_limiter_border : rate_limiter_pp;
    if (bucket.tokens < 1.0f) return false;
    if (rate_limiter.hits_this_frame >= rate_limiter.max_per_frame) return false;
    return true;
}

//--------------------------------------------------------------
void ofApp::consumeToken(const HitEvent& event) {
    bool is_border = event.surface >= 0;
    RateLimiter& bucket = is_border ? rate_limiter_border : rate_limiter_pp;
    bucket.tokens -= 1.0f;
    rate_limiter.hits_this_frame++;
}

//--------------------------------------------------------------
void ofApp::processPendingHits() {
    for (const auto& event : pending_hits) {
        if (canEmitHit(event)) {
            validated_hits.push_back(event);
            hits_validated++;
            validated_this_frame++;
            consumeToken(event);
            hits_this_second++;
        } else {
            hits_discarded_rate++;
            dropped_rate_this_frame++;
        }
    }
}

//--------------------------------------------------------------
void ofApp::drawDebugOverlay() {
    ofSetColor(255, 255, 255);
    stringstream ss;
    ss << "FPS: " << ofGetFrameRate() << " (target <=33ms)" << endl;
    ss << "update_total_ms: " << update_total_ms << endl;
    ss << "p2p_collision_ms: " << p2p_collision_ms << endl;
    ss << "plate_force_ms: " << plate_force_ms << endl;
    ss << "draw_ms: " << draw_ms << endl;
    ss << "narrow_phase_pairs_checked: " << narrow_phase_pairs_checked << endl;
    ss << "collisions_resolved: " << collisions_resolved << endl;
    ss << "osc_msgs_sent_per_sec: " << hits_per_second << " (per_sec)" << endl;
    ss << "osc_msgs_dropped_by_rate_limiter: " << hits_discarded_rate << " (per_sec)" << endl;
    ss << "this_frame: validated " << validated_this_frame << " sent " << sent_this_frame << " dropped " << dropped_rate_this_frame << endl;
    ss << "---" << endl;
    ss << "Particles (total): " << particles.size() << endl;
    ss << "Particles (rendered): " << particles_rendered_this_frame << endl;
    ss << "k_home: " << k_home << " k_drag: " << k_drag << " k_gesture: " << k_gesture << endl;
    ss << "Mouse vel: " << mouse.vel.length() << " px/s" << endl;
    ss << "Discarded (cooldown): " << hits_discarded_cooldown << endl;
    ss << "candidate_border: " << hits_candidate_border << " candidate_p2p: " << hits_candidate_p2p << endl;
    ss << "added_pending: " << hits_added_pending << " validated: " << hits_validated << " sent_osc: " << hits_sent_osc << " (per_sec)" << endl;
    ss << "Discarded (low_energy): " << hits_discarded_low_energy << endl;
    ss << "Tokens border: " << rate_limiter_border.tokens << " pp: " << rate_limiter_pp.tokens << endl;
    ss << "frame cap: " << rate_limiter.hits_this_frame << "/" << rate_limiter.max_per_frame << endl;
    ss << "OSC: " << (oscEnabled ? "ON" : "OFF");
    if (oscEnabled) ss << " " << oscHost << ":" << oscPort;

    float x = 20.0f;
    float lineHeight = 14.0f;
    int lineCount = 23;
    float y = ofGetHeight() - (lineCount * lineHeight) - 20.0f;
    if (y < 20.0f) y = 20.0f;

    // Fase 5: fondo semi-opaco para legibilidad (ancho por línea más larga × ~8px, alto por lineCount)
    const float padding = 7.0f;
    const int approxCharWidth = 8;
    const int maxLineChars = 58;
    float boxW = (float)(maxLineChars * approxCharWidth);
    float boxH = lineCount * lineHeight + 2.0f * padding;
    ofSetColor(0, 0, 0, 180);
    ofDrawRectangle(x - padding, y - padding, boxW, boxH);

    ofSetColor(255, 255, 255);
    ofDrawBitmapString(ss.str(), x, y);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    // v0.3: SPACE toggle para Chladni State
    if (key == ' ' || key == OF_KEY_SPACE) {
        if (!chladniState) {
            // Al activar Chladni State: guardar k_home actual
            k_home_previous = k_home;
            chladniState = true;
        } else {
            // Al desactivar Chladni State: restaurar k_home y sincronizar slider
            k_home = k_home_previous;
            kHomeSlider = k_home_previous;  // Sincronizar UI
            chladniState = false;
        }
        return;  // No procesar otros casos si se presionó SPACE
    }
    
    // Presets de cámara con teclas 1-4
    switch(key) {
        case '1':
            // Preset 1: Vista normal
            cameraZoom = 1.0f;
            cameraRotation = 0.0f;
            cameraZoomSlider = cameraZoom;
            cameraRotationSlider = cameraRotation;
            break;
        case '2':
            // Preset 2: Zoom in centro
            cameraZoom = 2.0f;
            cameraRotation = 0.0f;
            cameraZoomSlider = cameraZoom;
            cameraRotationSlider = cameraRotation;
            break;
        case '3':
            // Preset 3: Rotación 45°
            cameraZoom = 1.0f;
            cameraRotation = 45.0f;
            cameraZoomSlider = cameraZoom;
            cameraRotationSlider = cameraRotation;
            break;
        case '4':
            // Preset 4: Vista amplia
            cameraZoom = 0.5f;
            cameraRotation = 0.0f;
            cameraZoomSlider = cameraZoom;
            cameraRotationSlider = cameraRotation;
            break;
    }
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
    (void)x; (void)y; // Handler vacío (ofBaseApp); no se usa.
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    (void)x; (void)y; // Handler vacío (ofBaseApp); no se usa.
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    plateDirty = true;
    // Recalcular posiciones home cuando cambia el tamaño de ventana
    if (particles.size() > 0) {
        initializeParticles(particles.size());
    }
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    (void)msg; // Handler vacío (ofBaseApp); no se usa.
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    (void)dragInfo; // Handler vacío (ofBaseApp); no se usa.
}

//--------------------------------------------------------------
void ofApp::setupOSC() {
    // Configuración por defecto
    oscHost = "127.0.0.1";
    oscPort = 9000;
    oscEnabled = true;
    stateSendInterval = 0.1f;  // 10 Hz
    stateSendTimer = 0.0f;
    
    // Inicializar sender OSC
    oscSender.setup(oscHost, oscPort);
    
    ofLogNotice("ofApp") << "OSC configurado: " << oscHost << ":" << oscPort;
}

//--------------------------------------------------------------
void ofApp::sendHitEvent(const HitEvent& event) {
    if (!oscEnabled) {
        return;
    }
    
    ofxOscMessage msg;
    msg.setAddress("/hit");
    msg.addIntArg(event.id);           // int32 id
    msg.addFloatArg(event.x);          // float x (0..1)
    msg.addFloatArg(event.y);          // float y (0..1)
    msg.addFloatArg(event.energy);     // float energy (0..1)
    msg.addIntArg(event.surface);      // int32 surface (0=L, 1=R, 2=T, 3=B, -1=N/A)
    
    oscSender.sendMessage(msg, false);
    
    // Debug opcional (comentado para no saturar logs)
    // ofLogVerbose("ofApp") << "OSC /hit: id=" << event.id 
    //                       << " x=" << event.x 
    //                       << " y=" << event.y 
    //                       << " energy=" << event.energy 
    //                       << " surface=" << event.surface;
}

//--------------------------------------------------------------
void ofApp::sendStateMessage() {
    if (!oscEnabled) {
        return;
    }
    
    // Solo enviar si hay actividad (evitar spam cuando no hay hits)
    float activity = calculateActivity();
    if (activity < 0.001f) {
        return; // Sin actividad, no enviar
    }
    
    ofxOscMessage msg;
    msg.setAddress("/state");
    msg.addFloatArg(activity);         // float activity (0..1)
    msg.addFloatArg(calculateGesture()); // float gesture (0..1)
    msg.addFloatArg(calculatePresence()); // float presence (0..1)
    
    oscSender.sendMessage(msg, false);
    
    // Debug opcional
    // ofLogVerbose("ofApp") << "OSC /state: activity=" << activity;
}

//--------------------------------------------------------------
void ofApp::sendPlateMessage() {
    if (!oscEnabled) {
        return;
    }
    
    // Validar y clamp valores antes de enviar
    float freq = ofClamp(plateFreq, 20.0f, 2000.0f);
    float amp = ofClamp(plateAmp, 0.0f, 1.0f);
    int mode = ofClamp(plateMode, 0, 7);
    
    ofxOscMessage msg;
    msg.setAddress("/plate");
    msg.addFloatArg(freq);  // float freq (20.0-2000.0 Hz)
    msg.addFloatArg(amp);   // float amp (0.0-1.0)
    msg.addIntArg(mode);    // int32 mode (0-7)
    
    oscSender.sendMessage(msg, false);
    
    // Debug opcional
    // ofLogVerbose("ofApp") << "OSC /plate: freq=" << freq << " amp=" << amp << " mode=" << mode;
}

//--------------------------------------------------------------
float ofApp::calculateActivity() {
    // Normalizar hits_per_second al rango 0..1
    // Usar max_hits_per_second como referencia máxima
    if (max_hits_per_second <= 0.0f) {
        return 0.0f;
    }
    return ofClamp(hits_per_second / max_hits_per_second, 0.0f, 1.0f);
}

//--------------------------------------------------------------
float ofApp::calculateGesture() {
    // Para mouse input, usar velocidad normalizada como proxy de energía de gesto
    // En el futuro, con MediaPipe, esto sería energía agregada de gestos detectados
    if (!mouse.active) {
        return 0.0f;
    }
    
    float vel_magnitude = mouse.vel.length();
    float speed = ofClamp(vel_magnitude / speed_ref, 0.0f, 1.0f);
    
    // Suavizar para evitar cambios bruscos
    static float gesture_smooth = 0.0f;
    gesture_smooth = gesture_smooth * 0.9f + speed * 0.1f;
    
    return gesture_smooth;
}

//--------------------------------------------------------------
float ofApp::calculatePresence() {
    // Para mouse input, presence = 1.0 si está activo, 0.0 si no
    // En el futuro, con MediaPipe, esto sería confianza del tracking
    return mouse.active ? 1.0f : 0.0f;
}


#include "ofApp.h"
#include <sstream>

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
    max_hits_per_second = 400.0f;  // Máximo de hits por segundo (50-1000) - aumentado para sonido más denso
    burst = 500.0f;                // Burst máximo (100-1000) - aumentado para permitir más eventos simultáneos
    max_hits_per_frame = 30;        // Máximo de hits por frame (5-50) - aumentado para sonido más denso
    
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
    
    // Inicializar partículas
    initializeParticles(initialN);
    
    // Configurar OSC
    setupOSC();
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
    
    // Actualizar tamaño de partículas
    particleSize = particleSizeSlider;
    
    // Actualizar parámetros de cámara
    cameraZoom = cameraZoomSlider;
    cameraRotation = cameraRotationSlider;
    
    // Actualizar parámetros de Plate Controller
    plateFreq = plateFreqSlider;
    plateAmp = plateAmpSlider;
    plateMode = plateModeSlider;
    
    // Detectar cambio en número de partículas
    int targetN = nParticlesSlider;
    if (targetN != (int)particles.size()) {
        resizeParticles(targetN);
    }
    
    // Actualizar input del mouse
    updateMouseInput();
    
    // Aplicar fuerza de gesto a las partículas
    applyGestureForce();
    
    // Aplicar fuerza del Plate Controller a las partículas
    applyPlateForce();
    
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
    
    // Detectar colisiones entre partículas (si está habilitado)
    if (enable_particle_collisions) {
        checkParticleCollisions();
    }
    
    // Procesar eventos pendientes con rate limiting
    processPendingHits();
    
    // Enviar eventos OSC validados
    if (oscEnabled) {
        for (const auto& event : validated_hits) {
            sendHitEvent(event);
        }
        
        // Enviar mensaje /state periódicamente (10 Hz durante actividad)
        stateSendTimer += dt;
        if (stateSendTimer >= stateSendInterval) {
            sendStateMessage();
            stateSendTimer = 0.0f;
        }
        
        // Enviar mensaje /plate con rate limiting (20-30 Hz)
        plateSendTimer += dt;
        if (plateSendTimer >= plateSendInterval) {
            sendPlateMessage();
            plateSendTimer = 0.0f;
        }
    }
    
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
    
    // Aplicar transformaciones de cámara
    ofPushMatrix();
    float centerX = ofGetWidth() / 2.0f;
    float centerY = ofGetHeight() / 2.0f;
    ofTranslate(centerX, centerY);
    ofScale(cameraZoom, cameraZoom);
    ofRotateDeg(cameraRotation);
    ofTranslate(-centerX, -centerY);
    
    // Render de partículas como puntos
    ofSetColor(255, 255, 255); // Blanco/azulado metálico
    glPointSize(particleSize);  // Tamaño variable desde slider
    glEnable(GL_POINT_SMOOTH);
    glBegin(GL_POINTS);
    
    // Contador de partículas renderizadas (para debug)
    int rendered_count = 0;
    for (const auto& p : particles) {
        glVertex2f(p.pos.x, p.pos.y);
        rendered_count++;
    }
    glEnd();
    glDisable(GL_POINT_SMOOTH);
    
    // Restaurar transformaciones
    ofPopMatrix();
    
    // Guardar contador para debug overlay
    particles_rendered_this_frame = rendered_count;
    
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
        p.vel += (F_gesture / p.mass) * dt;
    }
}

//--------------------------------------------------------------
void ofApp::applyPlateForce() {
    // Solo aplicar fuerza si la amplitud del plate es significativa
    if (plateAmp < 0.01f) {
        return; // Plate silencioso, no aplicar fuerza
    }
    
    float winWidth = ofGetWidth();
    float winHeight = ofGetHeight();
    
    // ============================================================
    // 1. SISTEMA DE COORDENADAS DE PLACA FIJO (centro inmutable)
    // ============================================================
    // Centro de la placa: siempre el centro de la ventana (FIJO)
    float plateCenterX = winWidth * 0.5f;
    float plateCenterY = winHeight * 0.5f;
    
    // Tamaño de la placa: usar toda la ventana (FIJO)
    float plateSizeX = winWidth;
    float plateSizeY = winHeight;
    
    // ============================================================
    // 2. MAPEO DE plate_mode → (m, n) - SOLO PATRÓN ESPACIAL
    // ============================================================
    // plate_mode define el patrón espacial (nodos/antinodos)
    // NO debe depender de freq - el patrón es estacionario
    int m, n;
    switch (plateMode) {
        case 0: m = 1; n = 1; break;  // Modo fundamental
        case 1: m = 1; n = 2; break;  // 1 línea horizontal, 2 verticales
        case 2: m = 2; n = 1; break;  // 2 líneas horizontales, 1 vertical
        case 3: m = 2; n = 2; break;  // 2x2 grid
        case 4: m = 3; n = 1; break;  // 3 líneas horizontales
        case 5: m = 1; n = 3; break;  // 3 líneas verticales
        case 6: m = 3; n = 2; break;  // 3x2
        case 7: m = 2; n = 3; break;  // 2x3
        default: m = 1; n = 1; break;
    }
    
    // ============================================================
    // 3. FRECUENCIA SOLO PARA INTENSIDAD/ANIMACIÓN TEMPORAL
    // ============================================================
    // plate_freq NO modifica el patrón espacial
    // Solo se usa para intensidad de excitación
    float freqNorm = ofClamp((plateFreq - 20.0f) / (2000.0f - 20.0f), 0.0f, 1.0f);
    float excitationIntensity = 0.5f + freqNorm * 0.5f; // 0.5 a 1.0
    
    // Intensidad de fuerza: combina amplitud y frecuencia (solo para intensidad)
    float forceIntensity = plateForceStrength * plateAmp * excitationIntensity;
    
    // ============================================================
    // 4. CAMPO DE CHLADNI ESTACIONARIO (solo depende de modo y posición)
    // ============================================================
    const float PI_VAL = 3.14159265358979323846f;
    
    // Aplicar fuerza a cada partícula basada en campo de Chladni
    float dt = ofGetLastFrameTime();
    if (dt <= 0.0f) dt = 0.016f;
    
    for (auto& p : particles) {
        // ============================================================
        // 4a. TRANSFORMAR: pos_world → pos_plate → (x̂, ŷ) normalizado
        // ============================================================
        // Convertir posición de partícula a coordenadas locales de placa
        float xLocal = p.pos.x - plateCenterX;  // Coordenada local respecto al centro FIJO
        float yLocal = p.pos.y - plateCenterY;
        
        // Normalizar a rango [0, 1] para que coincida con sin(mπx) donde x ∈ [0,1]
        // Mapear [-size/2, +size/2] → [0, 1]
        float xHat = 0.5f + (xLocal / plateSizeX) * 0.5f;
        float yHat = 0.5f + (yLocal / plateSizeY) * 0.5f;
        
        // Clamp para asegurar que esté dentro de la placa
        xHat = ofClamp(xHat, 0.0f, 1.0f);
        yHat = ofClamp(yHat, 0.0f, 1.0f);
        
        // ============================================================
        // 4b. CALCULAR CAMPO ESTACIONARIO U(x̂, ŷ) = sin(mπx̂) * sin(nπŷ)
        // ============================================================
        // Este es el modo propio estacionario - NO depende de tiempo ni frecuencia
        float U_field = sin(m * PI_VAL * xHat) * sin(n * PI_VAL * yHat);
        
        // Magnitud del campo (energía de vibración en ese punto)
        // |U| alto = antinodo (vibra mucho)
        // |U| bajo ≈ 0 = nodo (no vibra, donde se acumula material)
        float fieldMagnitude = fabs(U_field);
        
        // ============================================================
        // 4c. CALCULAR GRADIENTE DEL CAMPO (dirección hacia nodos)
        // ============================================================
        // Gradiente de U en coordenadas normalizadas
        float dU_dxHat = m * PI_VAL * cos(m * PI_VAL * xHat) * sin(n * PI_VAL * yHat);
        float dU_dyHat = n * PI_VAL * sin(m * PI_VAL * xHat) * cos(n * PI_VAL * yHat);
        
        // Convertir gradiente de coordenadas normalizadas a coordenadas mundo
        // (escalar por tamaño de placa para mantener unidades correctas)
        float gradX_world = dU_dxHat / plateSizeX;
        float gradY_world = dU_dyHat / plateSizeY;
        
        ofVec2f gradient = ofVec2f(gradX_world, gradY_world);
        float gradMag = gradient.length();
        
        if (gradMag < 0.0001f) {
            // Estamos muy cerca de un nodo exacto, no aplicar fuerza
            continue;
        }
        
        // Normalizar gradiente
        gradient.normalize();
        
        // ============================================================
        // 4d. FUERZA HACIA NODOS (gradiente negativo del campo)
        // ============================================================
        // Las partículas se mueven "cuesta abajo" hacia nodos
        // Fuerza proporcional a:
        // - fieldMagnitude: más fuerza en antinodos (donde vibra más)
        // - forceIntensity: controlado por amp y freq (solo intensidad)
        float forceScale = fieldMagnitude * forceIntensity;
        
        // Dirección: opuesta al gradiente (hacia nodos donde U es mínimo)
        ofVec2f F_plate = -gradient * forceScale;
        
        // ============================================================
        // 4e. APLICAR FUERZA
        // ============================================================
        p.vel += (F_plate / p.mass) * dt;
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
    float width = ofGetWidth();
    float height = ofGetHeight();
    float collision_distance = particle_radius * 2.0f;  // Distancia mínima para colisión
    
    // Guardar velocidades PRE-colisión para todas las partículas
    for (auto& p : particles) {
        p.vel_pre = p.vel;
    }
    
    // Detectar colisiones entre todas las parejas de partículas
    // Usar doble loop pero evitar procesar la misma pareja dos veces
    for (size_t i = 0; i < particles.size(); i++) {
        Particle& p1 = particles[i];
        
        for (size_t j = i + 1; j < particles.size(); j++) {
            Particle& p2 = particles[j];
            
            // Calcular distancia entre partículas
            ofVec2f diff = p1.pos - p2.pos;
            float distance = diff.length();
            
            // Verificar si hay colisión
            if (distance < collision_distance && distance > 0.001f) {  // Evitar división por cero
                // Calcular punto de colisión (punto medio)
                ofVec2f collisionPoint = (p1.pos + p2.pos) * 0.5f;
                
                // Aplicar rebote físico (colisión elástica simple)
                // Calcular velocidad relativa usando velocidades PRE-colisión
                ofVec2f relVel = p1.vel_pre - p2.vel_pre;
                ofVec2f normal = diff.normalized();
                
                // Componente de velocidad relativa en dirección normal
                float velAlongNormal = relVel.dot(normal);
                
                // Solo procesar si las partículas se están acercando
                if (velAlongNormal < 0.0f) {
                    // Calcular impulso (simplificado, asumiendo masas iguales)
                    float impulse = velAlongNormal * (1.0f + restitution);
                    
                    // Aplicar impulso a ambas partículas
                    p1.vel -= normal * impulse * 0.5f;
                    p2.vel += normal * impulse * 0.5f;
                    
                    // Separar partículas para evitar penetración
                    float overlap = collision_distance - distance;
                    ofVec2f separation = normal * overlap * 0.5f;
                    p1.pos += separation;
                    p2.pos -= separation;
                    
                    // Generar evento de hit (usa vel_pre que ya fue guardado)
                    generateParticleHitEvent(p1, p2, collisionPoint);
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
    
    // Energía combinada: energy = clamp(a * speed_norm + b * dist_norm, 0..1)
    float energy = ofClamp(energy_a * speed_norm + energy_b * dist_norm, 0.0f, 1.0f);
    
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
    
    // Usar distancia promedio desde último hit (o usar distancia actual si no hay hit previo)
    float avg_distance = (p1.last_hit_distance + p2.last_hit_distance) * 0.5f;
    float dist_norm = ofClamp(avg_distance / dist_ref, 0.0f, 1.0f);
    
    // Energía combinada: energy = clamp(a * speed_norm + b * dist_norm, 0..1)
    float energy = ofClamp(energy_a * speed_norm + energy_b * dist_norm, 0.0f, 1.0f);
    
    // Validación mínima: descartar ruido numérico extremo
    if (energy < 0.001f) {
        return 0.0f;
    }
    
    return energy;
}

//--------------------------------------------------------------
void ofApp::generateParticleHitEvent(Particle& p1, Particle& p2, ofVec2f collisionPoint) {
    float timeNow = ofGetElapsedTimef();
    
    // Verificar cooldown para ambas partículas
    float timeSinceLastHit1 = timeNow - p1.lastHitTime;
    float timeSinceLastHit2 = timeNow - p2.lastHitTime;
    float cooldown_seconds = hit_cooldown_ms / 1000.0f;
    
    // Solo generar evento si ambas partículas están fuera del cooldown
    // (o al menos una, para evitar perder eventos)
    if (timeSinceLastHit1 < cooldown_seconds && timeSinceLastHit2 < cooldown_seconds) {
        hits_discarded_cooldown++;
        return; // Ambas partículas en cooldown
    }
    
    // Calcular energía del impacto
    float energy = calculateParticleCollisionEnergy(p1, p2);
    
    if (energy <= 0.0f) {
        return; // Energía demasiado baja (ruido numérico)
    }
    
    // Crear evento de hit usando la partícula con mayor energía o la primera
    // Usar punto de colisión como posición
    Particle& p = (p1.vel_pre.length() > p2.vel_pre.length()) ? p1 : p2;
    
    HitEvent event;
    event.id = p.id;  // Usar ID de la partícula con mayor velocidad
    event.x = ofClamp(collisionPoint.x / ofGetWidth(), 0.0f, 1.0f);
    event.y = ofClamp(collisionPoint.y / ofGetHeight(), 0.0f, 1.0f);
    event.energy = energy;
    event.surface = -1;  // -1 indica colisión partícula-partícula (no superficie)
    
    // Agregar a eventos pendientes
    pending_hits.push_back(event);
    
    // Actualizar estado de ambas partículas
    p1.lastHitTime = timeNow;
    p1.last_hit_distance = 0.0f;
    p1.last_surface = -1;
    
    p2.lastHitTime = timeNow;
    p2.last_hit_distance = 0.0f;
    p2.last_surface = -1;
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
    ss << "Particles (total): " << particles.size() << endl;
    ss << "Particles (rendered): " << particles_rendered_this_frame << " / " << particles.size() << endl;
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
    ss << "---" << endl;
    ss << "OSC: " << (oscEnabled ? "ON" : "OFF") << endl;
    if (oscEnabled) {
        ss << "OSC: " << oscHost << ":" << oscPort << endl;
    }
    
    // Dibujar en la parte inferior izquierda para no superponerse con la GUI
    // La GUI típicamente está en la esquina superior derecha
    float x = 20.0f;
    float lineHeight = 15.0f; // Altura aproximada de cada línea
    int lineCount = 15; // Número de líneas en el overlay (actualizado con OSC)
    float y = ofGetHeight() - (lineCount * lineHeight) - 20.0f;
    
    // Asegurar que no se salga de la pantalla
    if (y < 20.0f) {
        y = 20.0f;
    }
    
    ofDrawBitmapString(ss.str(), x, y);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
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

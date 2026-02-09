#include "ofApp.h"
#include <sstream>

//--------------------------------------------------------------
void ofApp::setup(){
    // Configurar parámetros iniciales
    k_home = 2.0f;
    k_drag = 1.0f;
    k_gesture = 50.0f;      // Fuerza del gesto
    sigma = 150.0f;         // Radio de influencia (pixels)
    speed_ref = 500.0f;     // Velocidad de referencia (pixels/s)
    smooth_alpha = 0.15f;   // Factor de suavizado
    int initialN = 2000;
    
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
    for (auto& p : particles) {
        p.update(dt, k_home, k_drag);
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
    
    // Normalizar velocidad y calcular dirección
    float vel_magnitude = mouse.vel.length();
    ofVec2f dir = mouse.vel.normalized();
    
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
        
        // Influencia gaussiana: w = exp(-(r^2)/(2*sigma^2))
        float w = exp(-(r * r) / (2.0f * sigma * sigma));
        
        // Fuerza de gesto: F_gesture = k_gesture * w * speed * dir
        ofVec2f F_gesture = k_gesture * w * speed * dir;
        
        // Aplicar fuerza directamente a la velocidad (impulso)
        p.vel += (F_gesture / p.mass) * dt;
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
    ofDrawBitmapString(ss.str(), 20, 20);
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

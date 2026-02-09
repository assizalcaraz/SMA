#include "ofApp.h"
#include <sstream>

//--------------------------------------------------------------
void ofApp::setup(){
    // Configurar parámetros iniciales
    k_home = 2.0f;
    k_drag = 1.0f;
    int initialN = 2000;
    
    // Configurar GUI
    gui.setup("Parameters");
    gui.add(nParticlesSlider.setup("N Particles", initialN, 500, 8000));
    gui.add(kHomeSlider.setup("k_home", k_home, 0.5f, 6.0f));
    gui.add(kDragSlider.setup("k_drag", k_drag, 0.5f, 3.0f));
    
    // Inicializar partículas
    initializeParticles(initialN);
}

//--------------------------------------------------------------
void ofApp::update(){
    // Actualizar parámetros desde sliders
    k_home = kHomeSlider;
    k_drag = kDragSlider;
    
    // Detectar cambio en número de partículas
    int targetN = nParticlesSlider;
    if (targetN != (int)particles.size()) {
        resizeParticles(targetN);
    }
    
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
    ofBeginShape(GL_POINTS);
    for (const auto& p : particles) {
        ofVertex(p.pos);
    }
    ofEndShape();
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
void ofApp::drawDebugOverlay() {
    ofSetColor(255, 255, 255);
    stringstream ss;
    ss << "FPS: " << ofGetFrameRate() << endl;
    ss << "Particles: " << particles.size() << endl;
    ss << "k_home: " << k_home << endl;
    ss << "k_drag: " << k_drag << endl;
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

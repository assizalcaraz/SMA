#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "Particle.h"
#include <vector>

class ofApp : public ofBaseApp{

	public:
		void setup() override;
		void update() override;
		void draw() override;
		void exit() override;

		void keyPressed(int key) override;
		void keyReleased(int key) override;
		void mouseMoved(int x, int y ) override;
		void mouseDragged(int x, int y, int button) override;
		void mousePressed(int x, int y, int button) override;
		void mouseReleased(int x, int y, int button) override;
		void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
		void mouseEntered(int x, int y) override;
		void mouseExited(int x, int y) override;
		void windowResized(int w, int h) override;
		void dragEvent(ofDragInfo dragInfo) override;
		void gotMessage(ofMessage msg) override;
		
	private:
		// Partículas
		std::vector<Particle> particles;
		
		// Input mouse (efector)
		struct MouseEfector {
			ofVec2f pos;          // Posición normalizada (0..1)
			ofVec2f pos_prev;     // Posición anterior
			ofVec2f pos_smooth;   // Posición suavizada
			ofVec2f vel;          // Velocidad
			bool active;          // Si el mouse está activo
		} mouse;
		
		// Parámetros físicos
		float k_home;
		float k_drag;
		float k_gesture;         // Fuerza del gesto
		float sigma;             // Radio de influencia (gaussiana)
		float speed_ref;         // Velocidad de referencia
		float smooth_alpha;      // Factor de suavizado (0.1-0.25)
		
		// GUI
		ofxPanel gui;
		ofxIntSlider nParticlesSlider;
		ofxFloatSlider kHomeSlider;
		ofxFloatSlider kDragSlider;
		ofxFloatSlider kGestureSlider;
		ofxFloatSlider sigmaSlider;
		ofxFloatSlider speedRefSlider;
		
		// Funciones auxiliares
		void initializeParticles(int n);
		void resizeParticles(int newN);
		void updateMouseInput();
		void applyGestureForce();
		void drawDebugOverlay();
};

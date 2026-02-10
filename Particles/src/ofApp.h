#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxOsc.h"
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
		
		// Estructura para eventos de hit
		struct HitEvent {
			int id;              // ID de partícula
			float x;              // Posición X normalizada (0..1)
			float y;              // Posición Y normalizada (0..1)
			float energy;         // Energía del impacto (0..1)
			int surface;          // Superficie impactada (0=L, 1=R, 2=T, 3=B, -1=N/A)
		};
		
		// Estructura para rate limiting (token bucket)
		struct RateLimiter {
			float tokens;         // Tokens disponibles
			float rate;          // Tokens por segundo (max_hits_per_second)
			float burst;         // Máximo de tokens acumulados
			int max_per_frame;   // Límite por frame
			int hits_this_frame; // Contador temporal
		};
		
		// Parámetros físicos
		float k_home;
		float k_drag;
		float k_gesture;         // Fuerza del gesto
		float sigma;             // Radio de influencia (gaussiana)
		float speed_ref;         // Velocidad de referencia
		float smooth_alpha;      // Factor de suavizado (0.1-0.25)
		float particleSize;     // Tamaño de partículas en píxeles (1.0-10.0)
		
		// Parámetros de cámara
		float cameraZoom;       // Zoom de cámara (1.0 = sin zoom, >1.0 = zoom in, <1.0 = zoom out)
		float cameraRotation;   // Rotación de cámara en grados
		
		// Parámetros de colisiones
		float restitution;       // Coeficiente de restitución (0.2-0.85)
		float hit_cooldown_ms;   // Cooldown por partícula en ms (20-120)
		float particle_radius;   // Radio de colisión entre partículas (píxeles)
		bool enable_particle_collisions;  // Habilitar/deshabilitar colisiones partícula-partícula
		
		// Parámetros de energía
		float vel_ref;           // Velocidad de referencia para energía (300-1000)
		float dist_ref;          // Distancia de referencia para energía (20-100)
		float energy_a;          // Peso de velocidad en energía (0.5-0.9)
		float energy_b;          // Peso de distancia en energía (0.1-0.5)
		
		// Parámetros de rate limiting
		float max_hits_per_second;  // Máximo de hits por segundo (50-1000)
		float burst;                // Burst máximo (100-1000)
		int max_hits_per_frame;     // Máximo de hits por frame (5-50)
		
		// Rate limiter
		RateLimiter rate_limiter;
		
		// Contadores de debug
		float hits_per_second;           // Promedio móvil de hits/seg
		int hits_discarded_rate;         // Hits descartados por rate limiting
		int hits_discarded_cooldown;      // Hits descartados por cooldown
		int hits_this_second;             // Contador temporal para hits/seg
		float time_accumulator;           // Acumulador de tiempo para hits/seg
		int particles_rendered_this_frame; // Contador de partículas renderizadas en este frame
		
		// Vectores temporales
		std::vector<HitEvent> pending_hits;    // Eventos generados en este frame
		std::vector<HitEvent> validated_hits;  // Eventos validados (para futuro OSC)
		
		// OSC
		ofxOscSender oscSender;
		std::string oscHost;                   // Host destino (default: 127.0.0.1)
		int oscPort;                           // Puerto destino (default: 9000)
		bool oscEnabled;                       // Habilitar/deshabilitar OSC
		float stateSendInterval;              // Intervalo para enviar /state (segundos)
		float stateSendTimer;                 // Timer para /state
		
		// GUI
		ofxPanel gui;
		ofxIntSlider nParticlesSlider;
		ofxFloatSlider kHomeSlider;
		ofxFloatSlider kDragSlider;
		ofxFloatSlider kGestureSlider;
		ofxFloatSlider sigmaSlider;
		ofxFloatSlider speedRefSlider;
		ofxFloatSlider restitutionSlider;
		ofxFloatSlider hitCooldownSlider;
		ofxFloatSlider particleRadiusSlider;
		ofxToggle enableParticleCollisionsToggle;
		ofxFloatSlider velRefSlider;
		ofxFloatSlider distRefSlider;
		ofxFloatSlider energyASlider;
		ofxFloatSlider energyBSlider;
		ofxFloatSlider maxHitsPerSecondSlider;
		ofxFloatSlider burstSlider;
		ofxIntSlider maxHitsPerFrameSlider;
		ofxFloatSlider particleSizeSlider;
		ofxFloatSlider cameraZoomSlider;
		ofxFloatSlider cameraRotationSlider;
		
		// Funciones auxiliares
		void initializeParticles(int n);
		void resizeParticles(int newN);
		void updateMouseInput();
		void applyGestureForce();
		void drawDebugOverlay();
		
		// Funciones de colisiones y eventos
		void checkCollisions();
		void checkParticleCollisions();  // Detectar colisiones entre partículas
		float calculateHitEnergy(Particle& p, int surface);
		float calculateParticleCollisionEnergy(Particle& p1, Particle& p2);
		void generateHitEvent(Particle& p, int surface);
		void generateParticleHitEvent(Particle& p1, Particle& p2, ofVec2f collisionPoint);
		void updateRateLimiter(float dt);
		bool canEmitHit();
		void consumeToken();
		void processPendingHits();
		
		// Funciones de OSC
		void setupOSC();
		void sendHitEvent(const HitEvent& event);
		void sendStateMessage();
		float calculateActivity();            // Calcular actividad normalizada (0..1)
		float calculateGesture();              // Calcular energía de gesto (0..1)
		float calculatePresence();             // Calcular confianza tracking (0..1)
};

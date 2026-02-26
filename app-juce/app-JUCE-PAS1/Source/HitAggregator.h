#pragma once

#include "FusedHitSnapshot.h"

/**
 * Agrega hits por cuadrante en ventanas de 20 ms.
 * Solo se usa en message thread; al cerrar ventana se producen FusedHitSnapshot (hasta 4).
 */
class HitAggregator
{
public:
    static constexpr int NUM_QUADRANTS = 4;
    static constexpr int WINDOW_MS = 20;
    /** Máximo de hits que cuentan para count/heurísticas por cuadrante por ventana; power/centroide siguen sumando. */
    static constexpr int MAX_HITS_PER_QUAD_PER_WINDOW = 64;

    HitAggregator() { reset(); }

    /** Añade un hit (message thread). impactIntensity 0..1, surface 0..3 bordes, -1 p-p. */
    void addHit(float x, float y, float impactIntensity, int surface);

    /**
     * Cierra la ventana actual y escribe en out hasta maxCount snapshots (no vacíos).
     * Returns número de snapshots escritos (0..4).
     */
    int closeWindow(FusedHitSnapshot* out, int maxCount);

    void reset();

private:
    static int quadrantFromXY(float x, float y);

    struct QuadBucket
    {
        double sumE = 0.0;      // E = sum(a_i^2), a_i = pow(impactIntensity, 1.5)
        double sumXW = 0.0;     // sum(x_i * w_i), w_i = a_i^2
        double sumYW = 0.0;
        double sumW = 0.0;
        int count = 0;
        int countEdges = 0;
        int countPP = 0;
    };
    QuadBucket buckets[NUM_QUADRANTS];
};

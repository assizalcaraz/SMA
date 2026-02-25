#include "HitAggregator.h"
#include <cmath>
#include <algorithm>

// Waveform enum order: Noise=0, Sine=1, Square=2, Saw=3, Triangle=4, Click=5, Pulse=6
static constexpr int WAVEFORM_NOISE = 0, WAVEFORM_SINE = 1, WAVEFORM_SQUARE = 2, WAVEFORM_SAW = 3;

int HitAggregator::quadrantFromXY(float x, float y)
{
    int qx = (x >= 0.5f) ? 1 : 0;
    int qy = (y >= 0.5f) ? 1 : 0;
    return qx + 2 * qy;
}

void HitAggregator::addHit(float x, float y, float impactIntensity, int surface)
{
    float a = std::pow(std::max(0.0f, impactIntensity), 1.5f);
    double w = static_cast<double>(a * a);
    int q = quadrantFromXY(x, y);
    if (q < 0 || q >= NUM_QUADRANTS) return;

    QuadBucket& b = buckets[q];
    b.sumE += w;
    b.sumXW += static_cast<double>(x) * w;
    b.sumYW += static_cast<double>(y) * w;
    b.sumW += w;
    if (b.count < MAX_HITS_PER_QUAD_PER_WINDOW)
        b.count++;
    if (surface >= 0 && surface <= 3)
        b.countEdges++;
    else if (surface == -1)
        b.countPP++;
}

int HitAggregator::closeWindow(FusedHitSnapshot* out, int maxCount)
{
    int n = 0;
    for (int q = 0; q < NUM_QUADRANTS && n < maxCount; q++)
    {
        QuadBucket& b = buckets[q];
        if (b.sumW <= 0.0) continue;

        FusedHitSnapshot& s = out[n++];
        s.quadrant = q;
        float E = static_cast<float>(b.sumE);
        float aOut = std::sqrt(E);
        aOut = std::min(1.0f, aOut);

        float xRep = static_cast<float>(b.sumXW / b.sumW);
        float yRep = static_cast<float>(b.sumYW / b.sumW);
        float pan = 2.0f * xRep - 1.0f;
        pan = std::max(-1.0f, std::min(1.0f, pan));
        s.gainL = std::sqrt(0.5f * (1.0f - pan));
        s.gainR = std::sqrt(0.5f * (1.0f + pan));

        s.amplitude = aOut;
        s.metalness = 0.5f;
        s.subOscMix = 0.0f;

        // baseFreq determinista 100-800 Hz desde y_rep (R1)
        float baseFreq = 100.0f + (1.0f - yRep) * 700.0f;
        s.baseFreq = std::max(100.0f, std::min(800.0f, baseFreq));

        // Brightness: más densidad -> más ancho de banda (saturar)
        float brightness = 0.3f + 0.7f * std::min(1.0f, b.count / 20.0f);
        s.brightness = std::max(0.0f, std::min(1.0f, brightness));

        // Damping: ligeramente menor con más densidad (más cuerpo)
        float damping = 0.5f - 0.15f * std::min(1.0f, b.count / 30.0f);
        s.damping = std::max(0.2f, std::min(0.8f, damping));

        // Waveform: más eventos -> más Noise/Saw
        if (b.count >= 15 || aOut > 0.6f)
            s.waveformAsInt = WAVEFORM_SAW;
        else if (b.count >= 8 || aOut > 0.4f)
            s.waveformAsInt = WAVEFORM_SQUARE;
        else if (b.count >= 3 || aOut > 0.2f)
            s.waveformAsInt = WAVEFORM_NOISE;
        else
            s.waveformAsInt = WAVEFORM_SINE;

        // M2/M3: border (0..3) = single source of truth: -3 dB, -3 semitones, optional brightness reduction
        bool predominantEdge = (b.countEdges > b.countPP);
        s.isBorder = predominantEdge;
        if (predominantEdge && aOut > 0.01f)
        {
            s.amplitude *= std::pow(10.0f, -3.0f / 20.0f);  // -3 dB (dbToGain(-3))
            s.baseFreq *= std::pow(2.0f, -3.0f / 12.0f);   // -3 semitones
            s.baseFreq = std::max(100.0f, std::min(800.0f, s.baseFreq));
            s.brightness *= 0.92f;  // optional slight reduction (applied here only)
        }

        // Centro bias opcional (M5): ligeramente más presente cerca del centro
        float dx = xRep - 0.5f;
        float dy = yRep - 0.5f;
        float distFromCenter = std::sqrt(dx * dx + dy * dy);
        if (distFromCenter < 0.35f && s.amplitude > 0.0f)
        {
            float centerBias = 1.0f + 0.08f * (1.0f - distFromCenter / 0.35f);
            s.amplitude = std::min(1.0f, s.amplitude * centerBias);
        }
    }

    reset();
    return n;
}

void HitAggregator::reset()
{
    for (int q = 0; q < NUM_QUADRANTS; q++)
    {
        buckets[q].sumE = 0.0;
        buckets[q].sumXW = 0.0;
        buckets[q].sumYW = 0.0;
        buckets[q].sumW = 0.0;
        buckets[q].count = 0;
        buckets[q].countEdges = 0;
        buckets[q].countPP = 0;
    }
}

# Memoria descriptiva — Trabajo Final ISTR

**Sistema de partículas interactivo y eventos sonoros en tiempo real**  
José Assiz Alcaraz Baxter — Imagen y Sonido en Tiempo Real — Febrero 2026

---

## Objetivo y motivación

El trabajo consiste en una **composición visual** (sistema de partículas en movimiento) que **modifica y produce eventos sonoros** en tiempo real. La motivación fue diseñar un sistema modular donde la imagen y el sonido estén acoplados mediante el protocolo OSC, de modo que la parte visual pueda evaluarse de forma independiente y, a la vez, integrarse con un sintetizador para una performance audiovisual.

---

## Arquitectura del sistema

La arquitectura es **modular**: la aplicación de openFrameworks (módulo Particles) y el sintetizador (JUCE) son procesos separados que se comunican por **OSC sobre UDP** en el puerto 9000. Así se evita acoplar imagen y sonido en un solo ejecutable y se permite reutilizar o sustituir el sintetizador.

**Pipeline:** Input (mouse o, en el futuro, gesto por cámara) → fuerzas aplicadas a las partículas → simulación física → detección de colisiones (bordes y entre partículas) → generación de eventos de impacto → envío de mensajes `/hit` por OSC → sintetizador recibe y genera audio.

---

## Componentes principales

- **Input:** Posición y velocidad del mouse (suavizadas) como efector; previsto como reemplazo futuro el gesto vía MediaPipe/cámara.
- **Física:** Partículas con fuerza de retorno al origen (home), amortiguación (drag) y fuerza de gesto con influencia gaussiana. Integración semi-implícita de Euler. Colisiones con los cuatro bordes y entre partículas, con coeficiente de restitución y cooldown por partícula.
- **Eventos:** Cada colisión válida genera un evento con posición normalizada, energía del impacto y superficie. Un rate limiter global (token bucket) evita saturar el canal OSC.
- **OSC:** Mensaje `/hit` con argumentos (id, x, y, energy, surface) enviado a 127.0.0.1:9000. Opcionalmente se envía `/state` con actividad agregada.
- **Extensiones:** Modo Chladni (tecla SPACE) para auto-organización de partículas en patrones nodales, y control de “placa” que excita el campo y también envía parámetros por OSC al sintetizador.

---

## Decisiones de diseño

Se priorizó tener un **loop completo** (partículas → colisiones → OSC) antes que integrar tracking por cámara. Por eso el **mouse es el input principal** de la entrega y MediaPipe queda como mejora opcional; esto permite demostrar y evaluar el sistema aunque no se disponga de webcam o haya problemas con el tracking. 

Los principales desafíos fueron: (1) calibrar fuerzas y rate limiting para que hubiera suficientes eventos sonoros sin saturar; (2) definir un contrato OSC estable (`/hit` con cinco argumentos) para no cambiar el protocolo en cada iteración; (3) incorporar colisiones partícula-partícula y el modo Chladni sin romper el comportamiento base.

---

## Resultados y conclusiones

Se obtuvo una aplicación estable que muestra un sistema de partículas reactivo al mouse, con colisiones en bordes y entre partículas, envío de eventos por OSC y documentación (manual de parámetros, especificación del mensaje `/hit`). El sistema produce eventos sonoros a partir de la composición visual.

**Mejoras post-evaluación (v0.4):** Tras una evaluación externa se implementaron correcciones de rendimiento y robustez: (1) **dt centralizado** — un único paso de tiempo por frame con clamp para estabilidad; (2) **grid espacial** para colisiones partícula–partícula (broad-phase 2D fijo, vecindario 3×3), reduciendo el coste de O(N²) a O(N) esperado y permitiendo escalar hasta N≈8000; (3) **resolución de impulso** con velocidades actuales y corrección posicional (slop + percent), con reglas de estabilidad obligatorias; (4) **campo Chladni precomputado** — grid 2D de ∇E muestreado por partícula (bilinear), con flag `plateDirty` y rebuild por debounce (50 ms); (5) **render con VBO** — mesh preasignado, actualización vía `updateVertexData()`, y shader de vértice para `gl_PointSize`; (6) **instrumentación obligatoria** — overlay con FPS, tiempos por subsistema (update, colisiones, plate, draw), parejas en narrow-phase, colisiones resueltas y tasas OSC. La documentación (manual, memoria) se actualizó para reflejar estos cambios.

Posibles mejoras: integración de MediaPipe para control por gesto, ajuste fino del mapeo energía–sonido en el sintetizador y una sesión de calibración conjunta entre la app visual y el sintetizador para una performance más pulida.

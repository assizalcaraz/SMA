# Análisis de Renderizado de Partículas

**Fecha:** 2026-01-20  
**Rama:** SMA_v0.2

---

## Problema Reportado

Se reporta que a partir de 2000 partículas, no se renderizan realmente esa cantidad. El sistema se vuelve pesado y lento arriba de 3500 partículas sin mejorar la densidad de la visualización. Se asume que las partículas existen pero no se renderizan.

---

## Análisis del Código Actual

### Ubicación del Renderizado

**Archivo:** `Particles/src/ofApp.cpp`  
**Función:** `ofApp::draw()` (líneas 191-211)

### Implementación Actual

```cpp
void ofApp::draw(){
    // Render de partículas como puntos
    ofSetColor(255, 255, 255);
    glPointSize(2.0f);
    glEnable(GL_POINT_SMOOTH);
    glBegin(GL_POINTS);
    for (const auto& p : particles) {
        glVertex2f(p.pos.x, p.pos.y);
    }
    glEnd();
    glDisable(GL_POINT_SMOOTH);
}
```

### Observaciones

1. **No hay limitación explícita**: El código renderiza todas las partículas en el vector `particles`
2. **Renderizado directo**: Usa `glBegin(GL_POINTS)` con un loop simple
3. **Tamaño fijo**: `glPointSize(2.0f)` es constante

---

## Hipótesis del Problema

### Hipótesis 1: Problema de Rendimiento (FPS)

**Más probable**: El problema no es que las partículas no se rendericen, sino que:
- Con muchas partículas (>2000), el loop de renderizado es costoso
- El FPS baja significativamente
- La percepción visual es que hay menos partículas porque el sistema está lento
- Las partículas se mueven más lento, dando sensación de menor densidad

**Evidencia esperada:**
- FPS bajo con muchas partículas
- Todas las partículas se renderizan, pero el sistema es lento

### Hipótesis 2: Limitaciones de OpenGL

**Posible**: `glPointSize()` tiene límites dependiendo del hardware:
- Algunos drivers limitan el tamaño máximo de puntos
- Puede haber límites en el número de primitivas por frame
- Límites de ancho de banda de GPU

**Evidencia esperada:**
- Límites específicos de hardware
- Comportamiento diferente en diferentes sistemas

### Hipótesis 3: Overlap Visual

**Posible**: Con muchas partículas pequeñas (2px), hay mucho overlap:
- Partículas superpuestas no se distinguen visualmente
- La densidad visual no aumenta proporcionalmente al número de partículas
- Sensación de que hay menos partículas de las que realmente hay

**Evidencia esperada:**
- Contador muestra todas las partículas renderizadas
- Pero densidad visual no aumenta proporcionalmente

---

## Implementación de Diagnóstico

### Contadores Agregados

Se agregaron contadores en el debug overlay para verificar:

1. **Total de partículas**: `particles.size()`
2. **Partículas renderizadas**: Contador en cada frame
3. **FPS**: Para medir rendimiento

### Código de Diagnóstico

```cpp
// En ofApp.h
int particles_rendered_this_frame;

// En draw()
int rendered_count = 0;
for (const auto& p : particles) {
    glVertex2f(p.pos.x, p.pos.y);
    rendered_count++;
}
particles_rendered_this_frame = rendered_count;

// En drawDebugOverlay()
ss << "Particles (total): " << particles.size() << endl;
ss << "Particles (rendered): " << particles_rendered_this_frame << " / " << particles.size() << endl;
```

---

## Pruebas Recomendadas

### Test 1: Verificar Renderizado

1. Ejecutar con 2000 partículas
2. Verificar en debug overlay: `Particles (rendered)` vs `Particles (total)`
3. Si son iguales → problema es de rendimiento, no de renderizado
4. Si son diferentes → hay limitación en el renderizado

### Test 2: Medir FPS

1. Probar con diferentes cantidades:
   - 500 partículas
   - 1000 partículas
   - 2000 partículas
   - 3500 partículas
   - 5000 partículas
2. Registrar FPS en cada caso
3. Verificar si FPS baja significativamente

### Test 3: Verificar Densidad Visual

1. Comparar densidad visual con diferentes cantidades
2. Verificar si la densidad aumenta proporcionalmente
3. Considerar overlap de partículas pequeñas

---

## Posibles Soluciones

### Si el problema es rendimiento (FPS bajo)

1. **Frustum Culling**: Solo renderizar partículas visibles
2. **Level of Detail (LOD)**: Reducir detalle con muchas partículas
3. **Instancing**: Usar instancing de OpenGL para renderizado más eficiente
4. **Culling por distancia**: No renderizar partículas muy lejanas
5. **Límite de renderizado**: Limitar número de partículas renderizadas (último recurso)

### Si el problema es overlap visual

1. **Tamaño variable**: Implementar tamaño de partículas variable
2. **Transparencia**: Usar alpha blending para ver overlap
3. **Color por densidad**: Colorear según densidad local

### Si el problema es limitación de OpenGL

1. **Verificar límites**: Consultar documentación de OpenGL
2. **Alternativas**: Usar VBOs o instancing en lugar de `glBegin(GL_POINTS)`

---

## Próximos Pasos

1. ✅ Agregar contadores de diagnóstico
2. ⏳ Ejecutar pruebas con diferentes cantidades de partículas
3. ⏳ Documentar resultados de FPS y renderizado
4. ⏳ Implementar optimizaciones si es necesario

---

## Referencias

- [openFrameworks - Rendering](https://openframeworks.cc/documentation/)
- [OpenGL Point Rendering](https://www.khronos.org/opengl/wiki/Primitive)

---

**Estado**: Diagnóstico implementado, pendiente pruebas

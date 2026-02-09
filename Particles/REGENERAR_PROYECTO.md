# Instrucciones para regenerar el proyecto Particles

El proyecto Xcode tiene referencias persistentes a addons que no se usan (`ofxOpenCv`, `ofxOsc`). La mejor solución es regenerar el proyecto usando el Project Generator de openFrameworks.

## Pasos para regenerar:

1. **Abrir Project Generator de openFrameworks:**
   - Navegar a `/Users/joseassizalcarazbaxter/Developer/of_v0.12.1_osx_release/projectGenerator`
   - Ejecutar `projectGenerator.app`

2. **Configurar el proyecto:**
   - **Project Path:** `/Users/joseassizalcarazbaxter/Documents/UNA/POSGRADO/2025_2/Sistema Modular Audiovisual/Particles`
   - **Project Name:** `Particles`
   - **Addons:** Seleccionar SOLO `ofxGui` (desmarcar todos los demás)

3. **Regenerar:**
   - Click en "Update" o "Generate"
   - Esto regenerará el proyecto Xcode con solo los addons seleccionados

4. **Verificar:**
   - Abrir `Particles.xcodeproj` en Xcode
   - Verificar que compila sin errores
   - El archivo `addons.make` ya está correcto (solo `ofxGui`)

## Alternativa: Limpiar build de Xcode

Si prefieres no regenerar, intenta:

1. En Xcode: `Product > Clean Build Folder` (Cmd+Shift+K)
2. Cerrar y reabrir Xcode
3. Intentar compilar de nuevo

Si el error persiste, la regeneración es la solución más confiable.

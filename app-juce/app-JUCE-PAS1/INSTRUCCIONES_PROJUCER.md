# Instrucciones para Regenerar Proyecto en Projucer

## Proceso Paso a Paso

### 1. Abrir Projucer
- Abre la aplicación Projucer
- File → Open
- Navega a: `app-juce/app-JUCE-PAS1/app-JUCE-PAS1.jucer`
- Abre el archivo

### 2. Verificar Módulo OSC
- En el panel izquierdo, selecciona "Modules"
- Verifica que `juce_osc` esté en la lista y marcado (checkbox activo)
- Si no está:
  - Haz clic en "+" o "Add Module"
  - Busca `juce_osc` en la lista
  - Selecciónalo y agrégalo

### 3. Verificar Configuración de Módulos
Para cada módulo (especialmente `juce_osc`):
- `showAllCode` = ON (checkbox marcado)
- `useLocalCopy` = OFF
- `useGlobalPath` = ON

### 4. Verificar Export Targets
- En el panel izquierdo, selecciona "Exporters"
- Verifica que "Xcode (macOS)" esté presente
- Si no está: File → New Export → Xcode (macOS)

### 5. Guardar y Regenerar
**IMPORTANTE: Hacer ambos pasos en orden**

1. **File → Save** (Cmd+S)
   - Guarda el archivo `.jucer`
   - Esto actualiza la configuración

2. **File → Save and Open in IDE** (Cmd+E)
   - Regenera todos los archivos del proyecto
   - Actualiza `JuceLibraryCode/JuceHeader.h`
   - Abre Xcode automáticamente

### 6. Verificar en Xcode
Después de regenerar, verifica:

1. Abre `JuceLibraryCode/JuceHeader.h`
2. Busca la línea:
   ```cpp
   #include <juce_osc/juce_osc.h>
   ```
3. Si NO está presente, vuelve a Projucer y repite el Paso 5

### 7. Limpiar y Recompilar
En Xcode:
1. **Product → Clean Build Folder** (Shift+Cmd+K)
2. **Product → Build** (Cmd+B)
3. Verifica que compile sin errores

## Verificación Final

Después de regenerar, el código debería:
- ✅ Compilar sin errores de `OSCReceiver`, `OSCMessage`, etc.
- ✅ Reconocer `oscReceiver.addListener(this)`
- ✅ Los callbacks `oscMessageReceived()` deberían funcionar

## Si Sigue Sin Funcionar

Si después de regenerar `addListener()` sigue sin existir:

1. Verifica la versión de JUCE: Help → About Projucer
2. En JUCE 8.0.12, la API puede ser diferente
3. Revisa la documentación de JUCE para `OSCReceiver::Listener`

## Troubleshooting

### Error: "No member named 'OSCReceiver'"
- El módulo no está incluido en `JuceHeader.h`
- Solución: Repetir Paso 5 (Save and Open in IDE)

### Error: "addListener doesn't exist"
- La API de OSCReceiver puede ser diferente en tu versión
- Solución: Verificar documentación de JUCE o usar alternativa

### Error: Compilación falla después de regenerar
- Limpia el build folder (Paso 7)
- Verifica que todos los módulos estén correctamente configurados

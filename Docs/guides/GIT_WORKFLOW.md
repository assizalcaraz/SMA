# Guía: Workflow Git

Esta guía documenta el workflow de control de versiones utilizado en el proyecto.

---

## Workflow /dia

El proyecto utiliza el workflow `/dia` para gestión de sesiones de desarrollo con Cursor/IA.

### Commits de Cursor/IA

**Formato de mensaje:**
```bash
git-commit-cursor -m "🦾 tipo: mensaje"
```

**Características:**
- Emoji 🦾 al inicio para identificación rápida en `git log`
- NO incluir `[dia]` en el mensaje (confunde)
- Ejemplo: `🦾 feat: agregar validación de datos [#sesion S01]`
- Autoría automática: `Cursor Assistant <cursor@dia.local>`

### Commits manuales del usuario

El usuario usa: `git -M "mensaje"` (sin emoji, autoría normal)

### Identificación en git log

- `Cursor Assistant <cursor@dia.local>` + 🦾 = commit de Cursor
- Autoría del usuario + sin 🦾 = commit manual

### Comandos /dia

- `dia start`: inicia sesión
- `dia pre-feat`: sugiere commit (usa `git-commit-cursor`)
- `dia end`: cierra sesión

**Nota:** NO ejecutar commits automáticamente, solo sugerir.

---

## Convenciones de Commits

### Tipos de commits

- `feat`: Nueva funcionalidad
- `fix`: Corrección de bug
- `docs`: Cambios en documentación
- `refactor`: Refactorización de código
- `perf`: Mejoras de rendimiento
- `test`: Agregar o modificar tests
- `chore`: Tareas de mantenimiento

### Ejemplos

```bash
🦾 feat: agregar control de tamaño de partículas
🦾 fix: corregir cálculo de energía en colisiones
🦾 docs: actualizar manual con nuevos parámetros
🦾 perf: optimizar renderizado de partículas
```

---

## Ramas

### Ramas principales

- `main` / `master`: Rama principal estable
- `synth`: Rama de desarrollo del sintetizador
- `SMA_v0.2`: Rama de versión 0.2 con mejoras

### Crear nueva rama

```bash
git checkout -b nombre-rama
```

### Trabajar en rama

```bash
# Crear rama desde rama actual
git checkout -b nueva-rama

# Trabajar en la rama
# ... hacer cambios ...

# Hacer commit
git-commit-cursor -m "🦾 feat: nueva funcionalidad"
```

---

## Estado del Repositorio

### Ver estado actual

```bash
git status
```

Muestra:
- Archivos modificados
- Archivos en staging
- Archivos sin seguimiento
- Cambios no commiteados

### Ver historial

```bash
# Últimos 20 commits
git log --oneline -20

# Ver todos los commits
git log --oneline --all
```

### Identificar commits de Cursor

```bash
# Filtrar commits de Cursor
git log --author="Cursor Assistant"
```

---

## Buenas Prácticas

1. **Commits frecuentes**: Hacer commits pequeños y frecuentes
2. **Mensajes descriptivos**: Usar mensajes claros que expliquen el cambio
3. **Separar cambios**: Hacer commits separados para código y documentación
4. **Revisar antes de commit**: Usar `git status` y `git diff` antes de commitear
5. **No commitear archivos temporales**: Verificar `.gitignore` está actualizado

---

## Referencias

- [Scopes de documentación](SCOPES_DOCUMENTACION.md) - Guía para actualizar documentación
- [Plan de implementación](../specs/PLAN_IMPLEMENTACION.md) - Sección de versionado

---

**Última actualización**: 2026-01-20

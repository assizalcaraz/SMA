#!/bin/bash
# Script para limpiar el build del proyecto Particles
# Uso: ./clean_build.sh

echo "Limpiando build del proyecto Particles..."

# Eliminar ejecutable en bin/
if [ -d "bin/ParticlesDebug.app" ]; then
    echo "Eliminando bin/ParticlesDebug.app..."
    rm -rf bin/ParticlesDebug.app
fi

# Eliminar objetos compilados si existen
if [ -d "obj" ]; then
    echo "Eliminando obj/..."
    rm -rf obj
fi

# Limpiar Derived Data de Xcode (opcional, descomentar si es necesario)
# rm -rf ~/Library/Developer/Xcode/DerivedData/Particles-*

echo "Limpieza completada."
echo ""
echo "Nota: La carpeta bin/data/ se mantiene (contiene assets del proyecto)."
echo "Para limpiar completamente, ejecuta: rm -rf bin/*"

#!/usr/bin/env python3
"""
Script de prueba para comunicación OSC
Envía mensajes OSC simulados para probar App B (JUCE) sin necesidad de App A (oF)

Uso:
    python scripts/test-osc.py
    python scripts/test-osc.py --mode random
    python scripts/test-osc.py --mode sequence --count 10
    python scripts/test-osc.py --mode stress --rate 200
"""

import argparse
import time
import random
import sys

try:
    from pythonosc import udp_client
except ImportError:
    print("Error: python-osc no está instalado.")
    print("Instalar con: pip install python-osc")
    sys.exit(1)


# Configuración por defecto
DEFAULT_HOST = "127.0.0.1"
DEFAULT_PORT = 9000
DEFAULT_MODE = "interactive"


def send_hit(client, particle_id, x, y, energy, surface):
    """Envía un mensaje OSC /hit"""
    client.send_message("/hit", [particle_id, x, y, energy, surface])
    print(f"Enviado /hit: id={particle_id}, x={x:.2f}, y={y:.2f}, energy={energy:.2f}, surface={surface}")


def send_state(client, activity, gesture, presence):
    """Envía un mensaje OSC /state"""
    client.send_message("/state", [activity, gesture, presence])
    print(f"Enviado /state: activity={activity:.2f}, gesture={gesture:.2f}, presence={presence:.2f}")


def mode_interactive(client):
    """Modo interactivo: el usuario presiona Enter para enviar hits"""
    print("\n=== Modo Interactivo ===")
    print("Presiona Enter para enviar un hit aleatorio")
    print("Escribe 'state' y Enter para enviar estado")
    print("Escribe 'q' y Enter para salir\n")
    
    particle_id = 0
    
    while True:
        try:
            user_input = input("> ").strip().lower()
            
            if user_input == 'q':
                break
            elif user_input == 'state':
                send_state(client, 
                          random.uniform(0.0, 1.0),
                          random.uniform(0.0, 1.0),
                          random.uniform(0.5, 1.0))
            else:
                # Enviar hit aleatorio
                x = random.uniform(0.0, 1.0)
                y = random.uniform(0.0, 1.0)
                energy = random.uniform(0.1, 1.0)
                surface = random.choice([0, 1, 2, 3])
                
                send_hit(client, particle_id, x, y, energy, surface)
                particle_id += 1
                
        except KeyboardInterrupt:
            print("\nInterrumpido por usuario")
            break


def mode_random(client, count=10, interval=0.1):
    """Modo aleatorio: envía hits aleatorios automáticamente"""
    print(f"\n=== Modo Aleatorio ===")
    print(f"Enviando {count} hits aleatorios con intervalo de {interval}s\n")
    
    for i in range(count):
        x = random.uniform(0.0, 1.0)
        y = random.uniform(0.0, 1.0)
        energy = random.uniform(0.1, 1.0)
        surface = random.choice([0, 1, 2, 3])
        
        send_hit(client, i, x, y, energy, surface)
        time.sleep(interval)
    
    print(f"\nEnviados {count} hits")


def mode_sequence(client, count=10, interval=0.1):
    """Modo secuencia: envía hits en secuencia predecible"""
    print(f"\n=== Modo Secuencia ===")
    print(f"Enviando {count} hits en secuencia con intervalo de {interval}s\n")
    
    surfaces = [0, 1, 2, 3]  # L, R, T, B
    
    for i in range(count):
        # Secuencia predecible
        x = (i % 10) / 10.0
        y = ((i // 10) % 10) / 10.0
        energy = 0.3 + (i % 7) * 0.1
        surface = surfaces[i % len(surfaces)]
        
        send_hit(client, i, x, y, energy, surface)
        time.sleep(interval)
    
    print(f"\nEnviados {count} hits en secuencia")


def mode_stress(client, rate=200, duration=5.0):
    """Modo estrés: envía hits a alta velocidad para probar rendimiento"""
    print(f"\n=== Modo Estrés ===")
    print(f"Enviando hits a {rate} hits/s durante {duration}s\n")
    print("Presiona Ctrl+C para detener antes\n")
    
    particle_id = 0
    interval = 1.0 / rate
    start_time = time.time()
    hits_sent = 0
    
    try:
        while time.time() - start_time < duration:
            x = random.uniform(0.0, 1.0)
            y = random.uniform(0.0, 1.0)
            energy = random.uniform(0.1, 1.0)
            surface = random.choice([0, 1, 2, 3])
            
            send_hit(client, particle_id, x, y, energy, surface)
            particle_id += 1
            hits_sent += 1
            
            time.sleep(interval)
            
    except KeyboardInterrupt:
        pass
    
    elapsed = time.time() - start_time
    actual_rate = hits_sent / elapsed if elapsed > 0 else 0
    
    print(f"\nEnviados {hits_sent} hits en {elapsed:.2f}s")
    print(f"Tasa real: {actual_rate:.1f} hits/s")


def mode_demo(client):
    """Modo demo: simula una sesión de interacción típica"""
    print("\n=== Modo Demo ===")
    print("Simulando sesión de interacción típica\n")
    
    # Fase 1: Inicio lento
    print("Fase 1: Inicio lento (pocos hits)")
    for i in range(5):
        x = random.uniform(0.3, 0.7)
        y = random.uniform(0.3, 0.7)
        energy = random.uniform(0.1, 0.3)
        surface = random.choice([0, 1, 2, 3])
        send_hit(client, i, x, y, energy, surface)
        time.sleep(0.5)
    
    # Fase 2: Actividad moderada
    print("\nFase 2: Actividad moderada")
    for i in range(5, 15):
        x = random.uniform(0.2, 0.8)
        y = random.uniform(0.2, 0.8)
        energy = random.uniform(0.3, 0.7)
        surface = random.choice([0, 1, 2, 3])
        send_hit(client, i, x, y, energy, surface)
        time.sleep(0.2)
    
    # Fase 3: Alta actividad
    print("\nFase 3: Alta actividad (muchos hits)")
    for i in range(15, 35):
        x = random.uniform(0.0, 1.0)
        y = random.uniform(0.0, 1.0)
        energy = random.uniform(0.5, 1.0)
        surface = random.choice([0, 1, 2, 3])
        send_hit(client, i, x, y, energy, surface)
        time.sleep(0.1)
    
    # Enviar estado final
    print("\nEnviando estado final")
    send_state(client, 0.75, 0.6, 0.9)
    
    print("\nDemo completada")


def main():
    parser = argparse.ArgumentParser(
        description="Script de prueba para comunicación OSC",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Modos disponibles:
  interactive  - Modo interactivo (presionar Enter para enviar hits)
  random       - Envía hits aleatorios automáticamente
  sequence     - Envía hits en secuencia predecible
  stress       - Test de estrés a alta velocidad
  demo         - Simula sesión de interacción típica

Ejemplos:
  python scripts/test-osc.py
  python scripts/test-osc.py --mode random --count 20
  python scripts/test-osc.py --mode stress --rate 200 --duration 10
        """
    )
    
    parser.add_argument(
        "--host",
        default=DEFAULT_HOST,
        help=f"Host destino (default: {DEFAULT_HOST})"
    )
    
    parser.add_argument(
        "--port",
        type=int,
        default=DEFAULT_PORT,
        help=f"Puerto destino (default: {DEFAULT_PORT})"
    )
    
    parser.add_argument(
        "--mode",
        choices=["interactive", "random", "sequence", "stress", "demo"],
        default=DEFAULT_MODE,
        help="Modo de operación (default: interactive)"
    )
    
    parser.add_argument(
        "--count",
        type=int,
        default=10,
        help="Número de hits a enviar (modos random y sequence, default: 10)"
    )
    
    parser.add_argument(
        "--interval",
        type=float,
        default=0.1,
        help="Intervalo entre hits en segundos (modos random y sequence, default: 0.1)"
    )
    
    parser.add_argument(
        "--rate",
        type=int,
        default=200,
        help="Tasa de hits por segundo (modo stress, default: 200)"
    )
    
    parser.add_argument(
        "--duration",
        type=float,
        default=5.0,
        help="Duración en segundos (modo stress, default: 5.0)"
    )
    
    args = parser.parse_args()
    
    # Crear cliente OSC
    print(f"Conectando a {args.host}:{args.port}")
    try:
        client = udp_client.SimpleUDPClient(args.host, args.port)
        print("Conectado exitosamente\n")
    except Exception as e:
        print(f"Error al conectar: {e}")
        sys.exit(1)
    
    # Ejecutar modo seleccionado
    if args.mode == "interactive":
        mode_interactive(client)
    elif args.mode == "random":
        mode_random(client, args.count, args.interval)
    elif args.mode == "sequence":
        mode_sequence(client, args.count, args.interval)
    elif args.mode == "stress":
        mode_stress(client, args.rate, args.duration)
    elif args.mode == "demo":
        mode_demo(client)
    
    print("\nFinalizado")


if __name__ == "__main__":
    main()

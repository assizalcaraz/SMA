# Guion de narración para el video demo (≤1 minuto)

La explicación debe ir **al principio del video**, en un máximo de 1 minuto. Después puede continuar la captura de la aplicación en funcionamiento sin narración.

---

## Guion largo (aprox. 45–60 s)

| Sección   | Tiempo | Texto |
|-----------|--------|--------|
| Apertura  | ≈10 s  | Este es el trabajo final de Imagen y Sonido en Tiempo Real: un sistema en el que una composición visual —un sistema de partículas— modifica y produce eventos sonoros en tiempo real. |
| Pipeline  | ≈15 s  | El movimiento del mouse —o en una versión futura, el gesto capturado por cámara— actúa como fuerza sobre las partículas. Esas partículas tienen física de retorno al origen y rebote. Cuando chocan con los bordes o entre sí, se generan eventos de impacto. |
| Comunicación | ≈10 s | Cada impacto se envía por OSC al sintetizador en el puerto 9000. El mensaje incluye posición, energía del golpe y qué borde se tocó. El sintetizador convierte eso en sonido en tiempo real. |
| Demo      | ≈10 s  | Ahora se ve la aplicación en funcionamiento: las partículas reaccionan al mouse, chocan en los bordes y se escucha el resultado en el sintetizador. También hay un modo Chladni donde las partículas se organizan en patrones nodales y un control de placa que excita el campo; ambos envían OSC para controlar el sonido. |
| Cierre    | ≈5 s   | El código, la memoria descriptiva y este video son los entregables del trabajo. |

---

## Guion corto (aprox. 30–40 s)

*Una sola toma:*

«Trabajo final ISTR: composición visual que produce sonido en tiempo real. Partículas controladas por mouse —o gesto— chocan en los bordes; cada impacto se envía por OSC al sintetizador. Así la imagen y el sonido están acoplados. A continuación, la demo en ejecución.»

---

## Notas para la grabación

- Mostrar en pantalla la ventana de **Particles** (y opcionalmente la del sintetizador o una mezcla de ambas).
- Si se muestra el modo Chladni (tecla SPACE) o los sliders de la placa, se puede usar la versión larga del guion para mencionarlos.
- Mantener la explicación hablada **dentro del primer minuto**; el resto del video puede ser solo demo sin voz.

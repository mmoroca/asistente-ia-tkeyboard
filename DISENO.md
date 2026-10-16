# Asistente IA para T-Keyboard: diseño (v0)

Estado: diseño confirmado por el usuario. Pendiente: que el usuario cree su propia cuenta y clave de Groq y pruebe la clave. No hay código todavía.

## Objetivo

Asistente de texto en el T-Keyboard: el usuario escribe con el teclado y recibe respuestas del modelo de Groq en la pantalla de 40×160 px. Solo texto, sin voz.

## Hardware y restricciones

- ESP32-C3, sin PSRAM. Memoria para TLS, JSON y el historial: hay que limitar el tamaño de todo.
- Pantalla 40 px de alto: caben 2 líneas de texto de 16 px.
- Sin batería fiable (ver decisión de batería). Funciona con cable USB.

## Proveedor

- **Groq** (API compatible con OpenAI), plan gratuito sin tarjeta.
- Modelo por defecto: `openai/gpt-oss-120b`. Alternativa más rápida: `openai/gpt-oss-20b`.
- Límites del plan gratuito (según fuentes de septiembre de 2026, verificar en la consola de Groq): 30 peticiones por minuto y 1.000 al día para gpt-oss-120b.
- No usar Llama 3.3 70B: Groq lo retiró el 16 de agosto de 2026.
- Endpoints: chat `POST https://api.groq.com/openai/v1/chat/completions`. Comprobación de la clave: `GET https://api.groq.com/openai/v1/models` (sin gastar tokens).

## Configuración (portal, reutilizado del reloj)

- Punto de acceso `AsistenteIA-XXXX`, formulario en 192.168.4.1.
- Campos: red WiFi (SSID), contraseña WiFi, clave de API de Groq (campo de contraseña).
- Al guardar: prueba la red y la clave antes de guardar. Si todo va bien, guarda y reinicia. Si no, muestra el error y sigue en el portal.
- Entrar en el portal: si no hay configuración guardada, o manteniendo ALT al encender.
- La clave de API se guarda en `Preferences` (flash, sin cifrar), igual que la contraseña WiFi del reloj.

## Seguridad de la conexión

- TLS con validación del certificado (bundle de certificados del ESP32). No usar `setInsecure()`, porque con una clave de API expondría la clave.

## Sesión

- Al arrancar, la sesión empieza en blanco. Al reiniciar se borra el historial (solo en RAM).
- Tras una conexión correcta, el usuario recibe: «¿En qué puedo ayudarte?» como mensaje recibido.
- El historial enviado al modelo se limita a los últimos mensajes (p. ej. 8) para no agotar la memoria.
- El mensaje de sistema pide respuestas breves en español. Se usa `max_tokens` bajo (p. ej. 200).

## Teclado

Teclas de control (posiciones de la matriz, según `example/BLE_keybord/BLE_keybord.ino`):

| Función | Tecla | Posición (col, fila) |
|---|---|---|
| Mayúsculas / desplazar | ⇧ izquierda | (1, 6) |
| Mayúsculas / desplazar | ⇧ derecha | (2, 3) |
| Símbolos (SYM) | sym | (0, 2) |
| Acentos (ALT) | alt | (0, 4) |
| Enviar | ↵ | (3, 3) |
| Borrar | ⌫ | (4, 3) |
| Espacio | espacio | (0, 5) |

- **Símbolos (SYM)**: cada tecla escribe el símbolo impreso en ella. La tabla `keyboard_symbol[][]` del ejemplo coincide con la foto del teclado en todos los casos comprobados.
- **Acentos (ALT)**, propuesta:
  - ALT + a, e, i, o, u → á, é, í, ó, ú.
  - ALT + Mayús + vocal → Á, É, Í, Ó, Ú.
  - ALT + n → ñ, ALT + Mayús + n → Ñ.
  - ALT + v → ¿ (la v es la tecla de «?»), ALT + b → ¡ (la b es la tecla de «!»).
  - Hay que evitar ALT + b para la retroiluminación (ejemplo oficial), porque ahora la hace la combinación de ⇧.
- **Mayúsculas**: ⇧ mantenido + tecla. Sin bloqueo de mayúsculas en la primera versión.
- **Desplazar**: pulsar y soltar ⇧ izquierda sube una línea; pulsar y soltar ⇧ derecha baja una línea. Solo cuenta si la tecla se suelta sin haber escrito nada con ella.
- **Retroiluminación**: ⇧ izquierda + ⇧ derecha a la vez. Mientras las dos estén pulsadas no se desplaza.
- Cualquier tecla de texto vuelve a la parte inferior del chat.

## Pantalla

- Vista única tipo registro: los mensajes van uno debajo de otro y la línea de escritura es la última.
- Dos líneas visibles de 18 caracteres (8 px por carácter). Ancho de texto: x = 8..151.
- Icono a la izquierda del primer renglón de cada mensaje: **↑ enviado**, **↓ recibido**. No hay una convención oficial; los dibujo como bitmap de 8×8 porque la fuente no tiene flechas.
- Barra de desplazamiento vertical en x = 156..159, con la posición del historial.
- Mientras espera la respuesta: indicador «…» en la línea de respuesta.
- Errores (red, clave, límite diario): mensaje en la pantalla y se puede seguir escribiendo.

## Pantalla apagada

- 15 s sin pulsar ninguna tecla: la pantalla se apaga para ahorrar batería. Cualquier tecla la enciende de nuevo.
- Confirmado por el usuario. La conexión WiFi y el historial no se tocan al apagar la pantalla.

## Idioma de las respuestas

- Campo en el portal: **Idioma de las respuestas**, con tres opciones:
  - **Automático** (por defecto): responde en el idioma de la pregunta.
  - **Español**: fija el idioma, aunque el usuario escriba en otro.
  - **English**: fija el idioma en inglés.
- Se implementa en el mensaje de sistema. Con Automático, el mensaje dice «responde en el mismo idioma que el usuario». Con un idioma fijo, dice «responde siempre en <idioma>».
- Al escribir en inglés no hace falta nada especial: el teclado tiene las mismas letras que el inglés. Los acentos solo se usan en español.

## Decisiones confirmadas

1. Modelo: `openai/gpt-oss-120b` (mejor español). Confirmado.
2. Mapa de acentos (ALT) con ¿ en ALT+v y ¡ en ALT+b. Confirmado.
3. ⇧ tocada sin escribir desplaza; ⇧ mantenida da mayúsculas; sin bloqueo de mayúsculas. Confirmado.
4. Pantalla apagada a los 15 s sin pulsar ninguna tecla. Confirmado.
5. Respuestas breves en el mensaje de sistema. Confirmado.
6. Idioma: Automático por defecto, con opción de fijar español o inglés en el portal (propuesta pendiente de confirmar).

## Acuerdos de cuenta y privacidad

- Cada usuario del asistente crea su propia cuenta en Groq y extrae su propia clave de API. El firmware no lleva ninguna clave fija: la clave se introduce en el portal de configuración, igual que la red WiFi.
- El usuario acepta el riesgo de privacidad del plan gratuito de Groq, que es responsabilidad de quien usa el asistente.
- Límites del plan gratuito: suficientes para un mini asistente personal.

## Estado del proyecto

- Base: `platformio.ini` (espressif32 @ 6.1, ttgo-t-oi-plus, ArduinoJson), `partitions_asistente.csv`, `.gitignore`, `lib/TFT_GC9D01N-Patched/`.
- `src/groq_ca.h`: certificado raíz GTS Root R4 (cadena de api.groq.com: WE1 -> GTS Root R4). Se usa con `setCACert`, sin `setInsecure`. Vigente hasta 2036. Si Groq cambia de autoridad certificadora, hay que sustituirlo.
- `src/main.cpp` (v1): compila con `pio run` (RAM 18 %, Flash 23 %). Incluye:
  - Portal `AsistenteIA-XXXX` con red, contraseña WiFi y clave de Groq. Prueba la red y luego la clave (`GET /openai/v1/models`) antes de guardar. Guarda en Preferences (`asistente`) y reinicia.
  - Chat: `POST /openai/v1/chat/completions` con `openai/gpt-oss-120b`, `max_tokens` 300, `reasoning_effort` low. Historial de 8 mensajes.
  - Teclado: ALT para acentos, SYM de un solo uso, ⇧ para mayúsculas y desplazamiento, ⇧+⇧ para retroiluminación, ↵ envía, ⌫ borra.
  - Pantalla: 2 líneas de 18 caracteres, iconos ↑ (cian) y ↓ (verde), barra de desplazamiento, apagado a los 15 s.
  - Pruebas en el PC (solo texto): división en líneas y limpieza de caracteres.
- Desviaciones respecto al diseño:
  - Sin icono WiFi en el chat: ocupa la zona de texto de la segunda línea. Se muestra el error «Sin WiFi» al enviar.
  - `max_tokens` 300 (en vez de 200), porque el modelo razona antes de contestar.
  - La llamada a Groq bloquea el teclado durante la espera (5 a 20 s). Se puede pasar a una tarea aparte si molesta.
  - La hora por NTP es necesaria para TLS: sin hora, el chat dice «Sincronizando la hora».
- Pendiente de probar en la placa: iconos (orientación), teclas, respuesta real de Groq y el portal.
- Pendiente del usuario: subir el firmware y probar.

## Cambios v2

- Fuente del chat: Spleen 6×12 (BSD de 2 cláusulas, © Frederic Cambus). `src/font6x12.h` se genera con `tools/gen_font6x12.py` desde `tools/spleen-6x12.bdf`. Licencia en `tools/LICENSE-spleen.txt`; hay que citarla en README y LICENSE.
- Chat de 3 líneas de 24 caracteres. Bandas en el eje de 40 px: 28..39 (arriba), 14..25 (medio) y 0..11 (abajo). El glifo se dibuja con la misma orientación que el texto anterior (fila 0 arriba, columnas a la derecha).
- La pantalla de arranque y del portal sigue con la fuente 8×16 del driver.
- Portal: se muestran los datos guardados (red, contraseña WiFi y clave de Groq), para cambiar solo la red al moverse. Casilla «Mostrar contraseña y clave».

# Asistente de IA para el T-Keyboard

Asistente personal de solo texto para el **T-Keyboard** de LilyGO (ESP32-C3 con pantalla GC9D01 de 0,99", 40×160 px). Escribes con el teclado, el dispositivo envía la pregunta por WiFi a la API de [Groq](https://console.groq.com) y la respuesta aparece en la pantalla pequeña.

English version: [README.md](README.md)

by @mmoroca & Leo.AI 2026 · Licencia: MIT

## Características

- Chat con un modelo de lenguaje (`openai/gpt-oss-120b` en Groq) usando el plan gratuito de Groq.
- Escritura completa con el teclado, con acentos del español y ñ, ¿ y ¡ (ver el mapa de teclas).
- Tres líneas visibles de 24 caracteres, con barra de desplazamiento para leer el historial de la sesión.
- Portal de configuración web: red WiFi (elegida de una lista o escrita a mano), contraseña WiFi y clave de API de Groq.
- La clave de API no va fija en el firmware. Cada persona introduce la suya en el portal.
- Recuperación automática: si se corta la WiFi, el dispositivo muestra las pantallas de reconexión y, cuando vuelve la red, reenvía la pregunta pendiente.
- La pantalla se apaga a los 15 segundos sin pulsar teclas. Cualquier tecla la vuelve a encender.
- La conversación está solo en RAM: se borra al reiniciar el dispositivo.

## Hardware

- LilyGO T-Keyboard (ESP32-C3, WiFi y BLE, pantalla GC9D01 de 0,99" 40×160 px, matriz de teclas).
- Cable USB para alimentar y grabar el firmware.

## Mapa de teclas

| Acción | Teclas |
|---|---|
| Escribir | Teclas tal como están impresas |
| Mayúsculas | Mantén pulsado ⇧ (izquierda o derecha) mientras escribes. No hay bloqueo de mayúsculas. |
| Vocales acentuadas | ALT + a, e, i, o, u → á, é, í, ó, ú |
| Vocales acentuadas en mayúscula | ALT + ⇧ + vocal → Á, É, Í, Ó, Ú |
| ñ / Ñ | ALT + n / ALT + ⇧ + n |
| ¿ / ¡ | ALT + v / ALT + b |
| Símbolos | Pulsa SYM y después la tecla. Solo la siguiente tecla usa el símbolo impreso en ella. |
| Subir en el chat | Toca ⇧ izquierda sin escribir nada |
| Bajar en el chat | Toca ⇧ derecha sin escribir nada |
| Retroiluminación del teclado | ⇧ izquierda + ⇧ derecha a la vez |
| Enviar | ↵ (Enter) |
| Borrar | ⌫ |
| Abrir el portal de configuración | Mantén ALT al encender el dispositivo o al reiniciarlo |

En la pantalla, **↑** (cian) marca los mensajes que enviaste, **↓** (verde) marca las respuestas, y **>** (amarillo) con el cursor `_` marca la línea donde estás escribiendo.

## Puesta en marcha

### 1. Obtén una clave de API de Groq (gratis)

1. Crea una cuenta en [console.groq.com](https://console.groq.com).
2. Abre **API Keys** y crea una clave. Groq solo la muestra una vez: cópiala y guárdala.

Cada persona crea su propia cuenta y su clave. El plan gratuito tiene límites de uso que fija Groq; consulta los valores actuales en la consola. Lee la política de datos de Groq antes de usar el asistente con información sensible.

### 2. Compila y graba el firmware

El proyecto usa PlatformIO. Abre la carpeta en VS Code con la extensión PlatformIO, o usa la línea de comandos:

```bash
pio run -e T-Keyboard-asistente              # compilar
pio run -e T-Keyboard-asistente -t upload    # grabar
pio device monitor -b 115200                 # monitor serie (opcional)
```

Si la grabación no empieza, pon la placa en modo descarga: mantén pulsado **BOOT**, conecta el USB (o pulsa RESET) y suelta BOOT. Consulta la wiki de T-Keyboard para más detalles.

### 3. Configura el dispositivo

1. Mantén pulsado **ALT** al encender (o en el primer arranque, cuando no hay configuración guardada). La pantalla muestra la red `AsistenteIA-XXXX`.
2. Conecta el móvil o el ordenador a esa red.
3. Abre `http://192.168.4.1` en el navegador.
4. Elige tu red WiFi de la lista, o escribe el nombre en el recuadro de debajo si está oculta. Escribe la contraseña WiFi y pega tu clave de API de Groq.
5. Pulsa **Guardar**. El dispositivo prueba la conexión WiFi y la clave. Si las dos son correctas, las guarda y se reinicia. Si no, la pantalla muestra el motivo y puedes corregir los datos y volver a intentarlo.

Los datos guardados se quedan en el dispositivo. Para cambiar solo la red WiFi más adelante (por ejemplo, al salir de casa), vuelve a entrar en el portal: la clave de Groq y el resto de campos ya aparecen escritos.

## Funcionamiento

- Tras conectarse, la pantalla muestra «¿En qué puedo ayudarte?».
- Sin WiFi durante más de 5 segundos, la pantalla alterna «Sin conexión WiFi» con «ALT + RESET» / «para nueva WiFi». Cuando vuelve la red, regresa el chat.
- Si envías una pregunta sin red, se guarda y se envía sola cuando estén listas la conexión y la hora. Mientras hay una pregunta pendiente, el dispositivo no acepta otra.
- El dispositivo obtiene la hora por NTP. TLS necesita la hora correcta, así que la primera respuesta tras el arranque puede tardar unos segundos más.
- Mientras espera la respuesta (normalmente entre 5 y 20 segundos), el teclado no responde.

## Estructura del proyecto

```
src/main.cpp              Firmware: teclado, pantalla, portal, cliente de Groq
src/font6x12.h            Fuente Spleen 6×12 (generada, incluye acentos del español)
src/groq_ca.h             Certificado raíz de api.groq.com (GTS Root R4)
lib/TFT_GC9D01N-Patched/  Driver de pantalla, con acentos y reset por software
tools/gen_font6x12.py     Regenera src/font6x12.h desde el archivo BDF
tools/spleen-6x12.bdf     Fuente original (BSD de 2 cláusulas)
partitions_asistente.csv  Tabla de particiones (app de 3,94 MB, sin OTA)
platformio.ini            Configuración de compilación
DISENO.md                 Notas de diseño
```

## Limitaciones conocidas

- Solo texto: sin entrada ni salida de voz.
- La conversación no se guarda entre reinicios.
- La respuesta se espera en el propio dispositivo: el teclado se bloquea durante la petición.
- La pantalla es pequeña: las respuestas largas se parten en varias líneas y se recorren con ⇧.

## Créditos y licencias

- Código de este proyecto: [MIT](LICENSE), por @mmoroca & Leo.AI.
- Driver de pantalla `lib/TFT_GC9D01N-Patched/`: MIT, Copyright (c) 2022 Xinyuan-LilyGO (modificado).
- Fuente Spleen 6×12 (`tools/spleen-6x12.bdf`, `src/font6x12.h`): BSD de 2 cláusulas, Copyright (c) 2018-2026 Frederic Cambus. Ver [tools/LICENSE-spleen.txt](tools/LICENSE-spleen.txt).
- ArduinoJson (dependencia): MIT, Copyright (c) Benoit Blanchon.
- API de Groq: sujeta a los términos de servicio de Groq.

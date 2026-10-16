# AI Assistant for the T-Keyboard

A text-only personal AI assistant for the LilyGO **T-Keyboard** (ESP32-C3 with a 0.99" GC9D01 display, 40×160 px). You type on the keyboard, the device sends your question over Wi-Fi to the [Groq](https://console.groq.com) API, and the answer appears on the small screen.

Versión en español: [README.es.md](README.es.md)

by @mmoroca & Leo.AI 2026 · License: MIT

## Features

- Chat with a large language model (`openai/gpt-oss-120b` on Groq) through the Groq free plan.
- Full keyboard text entry, with Spanish accents and ñ, ¿ and ¡ (see the key map below).
- Three visible lines of 24 characters, with a scroll bar to read the history of the session.
- Configuration web portal: Wi-Fi network (chosen from a list or typed by hand), Wi-Fi password and Groq API key.
- Your API key is never stored in the firmware. Each user enters their own key in the portal.
- Automatic recovery: if the Wi-Fi drops, the device shows the reconnection screens and resends the pending question once the network is back.
- The screen turns off after 15 seconds without key presses. Any key turns it back on.
- The conversation lives only in RAM: it is erased when the device restarts.

## Hardware

- LilyGO T-Keyboard (ESP32-C3, Wi-Fi and BLE, 0.99" GC9D01 display 40×160 px, keyboard matrix).
- USB cable for power and flashing.

## Key map

| Action | Keys |
|---|---|
| Type | Keys as printed |
| Capital letters | Hold ⇧ (left or right) while typing. There is no caps lock. |
| Accented vowels | ALT + a, e, i, o, u → á, é, í, ó, ú |
| Capital accented vowels | ALT + ⇧ + vowel → Á, É, Í, Ó, Ú |
| ñ / Ñ | ALT + n / ALT + ⇧ + n |
| ¿ / ¡ | ALT + v / ALT + b |
| Symbols | Press SYM, then the key. Only the next key uses the symbol printed on it. |
| Scroll up | Tap ⇧ left without typing anything |
| Scroll down | Tap ⇧ right without typing anything |
| Keyboard backlight | ⇧ left + ⇧ right together |
| Send | ↵ (Enter) |
| Delete | ⌫ |
| Open the configuration portal | Hold ALT while powering on or resetting the device |

The screen shows the state of the conversation: **↑** (cyan) marks the messages you sent, **↓** (green) marks the replies, and **>** (yellow) with a `_` cursor marks the line where you are typing.

## Setup

### 1. Get a Groq API key (free)

1. Create an account at [console.groq.com](https://console.groq.com).
2. Open **API Keys** and create a key. Groq shows it only once: copy it and keep it.

Each person creates their own account and key. The free plan has rate limits set by Groq; check the console for current values. Read Groq's data policy before using the assistant with sensitive information.

### 2. Build and flash the firmware

The project uses PlatformIO. Open the folder in VS Code with the PlatformIO extension, or use the command line:

```bash
pio run -e T-Keyboard-asistente              # compile
pio run -e T-Keyboard-asistente -t upload    # flash
pio device monitor -b 115200                 # serial log (optional)
```

If the upload does not start, put the board in download mode: hold **BOOT**, connect USB (or press RESET), then release BOOT. See the T-Keyboard wiki for details.

### 3. Configure the device

1. Hold **ALT** while powering on (or on the first boot, when there is no saved configuration). The screen shows the network `AsistenteIA-XXXX`.
2. Connect your phone or computer to that network.
3. Open `http://192.168.4.1` in a browser.
4. Choose your Wi-Fi network from the list, or type its name in the box below the list if it is hidden. Type the Wi-Fi password and paste your Groq API key.
5. Press **Guardar** (Save). The device tests the Wi-Fi connection and the key. If both are correct, it saves them and restarts. If not, the screen shows the reason, and you can correct the data and try again.

The saved data stays in the device. To change only the Wi-Fi network later (for example, when you move), enter the portal again: the Groq key and the rest of the fields are already filled in.

## Behaviour

- After connecting, the screen shows «¿En qué puedo ayudarte?».
- Without Wi-Fi for more than 5 seconds, the screen alternates «Sin conexión WiFi» with «ALT + RESET» / «para nueva WiFi». When the network returns, the chat comes back.
- If you send a question without network, it is kept and sent automatically when the connection and the clock are ready. While a question is pending, the device does not accept another one.
- The device gets the time by NTP. TLS needs the correct time, so the first answer after boot can take a few seconds.
- While the device waits for a reply (typically 5 to 20 seconds), the keyboard does not respond.

## Project structure

```
src/main.cpp              Firmware: keyboard, screen, portal, Groq client
src/font6x12.h            Spleen 6×12 font (generated, includes Spanish accents)
src/groq_ca.h             Root certificate for api.groq.com (GTS Root R4)
lib/TFT_GC9D01N-Patched/  Display driver, patched for accents and reset
tools/gen_font6x12.py     Regenerates src/font6x12.h from the BDF file
tools/spleen-6x12.bdf     Font source (BSD 2-Clause)
partitions_asistente.csv  Partition table (3.94 MB app, no OTA)
platformio.ini            Build configuration
DISENO.md                 Design notes (in Spanish)
```

## Known limitations

- Text only: no voice input or output.
- The conversation is not saved between restarts.
- The reply waits on the device: the keyboard is blocked during the request.
- The screen is small: long answers are split across several lines and you scroll with ⇧.

## Credits and licenses

- Code of this project: [MIT](LICENSE), by @mmoroca & Leo.AI.
- Display driver `lib/TFT_GC9D01N-Patched/`: MIT, Copyright (c) 2022 Xinyuan-LilyGO (patched).
- Font Spleen 6×12 (`tools/spleen-6x12.bdf`, `src/font6x12.h`): BSD 2-Clause, Copyright (c) 2018-2026 Frederic Cambus. See [tools/LICENSE-spleen.txt](tools/LICENSE-spleen.txt).
- ArduinoJson (dependency): MIT, Copyright (c) Benoit Blanchon.
- Groq API: subject to Groq's terms of service.

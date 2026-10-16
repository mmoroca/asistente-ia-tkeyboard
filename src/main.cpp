// Asistente de IA (Groq) para el T-Keyboard (ESP32-C3, pantalla GC9D01N 40x160).
//
// - Solo texto: escribes con el teclado y el modelo responde en la pantalla.
// - Configuración por portal: si no hay datos guardados o mantienes ALT al encender, la placa
//   crea la red "AsistenteIA-XXXX" y la página de configuración está en 192.168.4.1.
//   Cada usuario introduce su propia clave de API de Groq (console.groq.com, API Keys).
//   La red y la clave se prueban antes de guardar.
// - Teclas:
//     ALT + vocal = vocal acentuada (ALT+Mayús = mayúscula); ALT+n = ñ; ALT+v = ¿; ALT+b = ¡.
//     SYM + tecla = símbolo impreso en esa tecla (solo la siguiente tecla).
//     ⇧ izquierda sube y ⇧ derecha baja por el chat (tocar y soltar sin escribir nada).
//     ⇧ izquierda + ⇧ derecha = retroiluminación del teclado.
//     ↵ envía. ⌫ borra.
// - La pantalla se apaga a los 15 s sin pulsar ninguna tecla; cualquier tecla la enciende.
// - La sesión está solo en RAM: al reiniciar se borra.

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "groq_ca.h"
#include "font6x12.h"

// Pines del T-Keyboard: los mismos que el reloj y el Scrabble, que ya funcionan en la placa.
#define TFT_BL_PIN 8
#define TFT_DC 2
#define TFT_MOSI 10
#define TFT_SCLK 20
#define TFT_CS -1
#define TFT_MISO 2
#define TFT_RST -1
#define KB_BACKLIGHT_PIN 9

#include <TFT_GC9D01N.h>
static TFT_GC9D01N_Class TFT_099;

#define SCREEN_BACKLIGHT 50
#define SCREEN_OFF_MS 15000UL
#define WIFI_RETRY_MS 30000UL
#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "time.google.com"

#define GROQ_BASE "https://api.groq.com/openai/v1"
#define GROQ_MODEL "openai/gpt-oss-120b"
#define GROQ_MAX_TOKENS 300

static const char SYSTEM_PROMPT[] =
    "Eres un asistente breve. Tus respuestas se leen en una pantalla de 2 lineas de 18 caracteres. "
    "Responde en el mismo idioma que el usuario. Usa frases cortas, como maximo 3. "
    "No uses listas, markdown, asteriscos ni emojis. Si piden algo largo, da solo lo esencial.";

// Filas de texto (eje de 40 px, como en el reloj). Cada línea ocupa 16 px.
// ROW_TOP es la línea de arriba (hardware 16..31) y ROW_BOTTOM la de abajo (hardware 0..15).
#define ROW_TOP 7
#define ROW_BOTTOM 23
#define AXIS_A 40
#define AXIS_B 160
#define TEXT_X 8          // el texto empieza después del icono (columnas 0..7)
#define LINE_CHARS 24     // 24 caracteres de 6 px: columnas 8..151
#define VIS_LINES 3       // 3 líneas de 12 px en el eje de 40 px
#define SB_X 156          // barra de desplazamiento: columnas 156..159
// Base (eje de 40 px, hardware) de cada línea de arriba abajo: 28..39, 14..25 y 0..11.
static const unsigned BAND_A[VIS_LINES] = {28, 14, 0};

#define REG_MAX 120       // líneas guardadas del chat
#define INPUT_MAX 150     // bytes máximos de lo que se escribe
#define HIST_MAX 8        // mensajes enviados al modelo como contexto

#define C_CYAN 0x07FF
#define C_GREEN 0x07E0
#define C_TRACK 0x2104
#define C_YELLOW 0xFFE0

// ------------------------------------------------------------------ teclado

static const uint8_t rows[] = {0, 3, 18, 12, 11, 6, 7};
static const uint8_t cols[] = {1, 4, 5, 19, 13};
static const int rowCount = sizeof(rows) / sizeof(rows[0]);
static const int colCount = sizeof(cols) / sizeof(cols[0]);

// Posiciones [columna][fila], igual que keyboard[][] en BLE_keybord.ino del T-Keyboard.
#define K_ALT_C 0
#define K_ALT_R 4
#define K_SYM_C 0
#define K_SYM_R 2
#define K_SHL_C 1
#define K_SHL_R 6
#define K_SHR_C 2
#define K_SHR_R 3
#define K_ENTER_C 3
#define K_ENTER_R 3
#define K_BACK_C 4
#define K_BACK_R 3

// Carácter normal de cada tecla (0 = sin carácter).
static const char KEY_NORMAL[5][7] = {
    {'q', 'w', 0, 'a', 0, ' ', 0},
    {'e', 's', 'd', 'p', 'x', 'z', 0},
    {'r', 'g', 't', 0, 'v', 'c', 'f'},
    {'u', 'h', 'y', 0, 'b', 'n', 'j'},
    {'o', 'l', 'i', 0, '$', 'm', 'k'},
};

// Símbolo impreso en cada tecla (se usa con SYM).
static const char KEY_SYM[5][7] = {
    {'#', '1', 0, '*', 0, 0, '0'},
    {'2', '4', '5', '@', '8', '7', 0},
    {'3', '/', '(', 0, '?', '9', '6'},
    {'_', ':', ')', 0, '!', ',', ';'},
    {'+', '"', '-', 0, 0, '.', '\''},
};

struct Accent {
    uint8_t c, r;
    const char *lower;
    const char *upper;
};
// Acentos con ALT. ¿ y ¡ van en las teclas de «?» (v) y «!» (b), que ya están impresas así.
static const Accent ACCENTS[] = {
    {0, 3, "á", "Á"}, {1, 0, "é", "É"}, {4, 2, "í", "Í"}, {4, 0, "ó", "Ó"},
    {3, 0, "ú", "Ú"}, {3, 5, "ñ", "Ñ"}, {2, 4, "¿", "¿"}, {3, 4, "¡", "¡"},
};

static bool key_held[5][7];
static bool key_prev[5][7];
static bool key_edge[5][7];

static void scan_matrix() {
    for (int c = 0; c < colCount; c++) {
        pinMode(cols[c], OUTPUT);
        digitalWrite(cols[c], LOW);
        for (int r = 0; r < rowCount; r++) {
            pinMode(rows[r], INPUT_PULLUP);
            delay(1);
            bool now = (digitalRead(rows[r]) == LOW);
            key_edge[c][r] = now && !key_prev[c][r];
            key_prev[c][r] = now;
            key_held[c][r] = now;
            pinMode(rows[r], INPUT);
        }
        pinMode(cols[c], INPUT);
    }
}

static bool is_shift(int c, int r) {
    return (c == K_SHL_C && r == K_SHL_R) || (c == K_SHR_C && r == K_SHR_R);
}

static const char *accent_for(int c, int r, bool upper) {
    for (size_t i = 0; i < sizeof(ACCENTS) / sizeof(ACCENTS[0]); i++) {
        if (ACCENTS[i].c == c && ACCENTS[i].r == r) return upper ? ACCENTS[i].upper : ACCENTS[i].lower;
    }
    return nullptr;
}

// ------------------------------------------------------------------ dibujo

static void fill_rect(unsigned a0, unsigned a1, unsigned b0, unsigned b1, uint16_t color) {
    TFT_099.BlockWrite(a0, a1, b0, b1);
    unsigned long n = (unsigned long) (a1 - a0 + 1) * (b1 - b0 + 1);
    while (n--) TFT_099.WriteOneDot(color);
}

// Pantalla completa con dos líneas (como show_lines del reloj).
// Borrado completo de la pantalla: el arranque usa otra fuente y sus píxeles no deben quedar debajo.
static bool chat_clear_pending = true;   // al volver al chat, la siguiente vez que se dibuje se borra todo
static void clear_screen() {
    TFT_099.DispColor(0, 0, TFT_WIDTH, TFT_HEIGHT, BLACK);
}

static void show_lines(const char *top, const char *bottom) {
    clear_screen();
    chat_clear_pending = true;
    TFT_099.DispStr((char *) top, 0, ROW_TOP, WHITE, BLACK);
    TFT_099.DispStr((char *) bottom, 0, ROW_BOTTOM, WHITE, BLACK);
}

// Iconos de 8x8: ↑ enviado (cian) y ↓ recibido (verde). Se dibujan como el icono WiFi del reloj.
static const char ART_UP[8][9] = {
    "...##...", "..####..", ".######.", "##.##.##", "...##...", "...##...", "...##...", "...##...",
};
static const char ART_DOWN[8][9] = {
    "...##...", "...##...", "...##...", "...##...", "##.##.##", ".######.", "..####..", "...##...",
};

static void draw_icon8(uint8_t kind, unsigned a0, unsigned b0) {
    const char (*art)[9] = (kind == 1) ? ART_UP : ART_DOWN;
    uint16_t ink = (kind == 1) ? C_CYAN : C_GREEN;
    TFT_099.BlockWrite(a0, a0 + 7, b0, b0 + 7);
    for (int bb = 0; bb < 8; bb++) {
        for (int aa = 0; aa < 8; aa++) {
            int y = 7 - aa;
            TFT_099.WriteOneDot(art[y][bb] == '#' ? ink : BLACK);
        }
    }
}

// Barra de desplazamiento: columnas 156..159, toda la altura. Arriba = lo más antiguo.
// Se escribe con una sola ventana y el color de cada píxel calculado antes (sin dos ventanas solapadas).
static void draw_scrollbar(int total, int top) {
    uint16_t track = total > VIS_LINES ? C_TRACK : BLACK;
    int lo = -1, hi = -1;
    if (total > VIS_LINES) {
        int maxTop = total - VIS_LINES;
        int thumb = 40 * VIS_LINES / total;
        if (thumb < 6) thumb = 6;
        int travel = 40 - thumb;
        int pos = (int) ((long) top * travel / maxTop);
        hi = 39 - pos;          // arriba físicamente = hardware 39
        lo = hi - thumb + 1;
    }
    TFT_099.BlockWrite(0, AXIS_A - 1, SB_X, AXIS_B - 1);
    for (unsigned col = SB_X; col < AXIS_B; col++) {        // 4 columnas
        for (int a = 0; a < AXIS_A; a++) {                   // 40 filas, la de X avanza primero
            bool on = (lo >= 0 && a >= lo && a <= hi);
            TFT_099.WriteOneDot(on ? WHITE : track);
        }
    }
}

// ------------------------------------------------------------------ texto

static bool is_supported_2byte(unsigned char lead, unsigned char cont) {
    if (lead == 0xC3) {
        switch (cont) {
            case 0xA1: case 0xA9: case 0xAD: case 0xB3: case 0xBA: case 0xBC: case 0xB1:
            case 0x81: case 0x89: case 0x8D: case 0x93: case 0x9A: case 0x9C: case 0x91:
                return true;
        }
    } else if (lead == 0xC2) {
        return cont == 0xBF || cont == 0xA1  // ¿ ¡
               || cont == 0xB0 || cont == 0xBA || cont == 0xAA;  // ° º ª
    }
    return false;
}

// Deja solo caracteres que la fuente tiene. Lo demás se cambia o se quita.
static String sanitize(const char *s) {
    String out;
    const unsigned char *p = (const unsigned char *) s;
    while (*p) {
        unsigned char c = *p;
        if (c < 0x80) {
            if (c == '\n' || c == '\r' || c == '\t') out += ' ';
            else if (c == '*' || c == '`') { /* formato markdown: se quita */ }
            else if (c >= 0x20 && c < 0x7F) out += (char) c;
            p++;
            continue;
        }
        if ((c == 0xC2 || c == 0xC3) && p[1] && is_supported_2byte(c, p[1])) {
            out += (char) c;
            out += (char) p[1];
            p += 2;
            continue;
        }
        // Formas habituales de escribir grados, signos y espacios que el modelo puede devolver.
        if (c == 0xC2 && p[1] == 0xA0) { out += ' '; p += 2; continue; }             // espacio sin ruptura
        if (c == 0xCB && p[1] == 0x9A) { out += "\xc2\xba"; p += 2; continue; }     // ˚ como º
        if (c == 0xE1 && p[1] == 0xB5 && p[2] == 0x92) { out += "\xc2\xba"; p += 3; continue; }  // ᵒ como º
        if (c == 0xE2 && p[1] == 0x84 && p[2] == 0x83) { out += "\xc2\xba" "C"; p += 3; continue; }  // ℃
        if (c == 0xE2 && p[1] == 0x88 && p[2] == 0x92) { out += '-'; p += 3; continue; }  // signo menos −
        if (c == 0xE2 && p[1] == 0x80 && p[2]) {  // puntuación general de tres bytes
            unsigned char e = p[2];
            switch (e) {
                case 0x98: case 0x99: out += '\''; break;  // comillas simples curvas
                case 0x9C: case 0x9D: out += '"'; break;   // comillas dobles curvas
                case 0x93: case 0x94: out += '-'; break;   // guiones largos
                case 0xA6: out += "..."; break;            // puntos suspensivos
                case 0xA2: out += '-'; break;              // viñeta •
                case 0x89: case 0xAF: out += ' '; break;   // espacios finos
                default: out += '?'; break;
            }
            p += 3;
            continue;
        }
        // Resto de caracteres UTF-8: se cambian por «?» y se saltan sus bytes.
        int len = (c >= 0xF0) ? 4 : (c >= 0xE0) ? 3 : (c >= 0xC0) ? 2 : 1;
        p++;
        for (int i = 1; i < len && (*p & 0xC0) == 0x80; i++) p++;
        out += '?';
    }
    return out;
}

static int cp_len(unsigned char c) { return (c == 0xC2 || c == 0xC3) ? 2 : 1; }

// Una línea de la pantalla. icon: 0 ninguno, 1 enviado, 2 recibido.
struct DLine {
    char t[56];
    uint8_t icon;
};

struct Wrap {
    DLine *out;
    int cap;
    int n;
    uint8_t icon;
    bool first;
    char cur[56];
    int len;
    int chars;
};

static void wr_flush(Wrap &w) {
    if (w.n < w.cap) {
        DLine &d = w.out[w.n];
        memcpy(d.t, w.cur, w.len);
        d.t[w.len] = 0;
        d.icon = w.first ? w.icon : 0;
        w.first = false;
        w.n++;
    }
    w.len = 0;
    w.chars = 0;
    w.cur[0] = 0;
}

static void wr_cp(Wrap &w, const char *s, int l) {
    if (w.chars == LINE_CHARS) wr_flush(w);
    memcpy(w.cur + w.len, s, l);
    w.len += l;
    w.cur[w.len] = 0;
    w.chars++;
}

static void wr_word(Wrap &w, const char *s, int bytes) {
    int cps = 0;
    for (int i = 0; i < bytes; i++) {
        if (((unsigned char) s[i] & 0xC0) != 0x80) cps++;
    }
    if (w.chars > 0) {
        if (w.chars + 1 + cps > LINE_CHARS) wr_flush(w);
        else wr_cp(w, " ", 1);
    }
    int i = 0;
    while (i < bytes) {
        int l = cp_len((unsigned char) s[i]);
        if (i + l > bytes) l = bytes - i;
        wr_cp(w, s + i, l);
        i += l;
    }
}

// Parte el texto en líneas de 18 caracteres, cortando por palabras. Devuelve el número de líneas.
static int wrap_text(const char *src, uint8_t icon, DLine *out, int cap) {
    Wrap w;
    w.out = out;
    w.cap = cap;
    w.n = 0;
    w.icon = icon;
    w.first = true;
    w.len = 0;
    w.chars = 0;
    w.cur[0] = 0;
    const char *p = src;
    while (*p) {
        if (*p == ' ' || *p == '\n') {
            if (*p == '\n' && w.chars > 0) wr_flush(w);
            p++;
            continue;
        }
        const char *st = p;
        while (*p && *p != ' ' && *p != '\n') p++;
        wr_word(w, st, (int) (p - st));
    }
    if (w.chars > 0 || w.n == 0) wr_flush(w);
    return w.n;
}

// ------------------------------------------------------------------ chat (registro)

static DLine reg[REG_MAX];
static int reg_n = 0;
static bool placeholder_open = false;  // la última línea es el «...» de espera
static DLine disp[REG_MAX + 40];
static DLine inp_lines[32];
static String input;
static bool follow = true;   // mostrar siempre lo último
static int top_line = 0;     // primera línea visible si no se sigue
static bool busy = false;
static bool dirty = true;

static String hist_role[HIST_MAX];
static String hist_content[HIST_MAX];
static int hist_n = 0;

static void reg_push(const DLine &l) {
    if (reg_n == REG_MAX) {
        memmove(reg, reg + 1, sizeof(DLine) * (REG_MAX - 1));
        reg_n--;
    }
    reg[reg_n++] = l;
}

static void reg_add(uint8_t icon, const char *text) {
    static DLine tmp[80];
    int n = wrap_text(text, icon, tmp, 80);
    for (int i = 0; i < n; i++) reg_push(tmp[i]);
}

static void hist_add(const String &user, const String &reply) {
    for (int k = 0; k < 2; k++) {
        if (hist_n == HIST_MAX) {
            for (int i = 1; i < HIST_MAX; i++) {
                hist_role[i - 1] = hist_role[i];
                hist_content[i - 1] = hist_content[i];
            }
            hist_n--;
        }
        hist_role[hist_n] = (k == 0) ? "user" : "assistant";
        hist_content[hist_n] = (k == 0) ? user : reply;
        hist_n++;
    }
}

// Registro + línea de escritura al final. Devuelve el número total de líneas.
static int build_display() {
    memcpy(disp, reg, sizeof(DLine) * reg_n);
    int n = reg_n;
    // Cursor «_» al final de lo que se escribe; desaparece al enviar con ENTER.
    String shown = input;
    if (!busy) shown += '_';
    int k = wrap_text(shown.c_str(), 0, inp_lines, 32);
    inp_lines[0].icon = 3;  // primera línea de escritura: «>»
    for (int i = 0; i < k && n < REG_MAX + 40; i++) disp[n++] = inp_lines[i];
    return n;
}

static void input_append(const char *s) {
    if (input.length() + strlen(s) > INPUT_MAX) return;
    input += s;
    follow = true;
    dirty = true;
}

static void input_backspace() {
    if (input.length() == 0) return;
    int i = input.length() - 1;
    while (i > 0 && (((unsigned char) input[i]) & 0xC0) == 0x80) i--;
    input.remove(i);
    follow = true;
    dirty = true;
}

// Dibuja un glifo 6x12 en la banda a0..a0+11 y la columna b0..b0+5, con la misma orientación
// que el texto del driver: fila 0 del glifo en la parte alta (a mayor), columnas hacia la derecha.
static void draw_glyph6(int idx, unsigned a0, unsigned b0, uint16_t fg) {
    TFT_099.BlockWrite(a0, a0 + FONT6_H - 1, b0, b0 + FONT6_W - 1);
    for (int bo = 0; bo < FONT6_W; bo++) {
        for (int ao = 0; ao < FONT6_H; ao++) {
            int row = FONT6_H - 1 - ao;
            bool on = (FONT6_ROWS[idx][row] >> (7 - bo)) & 1;
            TFT_099.WriteOneDot(on ? fg : BLACK);
        }
    }
}

static int font6_index(uint32_t cp) {
    for (int i = 0; i < FONT6_COUNT; i++) {
        if (FONT6_CP[i] == cp) return i;
    }
    return 0;  // espacio, por si acaso
}

// Recorre un texto UTF-8 (solo caracteres de 1 o 2 bytes, como deja sanitize()).
static uint32_t next_cp(const char *&p) {
    unsigned char c = (unsigned char) *p++;
    if ((c == 0xC2 || c == 0xC3) && *p) {
        unsigned char d = (unsigned char) *p++;
        return ((uint32_t) (c & 0x1F) << 6) | (d & 0x3F);
    }
    return c;
}

// Una línea completa: icono a la izquierda y texto a partir de la columna 8.
static void draw_line6(const DLine *l, unsigned a0) {
    fill_rect(a0, a0 + FONT6_H - 1, 0, TEXT_X - 1, BLACK);
    unsigned b = TEXT_X;
    if (l) {
        if (l->icon == 3) draw_glyph6(font6_index('>'), a0, 1, C_YELLOW);  // «>» amarillo: línea de escritura
        else if (l->icon) draw_icon8(l->icon, a0 + 2, 0);  // icono de 8 px centrado en 12 px
        const char *p = l->t;
        while (*p && b + FONT6_W <= SB_X) {
            uint32_t cp = next_cp(p);
            draw_glyph6(font6_index(cp), a0, b, WHITE);
            b += FONT6_W;
        }
    }
    if (b < SB_X) fill_rect(a0, a0 + FONT6_H - 1, b, SB_X - 1, BLACK);
}

// Línea centrada en la franja de texto (6x12). a0 = base de la franja.
static void draw_centered6(const char *t, unsigned a0) {
    int n = 0;
    const char *p = t;
    while (*p) { next_cp(p); n++; }
    // Columna de inicio siempre par: con columna impar la placa dibuja mal las letras
    // (con n impar, (156 - 6n) / 2 sale impar). Así funciona con cualquier longitud.
    unsigned b = ((SB_X - n * FONT6_W) / 2) & ~1u;
    p = t;
    while (*p) {
        uint32_t cp = next_cp(p);
        draw_glyph6(font6_index(cp), a0, b, WHITE);
        b += FONT6_W;
    }
}

// Presentación de 3 s: nombre y autoría, centrados.
static void show_splash() {
    clear_screen();
    draw_centered6("AsistenteIA", 22);
    draw_centered6("by @mmoroca & Leo.AI '26", 6);
    delay(3000);
    chat_clear_pending = true;
}

static bool time_ready() { return time(nullptr) > 1700000000; }

static bool ntp_started = false;
static void start_ntp() {
    if (!ntp_started) {
        configTime(0, 0, NTP_SERVER_1, NTP_SERVER_2);  // TLS necesita la hora correcta
        ntp_started = true;
    }
}

static bool wait_time(unsigned long ms) {
    unsigned long t0 = millis();
    start_ntp();
    while (!time_ready() && millis() - t0 < ms) delay(100);
    return time_ready();
}

static void draw_screen() {
    int total = build_display();
    if (chat_clear_pending) {
        clear_screen();
        chat_clear_pending = false;
    }
    if (reg_n == 0 && input.length() == 0) {
        DLine st;
        strncpy(st.t, WiFi.status() == WL_CONNECTED ? "Conectado" : "Conectando WiFi...", sizeof st.t - 1);
        st.t[sizeof st.t - 1] = 0;
        st.icon = 0;
        draw_line6(&st, BAND_A[0]);
        for (int k = 1; k < VIS_LINES; k++) draw_line6(nullptr, BAND_A[k]);
        draw_scrollbar(0, 0);
        return;
    }
    int maxTop = total > VIS_LINES ? total - VIS_LINES : 0;
    if (follow) top_line = maxTop;
    if (top_line > maxTop) top_line = maxTop;
    for (int k = 0; k < VIS_LINES; k++) {
        int idx = top_line + k;
        draw_line6(idx < total ? &disp[idx] : nullptr, BAND_A[k]);
    }
    // Franjas entre líneas (hardware 12..13 y 26..27): ninguna línea las pinta, así que se limpian
    // aquí para que no queden restos de la pantalla de arranque (fuente 8x16).
    fill_rect(12, 13, 0, SB_X - 1, BLACK);
    fill_rect(26, 27, 0, SB_X - 1, BLACK);
    draw_scrollbar(total, top_line);
}

static void scroll_chat(int dir) {
    int total = build_display();
    int maxTop = total > VIS_LINES ? total - VIS_LINES : 0;
    int cur = follow ? maxTop : top_line;
    int nt = cur + dir;
    if (nt < 0) nt = 0;
    if (nt > maxTop) nt = maxTop;
    top_line = nt;
    follow = (nt == maxTop);
    dirty = true;
}

// ------------------------------------------------------------------ Groq

struct Config {
    String ssid;
    String pass;
    String apikey;
    bool valid;
};
static Config cfg;


static const char *GROQ_CHAT_URL = GROQ_BASE "/chat/completions";
static const char *GROQ_MODELS_URL = GROQ_BASE "/models";

// Comprueba la clave con GET /models (no gasta tokens). Devuelve el código HTTP o -1.
static int groq_check_key(const String &key) {
    WiFiClientSecure client;
    client.setCACert(GROQ_ROOT_CA);
    HTTPClient http;
    http.setTimeout(15000);
    if (!http.begin(client, GROQ_MODELS_URL)) return -1;
    http.addHeader("Authorization", "Bearer " + key);
    int code = http.GET();
    http.end();
    return code;
}

static bool groq_chat(const String &user, String &reply, String &err) {
    JsonDocument doc;
    doc["model"] = GROQ_MODEL;
    doc["max_tokens"] = GROQ_MAX_TOKENS;
    doc["reasoning_effort"] = "low";
    JsonArray msgs = doc["messages"].to<JsonArray>();
    JsonObject sys = msgs.add<JsonObject>();
    sys["role"] = "system";
    sys["content"] = SYSTEM_PROMPT;
    for (int i = 0; i < hist_n; i++) {
        JsonObject m = msgs.add<JsonObject>();
        m["role"] = hist_role[i];
        m["content"] = hist_content[i];
    }
    JsonObject u = msgs.add<JsonObject>();
    u["role"] = "user";
    u["content"] = user;
    String body;
    serializeJson(doc, body);

    WiFiClientSecure client;
    client.setCACert(GROQ_ROOT_CA);
    HTTPClient http;
    http.setTimeout(30000);
    if (!http.begin(client, GROQ_CHAT_URL)) {
        err = "No se pudo abrir la conexión";
        return false;
    }
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + cfg.apikey);
    int code = http.POST(body);
    String resp = (code > 0) ? http.getString() : String();
    http.end();

    if (code < 0) {
        err = "Sin conexión con Groq";
        return false;
    }
    if (code == 401 || code == 403) {
        err = "Clave no válida (" + String(code) + ")";
        return false;
    }
    if (code == 429) {
        err = "Límite de Groq alcanzado. Espera un poco";
        return false;
    }
    if (code != 200) {
        err = "Error " + String(code) + " de Groq";
        return false;
    }
    JsonDocument rdoc;
    if (deserializeJson(rdoc, resp)) {
        err = "Respuesta no válida";
        return false;
    }
    const char *content = rdoc["choices"][0]["message"]["content"].as<const char *>();
    if (!content || !*content) {
        err = "Sin respuesta. Prueba otra vez";
        return false;
    }
    reply = sanitize(content);
    reply.trim();
    if (reply.length() == 0) {
        err = "Sin respuesta. Prueba otra vez";
        return false;
    }
    return true;
}

// ------------------------------------------------------------------ configuración

static Config load_config() {
    Preferences p;
    p.begin("asistente", true);
    Config c;
    c.ssid = p.getString("ssid", "");
    c.pass = p.getString("pass", "");
    c.apikey = p.getString("apikey", "");
    p.end();
    c.valid = c.ssid.length() > 0 && c.apikey.length() > 0;
    return c;
}

static WebServer web(80);
static DNSServer dns;
enum PortalTest { PT_IDLE, PT_WIFI, PT_OK };
static PortalTest portal_test = PT_IDLE;

static String html_escape(const String &s) {
    String out;
    for (size_t i = 0; i < s.length(); i++) {
        char c = s[i];
        if (c == '<') out += "&lt;";
        else if (c == '>') out += "&gt;";
        else if (c == '&') out += "&amp;";
        else if (c == '"') out += "&quot;";
        else out += c;
    }
    return out;
}

// Página de configuración. Lista de redes (escaneo en cada carga) y campo de texto para escribir el nombre.
// Los datos guardados aparecen ya escritos. La clave de API se conserva aunque cambie la red.
static void portal_page() {
    String names[30];
    int rssi[30];
    int m = 0;
    if (portal_test != PT_WIFI) {   // escanear mientras se prueba una red interrumpiría la conexión
        int n = WiFi.scanNetworks();
        for (int i = 0; i < n && m < 30; i++) {
            String s = WiFi.SSID(i);
            if (s.length() == 0) continue;
            int r = WiFi.RSSI(i);
            int j = 0;
            while (j < m && names[j] != s) j++;
            if (j < m) {
                if (r > rssi[j]) rssi[j] = r;
                continue;
            }
            names[m] = s;
            rssi[m] = r;
            m++;
        }
        WiFi.scanDelete();
    }
    // Más señal primero.
    for (int i = 0; i < m; i++) {
        for (int j = i + 1; j < m; j++) {
            if (rssi[j] > rssi[i]) {
                String t = names[i]; names[i] = names[j]; names[j] = t;
                int u = rssi[i]; rssi[i] = rssi[j]; rssi[j] = u;
            }
        }
    }
    bool in_list = false;
    for (int j = 0; j < m; j++) {
        if (names[j] == cfg.ssid) in_list = true;
    }

    String sel;
    sel.reserve(1024);
    sel += "<select id='redes' onchange=\"var v=this.value;if(v)document.getElementById('ssid').value=v\" "
           "style='width:100%;font-size:18px'>";
    sel += "<option value=''>-- Elige una red de la lista --</option>";
    if (m == 0) sel += "<option value=''>-- No se ven redes: escribe el nombre abajo --</option>";
    if (cfg.ssid.length() && !in_list) {
        sel += "<option value='" + html_escape(cfg.ssid) + "' selected>" + html_escape(cfg.ssid) + " (guardada)</option>";
    }
    for (int j = 0; j < m; j++) {
        String e = html_escape(names[j]);
        sel += "<option value='" + e + "'" + (names[j] == cfg.ssid ? " selected" : "") + ">"
               + e + " (" + String(rssi[j]) + " dBm)</option>";
    }
    sel += "</select>";

    String h;
    h.reserve(4096);
    h += "<!doctype html><html><head><meta charset='utf-8'>"
         "<meta name='viewport' content='width=device-width,initial-scale=1'>"
         "<title>Asistente IA</title></head>"
         "<body style='font-family:sans-serif;max-width:420px;margin:20px auto'>"
         "<h2>Configurar asistente</h2><form method='POST' action='/save'>"
         "<label>Red WiFi (de la lista)<br>";
    h += sel;
    h += "</label>"
         "<p style='font-size:14px'><a href='/'>Buscar redes otra vez</a></p>"
         "<label>Escribir el nombre a mano (si no aparece en la lista o está oculta)<br><input id='ssid' name='ssid' required value='"
         + html_escape(cfg.ssid) + "' style='width:100%;font-size:18px'></label><br><br>"
         "<label>Contraseña WiFi<br><input id='pass' name='pass' type='password' value='"
         + html_escape(cfg.pass) + "' style='width:100%;font-size:18px'></label><br><br>"
         "<label>Clave de API de Groq<br><input id='apikey' name='apikey' type='password' required value='"
         + html_escape(cfg.apikey) + "' style='width:100%;font-size:18px'></label>"
         "<p style='font-size:14px'><label><input type='checkbox' onclick=\""
         "['pass','apikey'].forEach(function(i){document.getElementById(i).type=this.checked?'text':'password'},this)\"> "
         "Mostrar contraseña y clave</label></p>"
         "<p style='font-size:14px'>Cada persona usa su propia clave. Créala gratis en "
         "console.groq.com, en API Keys. Los datos guardados aparecen ya escritos; cambia solo lo necesario.</p>"
         "<button style='font-size:18px;padding:10px 20px'>Guardar</button>"
         "</form></body></html>";
    web.send(200, "text/html; charset=utf-8", h);
}

static String pend_ssid, pend_pass, pend_key;
static unsigned long portal_test_start = 0;
static unsigned long portal_restart_at = 0;

static void portal_save() {
    String ssid = web.arg("ssid");
    String key = web.arg("apikey");
    key.trim();
    if (ssid.length() == 0 || key.length() == 0) {
        web.send(400, "text/plain; charset=utf-8", "Faltan datos: red y clave de API");
        return;
    }
    if (portal_test == PT_WIFI) {
        web.send(409, "text/plain; charset=utf-8", "Ya hay una prueba en curso");
        return;
    }
    // No se guarda nada todavía: primero se comprueba la red y después la clave.
    pend_ssid = ssid;
    pend_pass = web.arg("pass");
    pend_key = key;
    portal_test = PT_WIFI;
    portal_test_start = millis();
    WiFi.begin(pend_ssid.c_str(), pend_pass.c_str());
    char shown[24];
    strncpy(shown, pend_ssid.c_str(), sizeof shown - 1);
    shown[sizeof shown - 1] = '\0';
    show_lines("Probando red...", shown);
    Serial.printf("Portal: probando la red %s\n", pend_ssid.c_str());

    web.send(200, "text/html; charset=utf-8",
             "<html><head><meta charset='utf-8'></head><body style='font-family:sans-serif'>"
             "<h2>Probando la red y la clave</h2><p>Mira la pantalla del asistente. Si todo es "
             "correcto, se guarda y se reinicia solo. Si no, vuelve a esta página y repite los "
             "datos.</p></body></html>");
}

static void portal_commit() {
    Preferences p;
    p.begin("asistente", false);
    p.putString("ssid", pend_ssid);
    p.putString("pass", pend_pass);
    p.putString("apikey", pend_key);
    p.end();
}

static void run_portal() {
    Serial.println("Modo configuración: red AsistenteIA");
    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect();

    char ap[24];
    uint64_t mac = ESP.getEfuseMac();
    snprintf(ap, sizeof ap, "AsistenteIA-%04X", (unsigned) (mac & 0xFFFF));
    WiFi.softAP(ap);
    IPAddress ip = WiFi.softAPIP();
    show_lines(ap, ip.toString().c_str());
    Serial.printf("Red %s, abre http://%s\n", ap, ip.toString().c_str());

    dns.start(53, "*", ip);
    web.on("/", HTTP_GET, portal_page);
    web.on("/save", HTTP_POST, portal_save);
    web.onNotFound([]() {
        web.sendHeader("Location", "http://192.168.4.1/", true);
        web.send(302, "text/plain", "");
    });
    web.begin();

    while (true) {
        dns.processNextRequest();
        web.handleClient();

        if (portal_test == PT_WIFI) {
            if (WiFi.status() == WL_CONNECTED) {
                show_lines("Probando la clave", "de Groq...");
                bool time_ok = wait_time(15000);
                int code = time_ok ? groq_check_key(pend_key) : -1;
                if (code == 200) {
                    portal_commit();
                    portal_test = PT_OK;
                    portal_restart_at = millis() + 2000;
                    show_lines("Clave correcta", "Reiniciando...");
                    Serial.println("Portal: red y clave correctas, guardadas; reiniciando");
                } else {
                    WiFi.disconnect();
                    portal_test = PT_IDLE;
                    if (!time_ok) show_lines("Sin hora de red", "Repite en la web");
                    else if (code == 401 || code == 403) show_lines("Clave no válida", "Repite en la web");
                    else show_lines("Sin acceso a Groq", "Repite en la web");
                    Serial.printf("Portal: clave no comprobada (codigo %d)\n", code);
                }
            } else if (millis() - portal_test_start > 25000) {
                WiFi.disconnect();
                portal_test = PT_IDLE;
                show_lines("No conecta la red", "Repite en la web");
                Serial.println("Portal: la red de prueba no conectó");
            }
        }
        if (portal_test == PT_OK && millis() > portal_restart_at) ESP.restart();
        delay(5);
    }
}

// ------------------------------------------------------------------ teclado: lógica

static bool screen_on = true;
static unsigned long last_key_ms = 0;
static bool kb_backlight_on = true;
static bool sl_clean = false;   // ⇧ izquierda pulsada sin escribir nada con ella
static bool sr_clean = false;   // ⇧ derecha pulsada sin escribir nada con ella
static bool prev_sl = false;
static bool prev_sr = false;
static bool sym_pending = false;

static void toggle_kb_backlight() {
    kb_backlight_on = !kb_backlight_on;
    digitalWrite(KB_BACKLIGHT_PIN, kb_backlight_on ? HIGH : LOW);
}

// Una consulta a Groq: muestra «...» mientras espera y luego la respuesta o el error.
static bool pending_on = false;   // hay una pregunta esperando a que vuelva la red
static String pending_q;

static void run_query(const String &text) {
    reg_add(2, "...");
    placeholder_open = true;
    busy = true;
    follow = true;
    dirty = true;
    draw_screen();

    String reply, err;
    bool ok = groq_chat(text, reply, err);
    busy = false;

    if (placeholder_open && reg_n > 0) {
        reg_n--;
        placeholder_open = false;
    }
    if (ok) {
        reg_add(2, reply.c_str());
        hist_add(text, reply);
    } else {
        reg_add(0, err.c_str());
        if (WiFi.status() != WL_CONNECTED) {
            // Se corta la red a mitad: la pregunta se reenvía sola al volver.
            pending_q = text;
            pending_on = true;
            reg_add(0, "Se reenviará al volver la WiFi.");
        }
    }
    follow = true;
    dirty = true;
}

static void send_message() {
    String text = input;
    text.trim();
    if (text.length() == 0 || busy) return;
    if (pending_on) {
        reg_add(0, "Espera: la pregunta anterior aún no se ha enviado.");
        follow = true;
        dirty = true;
        return;
    }
    reg_add(1, text.c_str());
    input = "";
    if (WiFi.status() != WL_CONNECTED || !time_ready()) {
        // Sin red u hora: se guarda y se envía sola en cuanto esté lista.
        pending_q = text;
        pending_on = true;
        reg_add(0, WiFi.status() != WL_CONNECTED ? "Sin WiFi. Se enviará al volver la red."
                                                : "Sincronizando la hora. Se enviará enseguida.");
        follow = true;
        dirty = true;
        return;
    }
    run_query(text);
}

static void handle_keys() {
    bool any_edge = false;
    bool other_edge = false;
    for (int c = 0; c < colCount; c++) {
        for (int r = 0; r < rowCount; r++) {
            if (!key_edge[c][r]) continue;
            any_edge = true;
            if (!is_shift(c, r)) other_edge = true;
        }
    }
    if (any_edge) {
        last_key_ms = millis();
        if (!screen_on) {
            TFT_099.backlight(SCREEN_BACKLIGHT);
            screen_on = true;
        }
        dirty = true;
    }

    bool sl = key_held[K_SHL_C][K_SHL_R];
    bool sr = key_held[K_SHR_C][K_SHR_R];
    bool sl_edge = key_edge[K_SHL_C][K_SHL_R];
    bool sr_edge = key_edge[K_SHR_C][K_SHR_R];

    // ⇧ izquierda + ⇧ derecha: retroiluminación (una sola vez aunque entren en la misma lectura).
    if ((sl_edge || sr_edge) && sl && sr) {
        toggle_kb_backlight();
        sl_clean = false;
        sr_clean = false;
    } else {
        if (sl_edge) sl_clean = true;
        if (sr_edge) sr_clean = true;
    }
    if (other_edge) {
        if (sl) sl_clean = false;
        if (sr) sr_clean = false;
    }
    // Tocar y soltar ⇧ sin escribir: desplaza. Izquierda sube (más antiguo), derecha baja.
    if (prev_sl && !sl && sl_clean) scroll_chat(-1);
    if (prev_sr && !sr && sr_clean) scroll_chat(+1);
    prev_sl = sl;
    prev_sr = sr;

    bool up = sl || sr;
    bool alt = key_held[K_ALT_C][K_ALT_R];
    for (int c = 0; c < colCount; c++) {
        for (int r = 0; r < rowCount; r++) {
            if (!key_edge[c][r]) continue;
            if (is_shift(c, r)) continue;
            if (c == K_ALT_C && r == K_ALT_R) continue;
            if (c == K_SYM_C && r == K_SYM_R) {
                sym_pending = true;
                continue;
            }
            if (c == K_ENTER_C && r == K_ENTER_R) {
                sym_pending = false;
                send_message();
                continue;
            }
            if (c == K_BACK_C && r == K_BACK_R) {
                sym_pending = false;
                input_backspace();
                continue;
            }

            char buf[2] = {0, 0};
            const char *out = nullptr;
            if (sym_pending && KEY_SYM[c][r]) {
                buf[0] = KEY_SYM[c][r];
                out = buf;
            } else if (alt && accent_for(c, r, up)) {
                out = accent_for(c, r, up);
            } else if (KEY_NORMAL[c][r]) {
                buf[0] = KEY_NORMAL[c][r];
                if (up && buf[0] >= 'a' && buf[0] <= 'z') buf[0] = buf[0] - 'a' + 'A';
                out = buf;
            }
            sym_pending = false;
            if (out) input_append(out);
        }
    }
}

// ------------------------------------------------------------------ red y arranque

static unsigned long next_wifi_check_ms = 0;
static bool greeted = false;

static void network_maintenance() {
    unsigned long now = millis();
    if (WiFi.status() == WL_CONNECTED) {
        start_ntp();
        if (!greeted) {
            reg_add(2, "¿En qué puedo ayudarte?");
            greeted = true;
            follow = true;
            dirty = true;
        }
        return;
    }
    if (now < next_wifi_check_ms) return;
    next_wifi_check_ms = now + WIFI_RETRY_MS;
    Serial.println("WiFi caído, reconectando");
    WiFi.disconnect();
    WiFi.begin(cfg.ssid.c_str(), cfg.pass.c_str());
}

static bool offline_view = false;     // la pantalla de sin conexión sustituye al chat
static unsigned long offline_since = 0;
static unsigned long offline_page_ms = 0;
static int offline_page = 0;

static void show_offline_page(int page) {
    if (!screen_on) return;
    if (page == 0) show_lines("Sin conexión WiFi", "Reconectando...");
    else show_lines("ALT + RESET", "para nueva WiFi");
}

// Sin WiFi durante más de 5 s: páginas alternas cada 3 s. Al volver la red, vuelve el chat.
static void offline_update() {
    unsigned long now = millis();
    if (WiFi.status() == WL_CONNECTED) {
        offline_since = 0;
        if (offline_view) {
            offline_view = false;
            chat_clear_pending = true;
            dirty = true;
        }
        return;
    }
    if (offline_since == 0) offline_since = now;
    if (!offline_view) {
        if (now - offline_since < 5000) return;
        offline_view = true;
        offline_page = 0;
        offline_page_ms = now;
        show_offline_page(0);
        return;
    }
    if (now - offline_page_ms >= 3000) {
        offline_page = 1 - offline_page;
        offline_page_ms = now;
        show_offline_page(offline_page);
    }
}

// Si había una pregunta pendiente y ya hay red y hora, se reenvía sin que el usuario haga nada.
static void pending_maintenance() {
    if (!pending_on || busy) return;
    if (WiFi.status() != WL_CONNECTED || !time_ready()) return;
    String q = pending_q;
    pending_q = "";
    pending_on = false;
    reg_add(0, "Red recuperada. Reenviando...");
    run_query(q);
}

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("Asistente IA iniciando");

    for (int r = 0; r < rowCount; r++) pinMode(rows[r], INPUT);
    for (int c = 0; c < colCount; c++) pinMode(cols[c], INPUT_PULLUP);
    pinMode(KB_BACKLIGHT_PIN, OUTPUT);
    digitalWrite(KB_BACKLIGHT_PIN, HIGH);
    kb_backlight_on = true;

    memset(key_held, 0, sizeof key_held);
    memset(key_prev, 0, sizeof key_prev);
    memset(key_edge, 0, sizeof key_edge);
    scan_matrix();  // lee ALT antes de arrancar la interfaz
    memset(key_edge, 0, sizeof key_edge);
    bool alt_at_boot = key_held[K_ALT_C][K_ALT_R];

    SPI.end();
    TFT_099.begin();
    TFT_099.DispColor(0, 0, TFT_WIDTH, TFT_HEIGHT, BLACK);
    TFT_099.backlight(SCREEN_BACKLIGHT);

    show_splash();  // 3 s: AsistenteIA / autoría

    cfg = load_config();
    if (alt_at_boot || !cfg.valid) {
        if (alt_at_boot) {
            show_lines("Modo configuración", "ALT detectado");
            delay(2000);
        }
        run_portal();  // no vuelve
    }

    show_lines("Conectando", "a la WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(cfg.ssid.c_str(), cfg.pass.c_str());
    next_wifi_check_ms = millis() + WIFI_RETRY_MS;
    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) delay(100);
    if (WiFi.status() == WL_CONNECTED) {
        show_lines("Conectado", "Sincronizando hora");
        start_ntp();
        wait_time(10000);
    }

    last_key_ms = millis();
    screen_on = true;
    chat_clear_pending = true;
    dirty = true;
    delay(800);
}

void loop() {
    scan_matrix();
    handle_keys();
    network_maintenance();
    offline_update();
    pending_maintenance();

    if (screen_on && millis() - last_key_ms > SCREEN_OFF_MS && !busy) {
        TFT_099.backlight(0);
        screen_on = false;
    }
    if (dirty && screen_on && !offline_view) {
        draw_screen();
        dirty = false;
    }
    delay(20);
}

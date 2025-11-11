#include <Adafruit_NeoPixel.h>

#define LED_PIN 48
#define NUMPIXELS 1

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Duraciones Morse
#define DOT_DURATION 200
#define DASH_DURATION 600
#define SYMBOL_SPACE 200
#define LETTER_SPACE 600
#define WORD_SPACE 1400

// Código Morse
const char* morseCode[] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",
  "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-",
  "..-", "...-", ".--", "-..-", "-.--", "--.."
};

String message = "SOS";

// Funciones auxiliares
void blinkColor(uint8_t r, uint8_t g, uint8_t b, int duration) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
  delay(duration);
  pixels.clear();
  pixels.show();
  delay(SYMBOL_SPACE);
}

void morseMessage(String msg, uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < msg.length(); i++) {
    char c = toupper(msg[i]);
    if (c >= 'A' && c <= 'Z') {
      String morse = morseCode[c - 'A'];
      for (int j = 0; j < morse.length(); j++) {
        if (morse[j] == '.') {
          blinkColor(r, g, b, DOT_DURATION);
        } else if (morse[j] == '-') {
          blinkColor(r, g, b, DASH_DURATION);
        }
      }
      delay(LETTER_SPACE - SYMBOL_SPACE);
    } else if (c == ' ') {
      delay(WORD_SPACE - LETTER_SPACE);
    }
  }
}

void blinkGreen(int times) {
  for (int i = 0; i < times; i++) {
    blinkColor(0, 255, 0, 300);
    delay(200);
  }
}

// Efecto de “pulse” (aumenta brillo)
void pulseEffect(int times) {
  uint8_t colors[7][3] = {
    {255, 0, 0},     // rojo
    {0, 255, 0},     // verde
    {0, 0, 255},     // azul
    {255, 255, 0},   // amarillo
    {255, 0, 255},   // magenta
    {0, 255, 255},   // cian
    {255, 255, 255}  // blanco
  };

  for (int t = 0; t < times; t++) {
    uint8_t r = colors[t % 7][0];
    uint8_t g = colors[t % 7][1];
    uint8_t b = colors[t % 7][2];

    for (int brightness = 0; brightness <= 255; brightness += 5) {
      pixels.setPixelColor(0, pixels.Color((r * brightness) / 255, (g * brightness) / 255, (b * brightness) / 255));
      pixels.show();
      delay(20);
    }
    delay(200);
    pixels.clear();
    pixels.show();
    delay(200);
  }
}

void setup() {
  pixels.begin();
  pixels.clear();
  pixels.show();
  Serial.begin(115200);
}

void loop() {
  // 1️⃣ SOS morado
  morseMessage("SOS", 128, 0, 128); // morado

  delay(500);

  // 2️⃣ 7 blinks verdes
  blinkGreen(7);

  delay(500);

  // 3️⃣ SOS morado otra vez
  morseMessage("SOS", 128, 0, 128);

  delay(500);

  // 4️⃣ Pulse (sube brillo) 7 veces cambiando color
  pulseEffect(7);

  // Al terminar, LED apagado
  pixels.clear();
  pixels.show();

  delay(3000);
}
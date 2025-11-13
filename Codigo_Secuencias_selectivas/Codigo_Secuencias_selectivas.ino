#include <Adafruit_NeoPixel.h>

// LED integrado (RGB) en la ESP32-S3
#define LED_PIN 48
#define NUMPIXELS 1

// Pines de botones (usar GND al presionar)
#define BTN_SOS 4
#define BTN_BLINK 5
#define BTN_PULSE 7

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Duraciones Morse3
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

// ------------------------
// Función general de parpadeo
// ------------------------
void blinkColor(uint8_t r, uint8_t g, uint8_t b, int duration) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
  delay(duration);
  pixels.clear();
  pixels.show();
  delay(SYMBOL_SPACE);
}

// ------------------------
// Mostrar mensaje en código Morse
// ------------------------
void morseMessage(String msg, uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < msg.length(); i++) {
    char c = toupper(msg[i]);
    if (c >= 'A' && c <= 'Z') {
      String morse = morseCode[c - 'A'];
      for (int j = 0; j < morse.length(); j++) {
        if (morse[j] == '.') blinkColor(r, g, b, DOT_DURATION);
        else if (morse[j] == '-') blinkColor(r, g, b, DASH_DURATION);
      }
      delay(LETTER_SPACE - SYMBOL_SPACE);
    } else if (c == ' ') {
      delay(WORD_SPACE - LETTER_SPACE);
    }
  }
}

// ------------------------
// Efecto parpadeo verde
// ------------------------
void blinkGreen(int times) {
  for (int i = 0; i < times; i++) {
    blinkColor(0, 255, 0, 300);
    delay(200);
  }
}

// ------------------------
// Efecto de pulso multicolor
// ------------------------
void pulseEffect(int times) {
  uint8_t colors[7][3] = {
    {255, 0, 0}, {0, 255, 0}, {0, 0, 255},
    {255, 255, 0}, {255, 0, 255}, {0, 255, 255}, {255, 255, 255}
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

// ------------------------
// Configuración
// ------------------------
void setup() {
  pixels.begin();
  pixels.clear();
  pixels.show();
  Serial.begin(115200);

  // Configurar botones con resistencia pull-up interna
  pinMode(BTN_SOS, INPUT_PULLUP);
  pinMode(BTN_BLINK, INPUT_PULLUP);
  pinMode(BTN_PULSE, INPUT_PULLUP);

  Serial.println("Sistema listo: presiona un botón.");
}

// ------------------------
// Bucle principal
// ------------------------
void loop() {
  // Leer botones (presionados = LOW)
  if (digitalRead(BTN_SOS) == LOW) {
    Serial.println("Botón SOS presionado");
    morseMessage("SOS", 128, 0, 128); // Morado
  }

  if (digitalRead(BTN_BLINK) == LOW) {
    Serial.println("Botón Blink presionado");
    blinkGreen(7);
  }

  if (digitalRead(BTN_PULSE) == LOW) {
    Serial.println("Botón Pulse presionado");
    pulseEffect(7);
  }

  delay(100); // Pequeña pausa para estabilidad
}
#include <Adafruit_NeoPixel.h>

// ------------------------
// CONFIGURACIÓN DE PINES
// ------------------------
#define LED_PIN 48
#define NUMPIXELS 1

#define BTN_SOS   4
#define BTN_BLINK 5
#define BTN_PULSE 7
#define BTN_IRQ   15   

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// ------------------------
// VARIABLES Y CONSTANTES
// ------------------------
#define DOT_DURATION   200
#define DASH_DURATION  600
#define SYMBOL_SPACE   200
#define LETTER_SPACE   600
#define WORD_SPACE    1400

const char* morseCode[] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",
  "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-",
  "..-", "...-", ".--", "-..-", "-.--", "--.."
};

volatile bool stopPressed = false;

// ------------------------
// FUNCIÓN DE INTERRUPCIÓN
// ------------------------
void IRAM_ATTR stopISR() {
  stopPressed = true;
}

// ------------------------
// FUNCIONES AUXILIARES
// ------------------------
void clearLED() {
  pixels.clear();
  pixels.show();
}

void blinkColor(uint8_t r, uint8_t g, uint8_t b, int duration) {
  if (stopPressed) return;
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
  delay(duration);
  clearLED();
  delay(SYMBOL_SPACE);
}

void morseMessage(String msg, uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < msg.length(); i++) {
    if (stopPressed) return;
    char c = toupper(msg[i]);
    if (c >= 'A' && c <= 'Z') {
      String morse = morseCode[c - 'A'];
      for (int j = 0; j < morse.length(); j++) {
        if (stopPressed) return;
        if (morse[j] == '.') blinkColor(r, g, b, DOT_DURATION);
        else blinkColor(r, g, b, DASH_DURATION);
      }
      delay(LETTER_SPACE - SYMBOL_SPACE);
    } else if (c == ' ') {
      delay(WORD_SPACE - LETTER_SPACE);
    }
  }
}

void blinkGreen(int times) {
  for (int i = 0; i < times; i++) {
    if (stopPressed) return;
    blinkColor(0, 255, 0, 300);
    delay(200);
  }
}

void pulseEffect(int times) {
  uint8_t colors[7][3] = {
    {255, 0, 0}, {0, 255, 0}, {0, 0, 255},
    {255, 255, 0}, {255, 0, 255}, {0, 255, 255},
    {255, 255, 255}
  };

  for (int t = 0; t < times; t++) {
    if (stopPressed) return;
    uint8_t r = colors[t % 7][0];
    uint8_t g = colors[t % 7][1];
    uint8_t b = colors[t % 7][2];

    for (int brightness = 0; brightness <= 255; brightness += 5) {
      if (stopPressed) return;
      pixels.setPixelColor(0, pixels.Color(
        (r * brightness) / 255,
        (g * brightness) / 255,
        (b * brightness) / 255));
      pixels.show();
      delay(15);
    }

    delay(200);
    clearLED();
    delay(200);
  }
}

// ------------------------
// SETUP
// ------------------------
void setup() {
  pixels.begin();
  clearLED();
  Serial.begin(115200);

  pinMode(BTN_SOS, INPUT_PULLUP);
  pinMode(BTN_BLINK, INPUT_PULLUP);
  pinMode(BTN_PULSE, INPUT_PULLUP);
  pinMode(BTN_IRQ, INPUT_PULLUP);

  // Configurar interrupción por botón IRQ
  attachInterrupt(digitalPinToInterrupt(BTN_IRQ), stopISR, FALLING);

  Serial.println("Sistema listo con botón IRQ funcional.");
}

// ------------------------
// LOOP PRINCIPAL
// ------------------------
void loop() {

  if (digitalRead(BTN_SOS) == LOW) {
    stopPressed = false;
    Serial.println("SOS iniciado");
    morseMessage("SOS", 128, 0, 128);
  }

  if (digitalRead(BTN_BLINK) == LOW) {
    stopPressed = false;
    Serial.println("Blink iniciado");
    blinkGreen(7);
  }

  if (digitalRead(BTN_PULSE) == LOW) {
    stopPressed = false;
    Serial.println("Pulse iniciado");
    pulseEffect(7);
  }

  // Si se activa interrupción (IRQ)
  if (stopPressed) {
    Serial.println(">> INTERRUPCIÓN ACTIVADA <<");
    clearLED();
    delay(200); // pequeña pausa para evitar rebotes
  }

  delay(100);
}
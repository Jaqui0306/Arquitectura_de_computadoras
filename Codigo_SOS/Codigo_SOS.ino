#include <Adafruit_NeoPixel.h>

#define LED_PIN 48      // 
#define NUMPIXELS 1     // 

// Inicializa el LED RGB
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

void setup() {
  pixels.begin();
  pixels.clear();
  pixels.show();
  Serial.begin(115200);
}

void blinkColor(uint8_t r, uint8_t g, uint8_t b, int duration) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
  delay(duration);
  pixels.clear();
  pixels.show();
}

// Simulación de tu fragmento con RGB_BUILTIN
void testRGB() {
  // Encender el LED en blanco por 1 segundo
  pixels.setPixelColor(0, pixels.Color(255, 0, 255));
  pixels.show();
  delay(1000);

  // Apagar el LED por 1 segundo
  pixels.clear();
  pixels.show();
  delay(1000);
}

void loop() {
  // Primero hacemos la prueba del LED
  testRGB();

  // Luego ejecutamos el mensaje en código Morse
  for (int i = 0; i < message.length(); i++) {
    char c = toupper(message[i]);
    if (c >= 'A' && c <= 'Z') {
      String morse = morseCode[c - 'A'];
      for (int j = 0; j < morse.length(); j++) {
        if (morse[j] == '.') {
          blinkColor(255, 0, 255, DOT_DURATION);
          delay(SYMBOL_SPACE);
        } else if (morse[j] == '-') {
          blinkColor(255, 0, 255, DASH_DURATION);
          delay(SYMBOL_SPACE);
        }
      }
      delay(LETTER_SPACE - SYMBOL_SPACE);
    } else if (c == ' ') {
      delay(WORD_SPACE - LETTER_SPACE);
    }
  }
  delay(2000);
}
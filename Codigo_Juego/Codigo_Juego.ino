// =================================================================
// PROYECTO: JUEGO DE ADIVINANZA ESP32-S3  (LED ÁNODO COMÚN)
// =================================================================

// --- DEFINICIÓN DE PINES ---

#define NUM_POS 6

// Pines LED {Rojo, Verde} – ÁNODO COMÚN
int PIN_LED_RG[NUM_POS][2] = {
  {2, 18},     // LED 0
  {15, 5},     // LED 1
  {6, 7},     // LED 2
  {8, 9},     // LED 3
  {10, 11},   // LED 4
  {12, 13}    // LED 5
};

// Pines botones del ESP32-S3
int PIN_BTN[NUM_POS] = {37, 42, 45, 46, 38, 39};

#define RESET_BTN 40   // Botón de reinicio físico

// =================================================================
// VARIABLES DEL JUEGO
// =================================================================
int ledGanador = 0;       
int intentosRestantes = 3; 
bool juegoActivo = false;  

const int ROJO = 0;
const int VERDE = 1;

// =================================================================
// FUNCIONES
// =================================================================

// Para LEDs de ÁNODO COMÚN -> APAGAR = HIGH
void apagarTodos() {
  for (int i = 0; i < NUM_POS; i++) {
    digitalWrite(PIN_LED_RG[i][ROJO], HIGH);
    digitalWrite(PIN_LED_RG[i][VERDE], HIGH);
  }
}

// Ánodo común -> ENCENDER = LOW
void encenderTodos(int colorIndex) {
  apagarTodos();
  for (int i = 0; i < NUM_POS; i++) {
    digitalWrite(PIN_LED_RG[i][colorIndex], LOW);
  }
}

void encenderLED(int ledIndex, int colorIndex) {
  digitalWrite(PIN_LED_RG[ledIndex][ROJO], HIGH);
  digitalWrite(PIN_LED_RG[ledIndex][VERDE], HIGH);
  digitalWrite(PIN_LED_RG[ledIndex][colorIndex], LOW);
}

void iniciarJuego() {
  apagarTodos();
  ledGanador = random(0, NUM_POS);
  intentosRestantes = 3;
  juegoActivo = true;

  Serial.println("--- NUEVO JUEGO ---");
  Serial.print("Led Ganador (oculto): ");
  Serial.println(ledGanador + 1);
}

void animacionFinal(int colorIndex) {
  juegoActivo = false;

  encenderTodos(colorIndex);
  delay(1000);

  for (int k = 0; k < 2; k++) {
    apagarTodos();
    delay(300);
    encenderTodos(colorIndex);
    delay(300);
  }

  delay(500);
  iniciarJuego();
}

void procesarBoton(int botonPresionado) {
  if (!juegoActivo) return;

  if (botonPresionado == ledGanador) {
    Serial.println("¡GANASTE!");
    animacionFinal(VERDE);

  } else {
    Serial.println("Incorrecto");
    intentosRestantes--;

    encenderLED(botonPresionado, ROJO);

    if (intentosRestantes <= 0) {
      Serial.println("PERDISTE");
      delay(500);
      animacionFinal(ROJO);
    }
  }
}

// =================================================================
// SETUP Y LOOP
// =================================================================
void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(1));

  for (int i = 0; i < NUM_POS; i++) {
    pinMode(PIN_LED_RG[i][ROJO], OUTPUT);
    pinMode(PIN_LED_RG[i][VERDE], OUTPUT);
    pinMode(PIN_BTN[i], INPUT_PULLUP);

    // ÁNODO COMÚN -> apagar = HIGH
    digitalWrite(PIN_LED_RG[i][ROJO], HIGH);
    digitalWrite(PIN_LED_RG[i][VERDE], HIGH);
  }

  pinMode(RESET_BTN, INPUT_PULLUP);

  Serial.println("Listo.");
  delay(500);
  iniciarJuego();
}

void loop() {

  // BOTÓN RESET
  if (digitalRead(RESET_BTN) == LOW) {
    delay(100);
    if (digitalRead(RESET_BTN) == LOW) iniciarJuego();
  }

  // BOTONES DEL JUEGO
  for (int i = 0; i < NUM_POS; i++) {
    if (digitalRead(PIN_BTN[i]) == LOW) {
      delay(50);
      if (digitalRead(PIN_BTN[i]) == LOW) {
        procesarBoton(i);
        while (digitalRead(PIN_BTN[i]) == LOW) delay(10);
      }
    }
  }
}
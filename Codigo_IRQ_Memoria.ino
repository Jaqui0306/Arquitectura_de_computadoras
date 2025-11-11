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

enum TaskType { NONE, SOS, BLINK, PULSE };

struct TaskState {
  TaskType type;
  int step;
  String msg;
};

#define MAX_TASKS 5
TaskState taskQueue[MAX_TASKS];
int queueStart = 0;
int queueEnd = 0;

TaskState currentTask = { NONE, 0, "" };

volatile bool irqPressed = false;
bool paused = false;

// ------------------------
// FUNCIONES BÁSICAS
// ------------------------
void IRAM_ATTR irqISR() {
  irqPressed = true;
}

void clearLED() {
  pixels.clear();
  pixels.show();
}

// delay fraccionado para permitir interrupciones inmediatas
bool safeDelay(int totalMs) {
  int waited = 0;
  const int slice = 10;
  while (waited < totalMs) {
    if (irqPressed ||
        digitalRead(BTN_SOS) == LOW ||
        digitalRead(BTN_BLINK) == LOW ||
        digitalRead(BTN_PULSE) == LOW)
      return false;
    delay(slice);
    waited += slice;
  }
  return true;
}

bool blinkColor(uint8_t r, uint8_t g, uint8_t b, int duration) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
  if (!safeDelay(duration)) { clearLED(); return false; }
  clearLED();
  return safeDelay(SYMBOL_SPACE);
}

// ------------------------
// COLA DE TAREAS
// ------------------------
bool enqueueTask(TaskState task) {
  int next = (queueEnd + 1) % MAX_TASKS;
  if (next == queueStart) return false;
  taskQueue[queueEnd] = task;
  queueEnd = next;
  return true;
}

bool dequeueTask(TaskState &task) {
  if (queueStart == queueEnd) return false;
  task = taskQueue[queueStart];
  queueStart = (queueStart + 1) % MAX_TASKS;
  return true;
}

bool queueEmpty() {
  return (queueStart == queueEnd);
}

// ------------------------
// FUNCIONES DE TAREAS
// ------------------------
bool runSOS(TaskState &task) {
  String msg = task.msg;
  for (int i = task.step; i < msg.length(); i++) {
    if (irqPressed) { task.step = i; return false; }
    char c = toupper(msg[i]);
    if (c >= 'A' && c <= 'Z') {
      String morse = morseCode[c - 'A'];
      for (int j = 0; j < morse.length(); j++) {
        if (!blinkColor(255, 0, 0, morse[j] == '.' ? DOT_DURATION : DASH_DURATION))
          { task.step = i; return false; }
      }
      if (!safeDelay(LETTER_SPACE - SYMBOL_SPACE)) { task.step = i; return false; }
    } else if (c == ' ') {
      if (!safeDelay(WORD_SPACE - LETTER_SPACE)) { task.step = i; return false; }
    }
  }
  return true;
}

bool runBlink(TaskState &task) {
  for (int i = task.step; i < 8; i++) {
    if (!blinkColor(0, 255, 0, 300)) { task.step = i; return false; }
    if (!safeDelay(200)) { task.step = i; return false; }
  }
  return true;
}

bool runPulse(TaskState &task) {
  uint8_t colors[7][3] = {
    {255, 0, 0}, {0, 255, 0}, {0, 0, 255},
    {255, 255, 0}, {255, 0, 255}, {0, 255, 255},
    {255, 255, 255}
  };

  for (int t = task.step; t < 7; t++) {
    if (irqPressed) { task.step = t; return false; }

    uint8_t r = colors[t % 7][0];
    uint8_t g = colors[t % 7][1];
    uint8_t b = colors[t % 7][2];

    for (int brightness = 0; brightness <= 255; brightness += 5) {
      if (irqPressed) { task.step = t; return false; }
      pixels.setPixelColor(0, pixels.Color(
        (r * brightness) / 255,
        (g * brightness) / 255,
        (b * brightness) / 255));
      pixels.show();
      if (!safeDelay(15)) { task.step = t; return false; }
    }

    clearLED();
    if (!safeDelay(200)) { task.step = t; return false; }
  }
  return true;
}

// ------------------------
// LOOP PRINCIPAL
// ------------------------
void setup() {
  pixels.begin();
  clearLED();
  Serial.begin(115200);

  pinMode(BTN_SOS, INPUT_PULLUP);
  pinMode(BTN_BLINK, INPUT_PULLUP);
  pinMode(BTN_PULSE, INPUT_PULLUP);
  pinMode(BTN_IRQ, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(BTN_IRQ), irqISR, FALLING);

  Serial.println("Sistema multitarea con interrupciones listo.");
}

void loop() {
  // --- BOTONES PARA TAREAS ---
  if (digitalRead(BTN_SOS) == LOW) {
    currentTask = { SOS, 0, "SOS" };
    paused = false; irqPressed = false;
    Serial.println("Iniciando SOS");
    delay(200);
  }
  if (digitalRead(BTN_BLINK) == LOW) {
    currentTask = { BLINK, 0, "" };
    paused = false; irqPressed = false;
    Serial.println("Iniciando BLINK");
    delay(200);
  }
  if (digitalRead(BTN_PULSE) == LOW) {
    currentTask = { PULSE, 0, "" };
    paused = false; irqPressed = false;
    Serial.println("Iniciando PULSE");
    delay(200);
  }

  // --- INTERRUPCIÓN POR IRQ ---
  if (irqPressed) {
    irqPressed = false;
    if (!paused && currentTask.type != NONE) {
      enqueueTask(currentTask);
      Serial.println("Tarea pausada y guardada en cola");
      currentTask.type = NONE;
      paused = true;
    } else if (paused && !queueEmpty()) {
      dequeueTask(currentTask);
      Serial.println("Reanudando tarea guardada");
      paused = false;
    } else {
      Serial.println("Nada que pausar o reanudar");
    }
    delay(250);
  }

  // --- EJECUCIÓN ---
  if (!paused && currentTask.type != NONE) {
    bool done = false;
    switch (currentTask.type) {
      case SOS:   done = runSOS(currentTask); break;
      case BLINK: done = runBlink(currentTask); break;
      case PULSE: done = runPulse(currentTask); break;
      default: break;
    }

    if (done) {
      Serial.println("Tarea completada");
      currentTask.type = NONE;
    }
  }

  delay(5);
}
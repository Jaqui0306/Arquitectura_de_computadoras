#include <Keypad.h>
#include <SPI.h>

// ==========================================
//  PINES MAX7219 (ESP32-S3)
// ==========================================
#define DIN_PIN 10   // MOSI
#define CLK_PIN 9    // SCK
#define CS_PIN  11   // SS

// ==========================================
//  FUNCIONES MAX7219
// ==========================================
void max7219Enviar(byte dir, byte val) {
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(dir);
  SPI.transfer(val);
  digitalWrite(CS_PIN, HIGH);
}

void max7219Init() {
  pinMode(CS_PIN, OUTPUT);
  SPI.begin(CLK_PIN, -1, DIN_PIN);

  max7219Enviar(0x09, 0x00);
  max7219Enviar(0x0A, 0x03); // brillo
  max7219Enviar(0x0B, 0x07);
  max7219Enviar(0x0C, 0x01);
  max7219Enviar(0x0F, 0x00);
}

// ==========================================
// MATRIZ
// ==========================================
byte matrizRAM[8];

void limpiarMatriz() {
  for (int i = 0; i < 8; i++) {
    matrizRAM[i] = 0;
    max7219Enviar(i + 1, 0);
  }
}

void refrescarMatriz() {
  for (int i = 0; i < 8; i++) {
    max7219Enviar(i + 1, matrizRAM[i]);
  }
}

void setPixel(int x, int y, bool val) {
  if (x < 0 || x > 7 || y < 0 || y > 7) return;

  if (val) matrizRAM[y] |=  (1 << (7 - x));
  else     matrizRAM[y] &= ~(1 << (7 - x));
}

// ==========================================
// TABLERO (#)
// ==========================================
void dibujarLineasTablero() {
  for (int y = 0; y < 8; y++) {
    setPixel(2, y, true);
    setPixel(5, y, true);
  }
  
  for (int x = 0; x < 8; x++) {
    setPixel(x, 2, true);
    setPixel(x, 5, true);
  }
}

// ==========================================
// TECLADO 4×3
// ==========================================
const byte FILAS = 4;
const byte COLUMNAS = 3;

char teclas[FILAS][COLUMNAS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte pinesFilas[FILAS]       = {4, 5, 6, 7};
byte pinesColumnas[COLUMNAS] = {15,16,17};

Keypad teclado = Keypad(makeKeymap(teclas), pinesFilas, pinesColumnas, FILAS, COLUMNAS);

// ==========================================
// JUEGO
// ==========================================
int tablero[9];
int ganador = 0;  // 0 = nadie, 1 = jugador, 2 = CPU

const int lineas[8][3] = {
  {0,1,2},{3,4,5},{6,7,8},
  {0,3,6},{1,4,7},{2,5,8},
  {0,4,8},{2,4,6}
};

// ==========================================
// PIEZAS 2×2
// ==========================================
void dibujarJugador(int pos) {
  int bx = (pos % 3) * 3;
  int by = (pos / 3) * 3;

  setPixel(bx,   by,   true);
  setPixel(bx+1, by,   true);
  setPixel(bx,   by+1, true);
  setPixel(bx+1, by+1, true);
}

void dibujarCPU(int pos) {
  int bx = (pos % 3) * 3;
  int by = (pos / 3) * 3;

  setPixel(bx+1, by,   true);
  setPixel(bx,   by+1, true);
  setPixel(bx+1, by+1, true);
}

// ==========================================
// REFRESCAR TABLERO
// ==========================================
void refrescarTablero() {
  limpiarMatriz();
  dibujarLineasTablero();

  for (int i = 0; i < 9; i++) {
    if (tablero[i] == 1) dibujarJugador(i);
    if (tablero[i] == 2) dibujarCPU(i);
  }

  refrescarMatriz();
}

// ==========================================
// IA DEL CPU
// ==========================================
int buscarJugada(int jugador) {
  for (int i = 0; i < 8; i++) {
    int a = lineas[i][0];
    int b = lineas[i][1];
    int c = lineas[i][2];

    if (tablero[a] == jugador && tablero[b] == jugador && tablero[c] == 0) return c;
    if (tablero[a] == jugador && tablero[c] == jugador && tablero[b] == 0) return b;
    if (tablero[b] == jugador && tablero[c] == jugador && tablero[a] == 0) return a;
  }
  return -1;
}

void movimientoCPU() {
  int jugada;

  jugada = buscarJugada(2);
  if (jugada != -1) { tablero[jugada] = 2; return; }

  jugada = buscarJugada(1);
  if (jugada != -1) { tablero[jugada] = 2; return; }

  if (tablero[4] == 0) { tablero[4] = 2; return; }

  int esquinas[4] = {0,2,6,8};
  for (int i = 0; i < 4; i++)
    if (tablero[esquinas[i]] == 0) { tablero[esquinas[i]] = 2; return; }

  int lados[4] = {1,3,5,7};
  for (int i = 0; i < 4; i++)
    if (tablero[lados[i]] == 0) { tablero[lados[i]] = 2; return; }
}

void reiniciarJuego() {
  for (int i = 0; i < 9; i++) tablero[i] = 0;
  ganador = 0;
  refrescarTablero();
}

// ==========================================
// DETECCIÓN DE GANADOR
// ==========================================
int verificarGanador() {
  for (int i = 0; i < 8; i++) {
    int a = lineas[i][0];
    int b = lineas[i][1];
    int c = lineas[i][2];

    if (tablero[a] != 0 && tablero[a] == tablero[b] && tablero[a] == tablero[c])
      return tablero[a];
  }
  return 0;
}

bool tableroLleno() {
  for (int i = 0; i < 9; i++)
    if (tablero[i] == 0) return false;
  return true;
}

// ==========================================
// ANIMACIÓN EN CARACOL
// ==========================================
void animacionCaracol() {
  limpiarMatriz();
  refrescarMatriz();
  delay(150);

  int left = 0, right = 7, top = 0, bottom = 7;

  while (left <= right && top <= bottom) {
    for (int x = left; x <= right; x++) { setPixel(x, top, true); refrescarMatriz(); delay(40); }
    top++;

    for (int y = top; y <= bottom; y++) { setPixel(right, y, true); refrescarMatriz(); delay(40); }
    right--;

    for (int x = right; x >= left; x--) { setPixel(x, bottom, true); refrescarMatriz(); delay(40); }
    bottom--;

    for (int y = bottom; y >= top; y--) { setPixel(left, y, true); refrescarMatriz(); delay(40); }
    left++;
  }

  delay(300);
}

// ==========================================
// MOSTRAR NÚMERO (1 o 2)
// ==========================================
void mostrarNumero(int n) {
  limpiarMatriz();

  if (n == 1) {
    for (int y = 1; y <= 6; y++) setPixel(4, y, true);
    setPixel(3, 2, true);
  }
  else if (n == 2) {
    for (int x = 2; x <= 5; x++) setPixel(x, 1, true);
    for (int x = 2; x <= 5; x++) setPixel(x, 4, true);
    for (int x = 2; x <= 5; x++) setPixel(x, 6, true);

    setPixel(5, 2, true);
    setPixel(2, 5, true);
  }

  refrescarMatriz();
}

// ==========================================
// MOSTRAR LETRA E (EMPATE)
// ==========================================
void mostrarEmpate() {
  limpiarMatriz();

  for (int y = 1; y <= 6; y++) setPixel(2, y, true);

  for (int x = 2; x <= 5; x++) setPixel(x, 1, true);
  for (int x = 2; x <= 5; x++) setPixel(x, 3, true);
  for (int x = 2; x <= 5; x++) setPixel(x, 6, true);

  refrescarMatriz();
}

// ==========================================
// SETUP
// ==========================================
void setup() {
  Serial.begin(115200);
  max7219Init();
  reiniciarJuego();
  Serial.println("Juego listo");
}

// ==========================================
// LOOP
// ==========================================
void loop() {

  // REINICIO EN CUALQUIER MOMENTO
  char tecla = teclado.getKey();
  if (tecla == '*') {
    reiniciarJuego();
    return;
  }

  if (!tecla) return;

  if (ganador != 0) return;

  // ===============================
  // Movimiento del jugador
  // ===============================
  if (tecla >= '1' && tecla <= '9') {
    int pos = tecla - '1';

    if (tablero[pos] == 0) {
      tablero[pos] = 1;
      ganador = verificarGanador();
      refrescarTablero();

      if (ganador == 1) {
        animacionCaracol();
        mostrarNumero(2);
        delay(1200);   // mostrar resultado
        reiniciarJuego();  // reset automático
        return;
      }

      if (!tableroLleno()) {
        movimientoCPU();
        ganador = verificarGanador();
      }

      refrescarTablero();

      if (ganador == 2) {
        animacionCaracol();
        mostrarNumero(1);
        delay(1200);
        reiniciarJuego();   // reset automático
        return;
      }

      if (tableroLleno()) {
        animacionCaracol();
        mostrarEmpate();
        delay(1200);
        reiniciarJuego();   // reset automático
        return;
      }
    }
  }
}
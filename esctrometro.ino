#include <TFT_HX8357.h>
#include <SD.h>

TFT_HX8357 tft = TFT_HX8357();

#define CENTRE 240
#define clockPin 2
#define siPin 3
#define VOUT A0
#define INTVAL A1
#define Pino_SD 53
#define pinCalibracao 7
#define pinGravacao 13

int Faktor = 4;
long exposure;
int brilhos[128];
int I0[128]; // Valores de referência
float transmitancia[128];
float absorbancia[128];
bool modoCalibracao = false;

void setup() {
  Serial.begin(38400);
  pinMode(Pino_SD, OUTPUT);
  pinMode(siPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(pinCalibracao, INPUT_PULLUP); // Botão de calibração
  pinMode(pinGravacao, INPUT_PULLUP);  // Botão de gravação
  
  // Setup do LCD
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(tft.color565(255, 255, 255), tft.color565(0, 0, 0));

  tft.fillRect(0, 0, 480, 14, tft.color565(210, 210, 210));
  tft.drawCentreString("Espectrometro", CENTRE, 3, 1);

  // Inicializa valores de I0 como zero
  for (int i = 0; i < 128; i++) {
    I0[i] = 0;
  }
}

void loop() {
  // Verifica se o botão de calibração foi pressionado
  if (digitalRead(pinCalibracao) == LOW) {
    modoCalibracao = true;
    capturaI0();
    modoCalibracao = false;
  }

  if (digitalRead(pinGravacao)== LOW) {
    while (digitalRead(pinGravacao) == LOW) {
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(tft.color565(255, 255, 255), tft.color565(0, 0, 0));
      tft.print("Gravando os dados no Cartão");

      Inicializa_SDcard();

      getCamera();

      Dados();
      tft.fillScreen(TFT_BLACK);
      return;
    }
  }
  tft.fillRect(10, 15, 382, 285, TFT_BLACK);

  exposure = analogRead(INTVAL);
  exposure /= 4;

  // Captura os valores de brilho
  getCamera();

  // Calcula e exibe T e A se não estiver no modo de calibração
  if (!modoCalibracao) {
    calculaAnalises();
  }

  // Desenha gráfico
  for (int i = 0; i < 128; i++) {
    int x = 10 + i * 3;
    int y1 = 299;
    int y2 = y1 - brilhos[i];

    tft.drawLine(x, y1, x, y2, GetColorByIndex(i));
  }

  delay(900);
}

void capturaI0() {
  tft.fillScreen(TFT_BLACK);
  tft.drawCentreString("Modo Calibracao", CENTRE, 100, 1);

  getCamera(); // Captura os valores iniciais de referência

  for (int i = 0; i < 128; i++) {
    I0[i] = brilhos[i];
  }

  tft.fillScreen(TFT_BLACK);
  tft.drawCentreString("I0 capturado!", CENTRE, 100, 1);
  delay(1000);
}

void calculaAnalises() {
  for (int i = 0; i < 128; i++) {
    if (I0[i] > 0) {
      transmitancia[i] = (float) brilhos[i] / I0[i];
      absorbancia[i] = -log10(transmitancia[i]);
    } else {
      transmitancia[i] = 0;
      absorbancia[i] = 0;
    }
  }

  // Exibe um exemplo no display e no Serial Monitor
  tft.fillRect(410, 160, 70, 10, TFT_BLACK);
  tft.drawNumber((int)(transmitancia[64] * 100), 428, 160, 1); // Exemplo de transmitância para 520 nm

  Serial.println("Transmitância e Absorbância:");
  for (int i = 0; i < 128; i++) {
    Serial.print("T[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.print(transmitancia[i]);
    Serial.print("  A[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(absorbancia[i]);
  }
}

void getCamera() {
  digitalWrite(clockPin, LOW);
  digitalWrite(siPin, HIGH);
  digitalWrite(clockPin, HIGH);
  digitalWrite(siPin, LOW);
  digitalWrite(clockPin, LOW);

  for (int j = 0; j < 128; j++) {
    digitalWrite(clockPin, HIGH);
    digitalWrite(clockPin, LOW);
  }

  delay(exposure);
  digitalWrite(siPin, HIGH);
  digitalWrite(clockPin, HIGH);
  digitalWrite(siPin, LOW);
  digitalWrite(clockPin, LOW);

  for (int j = 0; j < 128; j++) {
    delayMicroseconds(20);
    brilhos[j] = analogRead(VOUT) / Faktor;
    digitalWrite(clockPin, HIGH);
    digitalWrite(clockPin, LOW);
  }

  delayMicroseconds(20);
}

void Dados() {
  File myFile = SD.open("Dados.txt", FILE_WRITE);
  if (myFile) {
    for (int i = 0; i < 128; i++) {
      myFile.println(brilhos[i]);
    }
    myFile.close();
  }
}

void Inicializa_SDcard() {
  Serial.print(F("SDCARD INICIALIZANDO...."));
  if (!SD.begin(Pino_SD)) {
    Serial.println(F("Erro"));
    return;
  }
}
uint16_t GetColorByIndex(int index) {
  int red, green, blue;

  if (index >= 0 && index < 32) {
    red = 0;
    green = 0;
    blue = map(index, 0, 31, 160, 255);
  } else if (index >= 32 && index < 64) {
    red = 0;
    green = map(index, 32, 63, 200, 255);
    blue = 0;
  } else if (index >= 64 && index < 96) {
    red = map(index, 64, 95, 255, 255);
    green = map(index, 64, 95, 255, 255);
    blue = 0;
  } else if (index >= 96 && index < 128) {
    red = map(index, 96, 127, 255, 160);
    green = 0;
    blue = 0;
  } else {
    red = 0;
    green = 0;
    blue = 0;
  }

  return tft.color565(red, green, blue);
}

#include <TFT_HX8357.h>
#include <SD.h>

TFT_HX8357 tft = TFT_HX8357();

#define CENTRE 240
#define clockPin 2
#define siPin 3
#define VOUT A0
#define INTVAL A1
#define Pino_SD 53
#define Pino_Botao 13

int Faktor = 4;
long exposure;
int brilhos[128];
byte Status_Botao;

void setup() {
  Serial.begin(38400);
  pinMode(Pino_Botao, INPUT_PULLUP);
  pinMode(Pino_SD, OUTPUT);
  pinMode(siPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  // Setup the LCD
  tft.init();
  tft.setRotation(1);

  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 0, 480, 14, tft.color565(210, 210, 210));

  tft.setTextColor(tft.color565(255, 255, 255), tft.color565(0, 0, 0));
  tft.drawCentreString("400", 10 + 19, 310, 1);
  tft.drawCentreString("440", 10 + 19 + 1 * 50.6, 310, 1);
  tft.drawCentreString("480", 10 + 19 + 2 * 50.6, 310, 1);
  tft.drawCentreString("520", 10 + 19 + 3 * 50.6, 310, 1);
  tft.drawCentreString("560", 10 + 19 + 4 * 50.6, 310, 1);
  tft.drawCentreString("600", 10 + 19 + 5 * 50.6, 310, 1);
  tft.drawCentreString("640", 10 + 19 + 6 * 50.6, 310, 1);
  tft.drawCentreString("680", 10 + 19 + 7 * 50.6, 310, 1);
  tft.drawCentreString("[nm]", 420, 310, 1);

  tft.drawCentreString("exposicao =", 438, 140, 1);
  tft.drawCentreString("msec", 438, 180, 1);
}

void loop() {
  //Rotina Para Salvar os dados no Cartao SD quando o Botao é pressionado
  Status_Botao = digitalRead(Pino_Botao);

  if (Status_Botao == LOW) {
    while (Status_Botao == LOW) {
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

  tft.drawLine(10, 300, 10 + 380, 300, tft.color565(255, 255, 255));

  tft.fillRect(10, 15, 382, 285, TFT_BLACK);

  exposure = analogRead(INTVAL);
  exposure /= 4;

  getCamera();

  tft.fillRect(410, 160, 70, 10, TFT_BLACK);
  tft.setTextColor(tft.color565(255, 255, 255), tft.color565(0, 0, 0));
  tft.drawNumber(exposure, 428, 160, 1);

  for (int i = 0; i < 128; i++) {
    int x = 10 + i *3;
    int y1 = 299;
    int y2 = y1 - brilhos[i];

    tft.drawLine(x, y1, x, y2, GetColorByIndex(i));
  }

  tft.setTextColor(tft.color565(20, 20, 255), tft.color565(210, 210, 210));
  tft.drawCentreString("Espctrometro", CENTRE, 3, 1);

  delay(900);
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


/*uint16_t GetColorByIndex(int index) {
  int red, green, blue;

  if (index >= 0 && index < 32) {
    // Azul
    red = 0;
    green = 0;
    blue = 255;
  } else if (index >= 32 && index < 64) {
    // Verde
    red = 0;
    green = 255;
    blue = 0;
  } else if (index >= 64 && index < 96) {
    // Amarelo
    red = 255;
    green = 255;
    blue = 0;
  } else if (index >= 96 && index < 128) {
    // Vermelho
    red = 255;
    green = 0;
    blue = 0;
  } else {
    // Preto (caso de índice inválido)
    red = 0;
    green = 0;
    blue = 0;
  }

  return tft.color565(red, green, blue);
}
*/
uint16_t GetColorByIndex(int index) {
  int red, green, blue;

  if (index >= 0 && index < 32) {
    // Azul (400-440 nm)
    red = 0;
    green = 0;
    blue = map(index, 0, 31, 160, 255);
  } else if (index >= 32 && index < 64) {
    // Verde (440-520 nm)
    red = 0;
    green = map(index, 32, 63, 200, 255);
    blue = 0;
  } else if (index >= 64 && index < 96) {
    // Amarelo (520-600 nm)
    red = map(index, 64, 95, 255, 255);
    green = map(index, 64, 95, 255, 255);
    blue = 0;
  } else if (index >= 96 && index < 128) {
    // Vermelho (600-700 nm)
    red = map(index, 96, 127, 255, 160);
    green = 0;
    blue = 0;
  } else {
    // Preto (caso de índice inválido)
    red = 0;
    green = 0;
    blue = 0;
  }

  return tft.color565(red, green, blue);
}

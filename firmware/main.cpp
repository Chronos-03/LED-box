#include <Arduino.h>
#include <SPI.h>

// ================ PIN DEFINITIONS ================
#define LATCH_PIN   5
#define OE_PIN      4

uint8_t cube[8][8];

void clearCube();
void setVoxel(uint8_t x, uint8_t y, uint8_t z, bool on);
void drawCube();
void rain();
void wave();
void planeBounce();
void sparkle();

void setup() {
  Serial.begin(115200);
  
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(OE_PIN, OUTPUT);
  digitalWrite(OE_PIN, LOW);

  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));

  clearCube();
  Serial.println("Chronos LightCube Started");
}

void loop() {
  rain();
  // wave();
  // planeBounce();
  // sparkle();
}


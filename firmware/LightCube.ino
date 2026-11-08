/*
 * Chronos LightCube Firmware
 * 8x8x8 LED Cube - ESP32 + 74HC595 + TIP127
 * Author: Chronos
 */

#include <SPI.h>

// ================ PIN DEFINITIONS ================
#define LATCH_PIN   5       // RCLK of all 74HC595
#define OE_PIN      4       // Output Enable (active LOW)

// ================ CUBE BUFFER ================
// cube[layer][row] → each byte represents 8 columns
uint8_t cube[8][8];

// ================ SETUP ================
void setup() {
  Serial.begin(115200);
  
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(OE_PIN, OUTPUT);
  digitalWrite(OE_PIN, LOW);          // Enable outputs

  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));

  clearCube();
  Serial.println("Chronos LightCube Started");
}

// ================ MAIN LOOP ================
void loop() {
  // Change the effect here:
  rain();
  // wave();
  // planeBounce();
  // sparkle();
}

// ================ CORE FUNCTIONS ================

void clearCube() {
  memset(cube, 0, sizeof(cube));
}

void setVoxel(uint8_t x, uint8_t y, uint8_t z, bool on) {
  if (x > 7 || y > 7 || z > 7) return;
  if (on)
    cube[z][y] |= (1 << x);
  else
    cube[z][y] &= ~(1 << x);
}

void drawCube() {
  for (uint8_t layer = 0; layer < 8; layer++) {
    
    // Send 8 bytes for the 8 rows of this layer
    for (int8_t row = 7; row >= 0; row--) {
      SPI.transfer(cube[layer][row]);
    }

    // Send layer select byte
    // TIP127 is PNP → active LOW → invert the bit
    uint8_t layerSelect = ~(1 << layer);
    SPI.transfer(layerSelect);

    // Latch the data
    digitalWrite(LATCH_PIN, HIGH);
    digitalWrite(LATCH_PIN, LOW);

    delayMicroseconds(900);   // Display time per layer (tune if needed)
  }
}

// ================ EFFECTS ================

void rain() {
  static uint8_t drops[8][8] = {0};
  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate > 70) {
    lastUpdate = millis();

    // Move existing drops down
    for (int z = 7; z > 0; z--) {
      for (int y = 0; y < 8; y++) {
        drops[z][y] = drops[z-1][y];
      }
    }

    // Clear top layer and add new drops
    for (int y = 0; y < 8; y++) {
      drops[0][y] = 0;
    }

    for (int i = 0; i < 4; i++) {
      int x = random(8);
      int y = random(8);
      bitSet(drops[0][y], x);
    }

    // Copy to main cube buffer
    memcpy(cube, drops, sizeof(cube));
  }

  drawCube();
}

void wave() {
  static float phase = 0;
  phase += 0.12;

  clearCube();

  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      float dist = sqrt(pow(x - 3.5, 2) + pow(y - 3.5, 2));
      int z = constrain(3.5 + 3.2 * sin(dist * 0.7 - phase), 0, 7);
      setVoxel(x, y, z, true);
    }
  }

  drawCube();
}

void planeBounce() {
  static int z = 0;
  static int direction = 1;
  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate > 55) {
    lastUpdate = millis();
    z += direction;
    if (z >= 7 || z <= 0) direction = -direction;
  }

  clearCube();
  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      setVoxel(x, y, z, true);
    }
  }

  drawCube();
}

void sparkle() {
  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate > 35) {
    lastUpdate = millis();
    clearCube();
    for (int i = 0; i < 30; i++) {
      setVoxel(random(8), random(8), random(8), true);
    }
  }

  drawCube();
}
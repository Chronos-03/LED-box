# LED-BoX: 8x8x8 Audio-Reactive LED Cube

![Full 3D Render](IMAGES/CAD1.png)
![Full 3D Render](Images/CAD2.png)


**LED-BoX** is a 512-voxel (8x8x8) 3D LED cube powered by an **ESP32-WROOM-32** microcontroller. Designed by using SPI-multiplexed shift register architecture  and Darlington transistor layer drivers , the cube renders real-time 3D animations and audio-reactive spectrum visualizations. The system features dual audio reactivity, allowing it to operate in an ambient sound mode that captures room audio and voice frequencies via a built-in **INMP441 I2S digital MEMS microphone** to generate real-time frequency-reactive patterns, as well as a Bluetooth audio mode that connects wirelessly via the ESP32's onboard Bluetooth to synchronize 3D visual effects directly with playing music tracks.

---

## Features & Hardware Architecture

### 1. 512-LED Hierarchical Matrix Design (L0_A – L7_A & COL_00 – COL_77)
To keep the schematic clean, readable, and modular, the KiCad project is structured into **9 hierarchical sheets** where the root sheet ties everything together. The first 8 sheets represent the horizontal LED layers (Layer 0 through Layer 7), with each sheet containing an 8 x 8 grid of 64 LEDs to make up the 512 LEDs in total. Within each horizontal layer sheet, all 64 LED anodes are tied together onto a dedicated global layer net—labeled L0_A for the first sheet, L1_A for the second 8x8 matrix sheet, and continuing sequentially up to L7_A for the eighth sheet. Meanwhile, the LED cathodes are wired vertically across all 8 layers using an (X, Y) coordinate naming convention starting from COL_00, COL_01, and COL_02 all the way up to COL_77, forming 64 independent vertical cathode columns. The 9th sheet serves as the main controller and driver unit, housing the ESP32-DEVKITC, the 9-chip 74HC595 serial cascade, the 8 TIP127 layer drivers, the `INMP441` microphone interface, and the 5V 5A power distribution network that powers the entire build.

![Hierarchical Schematics](IMAGES/Root.png)
![LED Schematics](IMAGES/LED%20sheet.png)
![MCU Schematics](IMAGES/MainSheet.png)

---

### 2. Shift Register Cascade & High-Side Layer Drivers (U1 – U9 & Q1 – Q8)
Instead of using 72 separate GPIO pins to control the 64 columns and 8 layers individually, the driver architecture uses **9 daisy-chained 74HC595 8-bit serial-in, parallel-out shift registers (U1–U9)** driven over a single high-speed SPI bus from the ESP32. Serial data enters the first shift register (U1) on Pin 14 (SER via SPI_MOSI) and cascades out of Pin 9 (QH) directly into Pin 14 (SER) of the next chip in line (DOUT_1 through DOUT_8), effectively turning all 9 chips into a continuous 72-bit shift register. The first 8 shift registers (U1 to U8) act as column sink drivers for the 64 vertical cathode columns (COL_00 to COL_77), where every parallel output pin (QA–QH) routes through a **220Ω current-limiting resistor** (R1–R64) to safely sink current when pulled LOW.

Because a fully illuminated horizontal layer of 64 LEDs can draw nearly 1A of current—far exceeding what a logic IC pin can source directly—the 9th shift register (U9) is paired with 8 **TIP127 PNP Darlington transistors (Q1–Q8)** configured as high-side switches to control the anode layers (L0_A to L7_A). Each output pin of U9 connects through a **1kΩ base resistor** (R65–R72) to the Base (Pin 1) of its corresponding `TIP127` transistor, alongside a **10kΩ pull-up resistor** (R73–R80) tied to +5V to ensure the transistors snap cleanly off between layer scans without ghosting. When `U9` pulls a transistor base LOW, the TIP127 conducts +5V` from its Emitter (Pin 3) out through its Collector (Pin 2), energizing that specific horizontal LED layer. To maintain signal integrity across the board, each `74HC595` is paired with a local **`100nF` ceramic decoupling capacitor** (`C1`–`C9`) across `VCC` and `GND` to filter out high-frequency switching noise, while a **`10kΩ` pull-up resistor (`R81`)** on the shared `SPI_OE` line keeps all shift register outputs blanked during ESP32 startup.

![Shift Registers and Layer Drivers](IMAGES/Controllign%20IC.png)
![Shift Registers and Layer Drivers](IMAGES/Row.png)


---

### 3. Digital Audio Interface (`INMP441` I2S MEMS Microphone)
For crisp, noise-free audio reactivity without the power-supply hum common to analog microphone amplifiers, an **INMP441 omnidirectional I2S MEMS microphone module (`J1`)** is interfaced directly with the ESP32. The module is powered from the ESP32's regulated **`+3.3V`** rail and bypassed with a local **`100nF` decoupling capacitor (`C12`)** to keep LED switching noise out of the audio front-end, with its `L/R` channel select pin tied to `GND` for left-channel operation. As background music or voice plays in the room, the ESP32 continuously samples raw digital audio frames over the three I2S clock and data lines (`I2S_WS`, `I2S_SCK`, and `I2S_SD`) and performs real-time Fast Fourier Transform (FFT) spectrum analysis to translate bass drops, mid-range rhythms, and high-frequency treble notes into dynamic 3D animations.

![Microphone Schematic](IMAGES/mic.png)

---

### 4. Power Distribution (5V 5A SMPS & Bulk Filtering)
The entire LED cube and controller circuitry is powered by an external **5V 5A (25W) regulated DC switching power supply** connected to the board via a heavy-duty 2-pin screw terminal (J2). Because the cube operates on 1:8 time-division multiplexing—where only one horizontal layer of 64 LEDs is illuminated at any given microsecond before rapidly scanning to the next layer faster than the human eye can perceive—the continuous active current draw stays around 1.0A to 1.3A, giving the 5V 5A adapter more than three times the required power headroom. Right at the J2 power input terminal, two **1000µF 16V bulk electrolytic capacitors** (C10 and C11) are wired in parallel across the +5V and GND rails to act as a local charge reservoir, absorbing sudden current spikes during bright full-layer flashes and preventing voltage droop from resetting the microcontroller.

![Power Input Circuit](IMAGES/POWER.png)

---

### 5. ESP32-DEVKITC Control Unit (U10)
An **ESP32-DEVKITC (38-Pin)** development board (U10) serves as the central processing unit of the project, receiving +5V power from the main input rail while providing a clean +3.3V output for the digital microphone. Taking advantage of its dual-core 240 MHz architecture, the ESP32 pushes 72-bit frame buffers out to the 9-chip 74HC595 cascade over its hardware SPI lines (SPI_MOSI, SPI_CLK, SPI_LATCH, and active-low SPI_OE) at a high refresh rate so the 3D volumetric display appears completely flicker-free. Simultaneously, the second processing core handles incoming digital audio streams from the INMP441 microphone over the I2S bus (I2S_SD, I2S_SCK, and I2S_WS) as well as wireless Bluetooth audio synchronization, converting sound waves into fluid 3D lighting patterns in real time.

![ESP32 Schematic](Images/esp)

## How to Assemble
1 - Make the Led Matrix as per the Schematics , u have to make total 8 matrix each of the 64 led in the 8 x 8 and then use the brass 20SWG or Copper 20SWG wire to hold the layers.

2 - The use the Zero Dot pcb or the perfboard to solder all the necesary components.

3 - The use the support plate which is name as holding plate print the and put the one matrix on the plate it will act as the solid support which will directly glue with the main body to hold all the led adn also ther is thin hole for the copper hard wire to make the 3d matrix so pass the wire accoridngly.
![Support Plate Image](IMAGES/support.png)

4 - Then pass the wire inside the main boc which is the main unit of the Led Cube which will hold the all the led pass the brass wire into the hole and stick the supporting plate to the main body then solder the connection to the Zero Dot board  and then colde the main box from the bottom using glue.
![Main Body](IMAGES/main.png)
![Main Body](IMAGES/main2.png)
![Body Cover](IMAGES/Bottom.png)

## Firmware
This firmware made without prior testing as I didnt complete the hardware once the hardware will complete i will run the firmware and make the optimal changes to make it run flawlessly...

## BOM

## Bill of Materials (BOM)

| Part | Description | Qty | Unit Price | Total | Vendor | Buying Link |
| :--- | :--- | :---: | :---: | :---: | :--- | :--- |
| **LED (5mm Diffused)** | Main Component (Packs of 100) | 6 | $1.16 | $6.96 | MakerBazar | [Link](https://makerbazar.in/products/dip-led-5mm-oval-shape?variant=42130076696816) |
| **ESP32-WROOM-32** | Main Control Unit | 1 | $4.67 | $4.67 | Robu.in | [Link](https://robu.in/product/esp-wroom-32-esp32-wifi-bt-ble-mcu-module/) |
| **74HC595** | 8-Bit Shift Register IC | 9 | $0.19 | $1.71 | Hubtronics | [Link](https://hubtronics.in/1181) |
| **TIP127** | PNP Darlington Transistor (TO-220) | 8 | $0.01 | $0.08 | Quartz Components | [Link](https://quartzcomponents.com/products/tip127-darlington-pnp-transistor-to-220) |
| **INMP441 MEMS** | I2S Digital Microphone Module | 1 | $1.59 | $1.59 | Robu.in | [Link](https://robu.in/product/inmp441-mems-high-precision-omnidirectional-microphone-module-i2s/) |
| **5V 5A Adapter** | 25W SMPS Power Supply | 1 | $4.55 | $4.55 | Quartz Components | [Link](https://quartzcomponents.com/products/5v-5a-power-supply-charger-25w-smps-adapter) |
| **Dot / Vero Board** | Prototyping & Soldering Board | 1 | $0.42 | $0.42 | MakerBazar | [Link](https://makerbazar.in/products/general-purpose-solderable-vero-board?variant=48251196834032) |
| **220Ω Resistor** | Column Current Limiting Resistors | 64 | $0.01 | $0.64 | Robu.in | [Link](https://robu.in/product/1-month-warranty-1209/) |
| **1kΩ Resistor** | Transistor Base Resistors | 8 | $0.01 | $0.08 | Robu.in | [Link](https://robu.in/product/1k-ohm-1w-metal-film-resistor-pack-of-40/) |
| **Screw Terminal** | 2-Pin 5.08mm Power Input (`J2`) | 1 | $0.11 | $0.11 | Robu.in | [Link](https://robu.in/product/kf301-2-pin-5-08mm-pitch-plug-in-screw-terminal-block-connector-pack-of-5/) |
| **1000µF 16V Cap** | Bulk Power Filtering Capacitors | 2 | $0.01 | $0.02 | Robu.in | [Link](https://robu.in/product/hy1c102m100170cd288-hyncdz-1000uf-16v-65m%cf%89100khz-%c2%b120-1-35a100khz-plugind10xl17mm-aluminum-electrolytic-capacitors-leaded-rohs/) |
| **100nF Capacitor** | IC Decoupling Ceramic Capacitors | 10 | $0.01 | $0.10 | Robu.in | [Link](https://robu.in/product/100nf-50v-disc-capacitor/) |
| **10kΩ Resistor** | `SPI_OE` Pull-Up Resistor | 1 | $0.01 | $0.01 | Robu.in | [Link](https://robu.in/product/10k-ohm-0-25w-metal-film-resistor-pack-of-100/) |
| **3D Printed Case** | Base Enclosure for Electronics | 1 | $10.00 | $10.00 | Printing Legion | [Link](https://printlegion.hackclub.com/) |
| **Total** | | **124** | | **$30.94** | | |

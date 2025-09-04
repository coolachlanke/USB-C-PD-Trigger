# USB-C PD Trigger Board

---

## Overview
A custom 4-layer PCB for evaluating and triggering USB-C Power Delivery (PD) voltage profiles.  
The board combines an STM32F401RBT6 microcontroller with a Cypress CYPD3177 PD sink controller, enabling flexible control and monitoring of USB-C PD outputs.

---

## Features

- **USB-C Input**
  - Receptacle with differential pair routing for DFU support
  - CYPD3177 PD sink controller with I2C interface to MCU
  - Default PDO profile via resistor divider

- **Microcontroller**
  - STM32F401RBT6
  - SWD header for programming/debugging
  - USB DFU capability
  - I2C communication with PD sink

- **User Interface**
  - Single pushbutton to cycle through PDOs
  - 5x yellow indicator LEDs for 5 V, 9 V, 12 V, 15 V, 20 V

- **Connectivity**
  - VOUT terminal block for load connection
  - UART header (TX/RX) for serial monitoring

- **Debug/Monitoring**
  - Testpoints for VIN, VOUT, and GND

---

## Engineering Notes

- **Stackup:** 4 layers  
  - L1: Signal  
  - L2: Solid GND plane  
  - L3: Power plane  
  - L4: Signal

- **Differential Pair Routing (D+/D−)**
  - ~35 mm trace length  
  - ~1.3 mm mismatch (acceptable for USB Full Speed)  
  - Routed mainly on bottom layer with coplanar GND pour

- **Impedance Control**
  - Target: 90 Ω differential  
  - Widths tuned (~0.286 mm) using JLCPCB calculator

- **Grounding**
  - Full-plane GND layers with stitching vias for shielding  
  - Top and bottom copper filled with GND pours

- **Design Rule Check**
  - Fully clean, with only minor silkscreen overlap warnings (ignored for fabrication)

---

## Manufacturing

- Gerbers, BOM, and CPL generated using the JLCPCB KiCad plugin  
- Board ready for fabrication via JLCPCB  

---

## Future Improvements
- Add UART command interface for PDO selection  
- Expand to higher-speed USB signaling support

---

## Render

![USB-C PD Trigger Board](./docs/render.png)

# Full-Duplex InfraRed(IR) Communication
**Course:** CSE3442- Embedded I, UT Arlington
**Platform:** TI Tiva C TM4C123GXL LaunchPad (ARM Cortex-M4F)
**Language:** C (Direct Register Manipulation)

## Project Overview
This project establishes a robust, full-duplex wireless optical communication link between two TM4C microcontroller using IR transmission. The system bridges a high-speed PC terminal interface with a low-speed optical link, handling asynchronous data transfer, custom packet parsing, and real-time error detection via a completely interrupt-driven firmware architecture.

In addition to embedded firmware design, this project required hardware-in-the-loop co-design, including soldering a custom analog interface circuit to handle carrier wave modulation.
88
## System Architecture & Hardware Design

### 1. Optical Modulation Circuit
TSOP IR receiver requires an incoming signal modulated on 38kHz carrier wave to isolate it from ambient environmental noise.
* **Carrier Generation:** Configured the M0PWM0 module on pin PB6 to output a continuous 38kHz square wave.
* **Transistor Logic Gating:** Implemented a hardware gating circuit utilizing BJT transistors. the baseband 'UART7_TX' signal (PE1) gates the 38kHz carrier wave, driving the IR LED emitter only when data bits are actively transmitting.

### 2. Microcontroller Pin Mapping
* **PA0/ PA1:** 'UART0' Rx/Tx -> Connected to PC Terminal (115,200 Baud, 8N1)
* **PE0/ PE1:** 'UART7' Rx/Tx -> Connected to IR Receiver/ Transistor Driver Circuit (300 Baud, 8E1)
* **PB6:** 'M0PWM0' -> 38kHz Carrier Wave Generator
* **PF1:** ON-board RED LED -> Serial Parity Error Indicator (One-shot driven)
* **PF2:** ON-board BLUE LED -> ISR Entry/ Exit execution indicator

## Firmware Implementation Details

### Real-Time Constraints & Interrupt Priorities
A major engineering challenge was bridging the timing mismatch between PC/ optical terminals:
* **UART0 (PC):** 115,200 baud translate to ~87E-6s per char transmission.
* **UART7 (IR):** 300 baud translate to ~33.3E-3s per char transmission.

To prevent the CPU from freezing or dropping incoming data packets during full-duplex transactions, the entire application was decoupled into a prioritized background/foreground thread model:

1. **UART0 Rx Interrupt (Priority 2):** Fastest interrupt response to instantly capture high-speed terminal input and prevent FIFO overruns.
2. **UART7 Rx/Tx Interrupt (Priority 3):** Handles optical data movement. Features automated hardware FIFO handling. It also use Bit-banded memory mapping to flash the LED, allowing runtime validation via oscilloscope.

3. **Timer0A Interrupt (Priority 5):** Configured as a low-priority one-shot timer. If the hardware detects a *Parity Error* on the optical link, the UART7 ISR catches the fault, turn Red LED on, and loads TIMER01 with 40Mhz clock cycle count to automatically extinguish the error indicator after 1.0 second.

### Memory Layout & Data Structures
Custom status flags and structures manage string processing cleanly across thread barriers without blocking dependencies:
```c
typedef struct USER_DATA {
    char buffer[BUFFER_MAX];
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char fieldType[MAX_FIELDS];
} USER_DATA;
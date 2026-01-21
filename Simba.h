#pragma once
#include <sbus.h>
#include <Adafruit_NeoPixel.h>
#include <MadgwickAHRS.h>
#include "config.h"


// Receiver
extern bfs::SbusRx sbus_rx;
extern bfs::SbusData data;



// Built-in LED
extern Adafruit_NeoPixel led;
extern bool builtInLedOn;
extern Color color;


// Timer flags
extern volatile bool PIDF; // PID flag
extern volatile bool LEDF; // LED flag
extern volatile bool ALTF; // Altimeter flag
extern volatile bool OUTF; // Output flag
extern volatile bool RXDF; // Receiver data in flag
extern volatile bool TXDF; // Receiver data out flag
extern volatile int Ticks;

// Filter
extern Madgwick filter;


void builtInLed( uint8_t brightness);

void setupTimer();
void setLedTickDivider(uint32_t divider);


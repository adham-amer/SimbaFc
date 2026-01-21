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


// Timer flags
extern volatile bool PIDF;
extern volatile bool LEDF;
extern volatile int Ticks;

// Filter
extern Madgwick filter;


void builtInLed(uint8_t red,uint8_t green,uint8_t blue,uint8_t brightness);

void setupTimer();





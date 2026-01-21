#include <sbus.h>
#include <Wire.h>
#include "I2Cdev.h"
#include <Adafruit_NeoPixel.h>
#include <MadgwickAHRS.h>
#include "imu.h"


//Reciver SBUS out
#define SBUS_RX_PIN 10
#define SBUS_TX_PIN 11 


//built-in led
#define LED_PIN 48
#define NUM_LEDS 1


//Reciver
extern bfs::SbusRx sbus_rx;
extern bfs::SbusData data;



//BUILT-IN LED
extern Adafruit_NeoPixel led;
extern bool builtInLedOn;


//Timers Flags
extern volatile bool PIDF;
extern volatile bool LEDF;
extern volatile int Ticks;

//IMU
extern int16_t rax, ray, raz;
extern int16_t rgx, rgy, rgz;
extern float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
extern void InitIMU();
extern void IMURotation();
extern void IMUCalibrate();


//filter
extern Madgwick filter;

//PID


void builtInLed(uint8_t red,uint8_t green,uint8_t blue,uint8_t brightness);

void setupTimer();





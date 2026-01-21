#include "imu.h"
#include "config.h"
#include <SPI.h>

// ====== SPI CONFIG ======
static constexpr uint8_t  SPI_READ = 0x80;

// Use FSPI on ESP32-S3 Arduino core   
//test
static SPIClass imuSpi(FSPI);

// ====== BMI160 REGISTERS / COMMANDS ======
static constexpr uint8_t REG_CHIP_ID   = 0x00; // reads 0xD1
static constexpr uint8_t REG_ACC_CONF  = 0x40;
static constexpr uint8_t REG_ACC_RANGE = 0x41;
static constexpr uint8_t REG_GYR_CONF  = 0x42;
static constexpr uint8_t REG_GYR_RANGE = 0x43;

static constexpr uint8_t REG_CMD       = 0x7E;

// Commands (BMI160)
static constexpr uint8_t CMD_SOFTRESET  = 0xB6;
static constexpr uint8_t CMD_ACC_NORMAL = 0x11;
static constexpr uint8_t CMD_GYR_NORMAL = 0x15;

// Data registers:
// NOTE: On BMI160, accel/gyro data are contiguous starting at DATA_0 (0x12).
// 0x12..0x17 accel XYZ (LSB/MSB), 0x0C..0x11 gyro XYZ on some Bosch parts,
// but BMI160 layout commonly:
//   0x0C..0x11 = GYR_X/Y/Z_LSB/MSB
//   0x12..0x17 = ACC_X/Y/Z_LSB/MSB
// If your datasheet table shows different, change these.
static constexpr uint8_t REG_GYR_DATA_START = 0x0C; // gx..gz 6 bytes

// ====== LOW-LEVEL HELPERS ======
static inline void csLow()  { digitalWrite(kImuCsPin, LOW); }
static inline void csHigh() { digitalWrite(kImuCsPin, HIGH); }

static uint8_t rd8(uint8_t reg)
{
  imuSpi.beginTransaction(SPISettings(kImuSpiHz, MSBFIRST, kImuSpiMode));
  csLow();
  imuSpi.transfer(reg | SPI_READ);
  uint8_t v = imuSpi.transfer(0x00);
  csHigh();
  imuSpi.endTransaction();
  return v;
}

static void wr8(uint8_t reg, uint8_t val)
{
  imuSpi.beginTransaction(SPISettings(kImuSpiHz, MSBFIRST, kImuSpiMode));
  csLow();
  imuSpi.transfer(reg & 0x7F);
  imuSpi.transfer(val);
  csHigh();
  imuSpi.endTransaction();
}

static void burstRead(uint8_t startReg, uint8_t* buf, size_t n)
{
  imuSpi.beginTransaction(SPISettings(kImuSpiHz, MSBFIRST, kImuSpiMode));
  csLow();
  imuSpi.transfer(startReg | SPI_READ);
  for (size_t i = 0; i < n; i++) buf[i] = imuSpi.transfer(0x00);
  csHigh();
  imuSpi.endTransaction();
}

static inline int16_t le16(const uint8_t* p)
{
  return (int16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

// ====== PUBLIC API ======
bool imu_init()
{
  pinMode(kImuCsPin, OUTPUT);
  csHigh();

  // Start SPI on custom pins
  imuSpi.begin(kImuSckPin, kImuMisoPin, kImuMosiPin, kImuCsPin);

  // Verify CHIP_ID
  const uint8_t id = rd8(REG_CHIP_ID);
  if (id != 0xD1) return false;

  // Soft reset
  delay(50);
  wr8(REG_CMD, CMD_SOFTRESET);
  delay(50); // NVM reload/boot time (datasheet typical)
  rd8(REG_CHIP_ID);
  delay(50);
  // Power accel + gyro to normal mode
  wr8(REG_CMD, CMD_ACC_NORMAL);
  delay(50); // accel start-up time (datasheet typical)
  rd8(REG_CHIP_ID);
  delay(50);
  wr8(REG_CMD, CMD_GYR_NORMAL);
  delay(80); // gyro start-up time (datasheet typical)
  rd8(REG_CHIP_ID);
  delay(50);
  // Configure for high-rate polling:
  // You want 1 kHz host loop. Set ODR >= 1 kHz so you don't read repeats.
  // Common choice: 1600 Hz ODR for both accel & gyro with reasonable bandwidth.
  //
  // ACC_CONF/GYR_CONF bitfields are sensor-specific; set them per BMI160 tables.
  // Below are placeholders you MUST tune:
  //
  // Example intent (not guaranteed correct bitfields without your exact table):
  // - accel ODR = 1600 Hz
  // - gyro  ODR = 1600 Hz
  // - bandwidth/filters: moderate (flight control)
  //
  // BMI160 ACC_CONF/GYR_CONF:
  // - ODR = 1600 Hz (0x0C)
  // - BW  = normal mode (0x02 in BW field)
  wr8(REG_ACC_CONF,  0x2C);
  delay(10); // config update time (datasheet typical)
  wr8(REG_GYR_CONF,  0x2C);
  delay(10); // config update time (datasheet typical)

  // Ranges (leave as-is unless you want different sensitivity)
  wr8(REG_ACC_RANGE, 0x03); // +/-2g
  delay(10); // config update time (datasheet typical)
  wr8(REG_GYR_RANGE, 0x00); // +/-2000 dps
  delay(10); // config update time (datasheet typical)

  return true;
}

bool imu_read(ImuSample& out)
{
  // Burst gyro+accel in one transaction: 12 bytes from 0x0C..0x17
  uint8_t buf[12];
  burstRead(REG_GYR_DATA_START, buf, sizeof(buf));

  out.gx = le16(&buf[0]);
  out.gy = le16(&buf[2]);
  out.gz = le16(&buf[4]);

  out.ax = le16(&buf[6]);
  out.ay = le16(&buf[8]);
  out.az = le16(&buf[10]);

  return true;
}

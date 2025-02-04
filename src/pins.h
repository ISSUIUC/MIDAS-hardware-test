#pragma once

// SPI sensor bus
#define SPI_MISO 39
#define SPI_MOSI 37
#define SPI_SCK 38

// barometer chip select
#define MS5611_CS 43

// gyro chip select
#define LSM6DS3_CS 36

// high g chip select
#define KX134_CS 40

// low g chip select
#define ADXL355_CS 44

// magnetometer chip select
#define LIS3MDL_CS 35

// orientation chip select, interrupt
#define BNO086_CS 48
#define BNO086_INT 34
#define BNO086_RESET 33

// i2c bus pins
#define I2C_SDA 10
#define I2C_SCL 9

// buzzer pin
#define BUZZER_PIN 17
#define BUZZER_CHANNEL 1

// CAN pins
#define CAN_CS 2
#define RFM96W_CS 5

// GPS I2C location
#define GNSS_I2C_LOCATION 0x3A
#define GPS_RESET GpioAddress(2, 017)
#define GPS_ENABLE 0

#define FLASH_DAT0 13
#define FLASH_DAT1 12
#define FLASH_DAT2 15
#define FLASH_DAT3 16
#define FLASH_CLK 18
#define FLASH_CMD 14

#define E22_CS 5
#define E22_DI01 4
#define E22_DI03 3
#define E22_BUSY 6
#define E22_RXEN 7
#define E22_RESET 8

#define CAN_1 2
#define CAN_2 1

#define INA745_ADDR 0x44

#define PYRO_SDA 41
#define PYRO_SCL 42
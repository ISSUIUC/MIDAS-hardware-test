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
#define BNO086_

// i2c bus pins
#define I2C_SDA 10
#define I2C_SCL 9

// CAN pins
#define CAN_CS 2
#define RFM96W_CS 5

// GPS I2C location
#define GNSS_I2C_LOCATION 0x3A
#define GPS_RESET GpioAddress(2, 017)
#define GPS_ENABLE 0
        
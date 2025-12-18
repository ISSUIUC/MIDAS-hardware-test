#pragma once

// SPI sensor bus
#define SPI_MISO 3
#define SPI_MOSI 2
#define SPI_SCK 1

// barometer chip select
#define MS5611_CS 44

// imu chip select
#define LSM6DSV320X_CS 43
#define LSM_INT1 15
#define LSM_INT2 42 

// magnetometer chip select
#define MMC5983_CS 4

// i2c bus pins
#define I2C_SDA 21
#define I2C_SCL 26

// buzzer pin
#define BUZZER_PIN 12
#define BUZZER_CHANNEL 1

// GPS I2C location
#define GNSS_I2C_LOCATION 0x3A //??
#define GPS_RESET 5
#define GPS_ENABLE 0

// Flash
#define FLASH_DAT0 33
#define FLASH_DAT1 34
#define FLASH_DAT2 18
#define FLASH_DAT3 17
#define FLASH_CLK 16
#define FLASH_CMD 47

// LoRa
#define E22_CS 37
#define E22_DI01 41
#define E22_DI03 40
#define E22_BUSY 38
#define E22_RXEN 39
#define E22_RESET 6

// LEDs
#define LED_RED 11
#define LED_ORANGE 10
#define LED_GREEN 9
#define LED_BLUE 8
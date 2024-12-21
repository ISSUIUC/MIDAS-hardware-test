/* basic testing script for midas board bringup. tests spi sensors as well as emmc chip */

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <FS.h>
#include <SD_MMC.h>
#include <MicroNMEA.h>
#include <RH_RF95.h>
#include <SD_MMC.h>
// #include <CANBus-SOLDERED.h>

#include "pins.h"
#include "bno_functions.h"
#include "emmc_functions.h"
#include "TCAL9539.h"
#include "teseo_liv3f_class.h"
#include "ads7138-q1.h"

#include <MS5611.h>
#include <SparkFun_Qwiic_KX13X.h>
#include <PL_ADXL355.h>
#include <Arduino_LSM6DS3.h>
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_BNO08x.h>
#include <SparkFun_u-blox_GNSS_v3.h>
#include <MicroNMEA.h> //http://librarymanager/All#MicroNMEA
#include <LoRaWan-Arduino.h>

#define WAIT_FOR_SERIAL
// #define MCU_TEST
// #define ENABLE_BAROMETER
// #define ENABLE_HIGHG
// #define ENABLE_LOWG
// #define ENABLE_LOWGLSM
// #define ENABLE_MAGNETOMETER
// #define ENABLE_ORIENTATION
// #define ENABLE_EMMC
// #define ENABLE_ADS
// #define ENABLE_GPIOEXP
// #define ENABLE_GPS
// #define ENABLE_LORA
// #define ENABLE_CAN
// #define ENABLE_FLASH
// #define ENABLE_INA
// #define ENABLE_CHRISTMAS

// Please be careful
// This will init the gpio expander by itself
#define PYRO_TEST

#ifdef ENABLE_LORA
	hw_config hwConfig;
#endif

#ifdef ENABLE_BAROMETER
	MS5611 MS(MS5611_CS);
#endif

#ifdef ENABLE_HIGHG
	QwiicKX134 KX;
#endif

#ifdef ENABLE_LOWG
	PL::ADXL355 sensor(ADXL355_CS);
#endif

#ifdef ENABLE_LOWGLSM
	LSM6DS3Class LSM(SPI, LSM6DS3_CS, 46);
#endif

#ifdef ENABLE_MAGNETOMETER
	Adafruit_LIS3MDL LIS3MDL;
#endif

#ifdef ENABLE_ORIENTATION
	Adafruit_BNO08x imu(BNO086_RESET);
#endif

#ifdef ENABLE_EMMC
	uint8_t buff[8192];
#endif


#ifdef ENABLE_GPS
SFE_UBLOX_GNSS myGNSS;
char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));
#endif

#ifdef ENABLE_INA

#endif
#ifdef ENABLE_CAN
CANBus CAN(CAN_CS); // Set CS pin
#endif
#define MAX_DATA_SIZE 64

#ifdef PYRO_TEST
	int CUR_PYRO = 0; // 0 --> off, 1-->A, 2-->B, 3-->C, 4-->D

#endif


#ifdef ENABLE_LORA

#define RF_FREQUENCY 430000000  // Hz
#define TX_OUTPUT_POWER 22		// dBm
#define LORA_BANDWIDTH 0		// [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR 8 // [SF7..SF12]
#define LORA_CODINGRATE 4		// [1: 4/5, 2: 4/6,  3: 4/7,  4: 4/8]
#define LORA_PREAMBLE_LENGTH 8  // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0   // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 3000
#define TX_TIMEOUT_VALUE 3000
#define LORA_BUFFER_SIZE 64 // Define the payload size here

void OnTxDone(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnTxTimeout(void);
void OnRxTimeout(void);
void OnRxError(void);
void OnCadDone(bool cadResult);

static RadioEvents_t RadioEvents;
static uint16_t BufferSize = LORA_BUFFER_SIZE;
static uint8_t RcvBuffer[LORA_BUFFER_SIZE];
static uint8_t TxdBuffer[LORA_BUFFER_SIZE];
static bool isMaster = true;
const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

time_t timeToSend;
time_t cadTime;
uint8_t pingCnt = 0;
uint8_t pongCnt = 0;

// Lora callbacks
void OnTxDone(void)
{
	Serial.println("LoRa Callback - OnTxDone");
	Radio.Rx(RX_TIMEOUT_VALUE);
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
	Serial.println("LoRa Callback - OnRxDone");
	delay(10);
}

void OnTxTimeout(void)
{
	Serial.println("LoRa Callback - OnTxTimeout");
	Radio.Rx(RX_TIMEOUT_VALUE);
}

void OnRxTimeout(void)
{
	Serial.println("LoRa Callback - OnRxTimeout");
	Radio.Rx(RX_TIMEOUT_VALUE);
}

void OnRxError(void)
{
	Serial.println("RX ERR!");
}

void OnCadDone(bool cadResult)
{
	Serial.println("fr i dont know what this does");
}
#endif

#ifdef ENABLE_CHRISTMAS
// I wanted to be cute
#include <buzzer.h>

bool cur_light_state = false;

#endif

void setup() {
	Serial.begin(9600);


	#ifdef WAIT_FOR_SERIAL
		while(!Serial);
		Serial.println("Serial ready");
	#endif


	delay(1000);

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
	// Wire.begin(PYRO_SDA, PYRO_SCL);

	Wire.begin(I2C_SDA, I2C_SCL);
	Serial.println("Initialized SPI");

	// Serial.println("beginning sensor test");
	
	// pinMode(SPI_MOSI, OUTPUT);
	// while(1) {
	// 	delay(1000);
	// 	digitalWrite(SPI_MOSI, LOW);
	// 	delay(1000);
	// 	digitalWrite(SPI_MOSI, HIGH);
	// }
	pinMode(LSM6DS3_CS, OUTPUT);
	pinMode(KX134_CS, OUTPUT);
	pinMode(ADXL355_CS, OUTPUT);
	pinMode(LIS3MDL_CS, OUTPUT);
	pinMode(BNO086_CS, OUTPUT);
	pinMode(BNO086_RESET, OUTPUT);
	pinMode(CAN_CS, OUTPUT);
	pinMode(RFM96W_CS, OUTPUT);
	// pinMode(21, OUTPUT);
	// pinMode(47, OUTPUT);
// pinMode(41, OUTPUT);
// 	pinMode(42, OUTPUT);
// 	digitalWrite(41, LOW);
// 	digitalWrite(42, LOW);
// digitalWrite(21, HIGH);
// digitalWrite(47, LOW);
	digitalWrite(MS5611_CS, HIGH);
	digitalWrite(LSM6DS3_CS, HIGH);
	digitalWrite(KX134_CS, HIGH);
	digitalWrite(ADXL355_CS, HIGH);
	digitalWrite(LIS3MDL_CS, HIGH);
	digitalWrite(BNO086_CS, HIGH);
	digitalWrite(CAN_CS, HIGH);
	digitalWrite(RFM96W_CS, HIGH);

	digitalWrite(BNO086_RESET, LOW);

#ifdef ENABLE_CHRISTMAS
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
#endif

#ifdef ENABLE_FLASH
	Serial.println("Connecting to SD...");
    if (!SD_MMC.setPins(FLASH_CLK, FLASH_CMD, FLASH_DAT0)) {
        while (1) { Serial.println("No flash!"); }
    }
	Serial.println("It's okay");
    if (!SD_MMC.begin("/sd", true, false, SDMMC_FREQ_52M, 5)) {
        while (1) { Serial.println("Weird error!"); }
    }
	Serial.println("It's really okay");
	Serial.println(SD_MMC.totalBytes());
	Serial.println(SD_MMC.usedBytes());
	Serial.println(SD_MMC.cardType());
	auto file = SD_MMC.open("/test", FILE_READ, true);
	if (!file) { 
		Serial.println("Failed to open file");
	}
	Serial.println(SD_MMC.totalBytes());
	char t[256];
	file.read((uint8_t*) t, strlen("Hello world"));
	Serial.println(t);
#endif
#ifdef ENABLE_CAN
    // Serial.println("Starting I2C...");
	CAN.setSPI(&SPI);
	while (0 != CAN.begin(CAN_125K_500K))// Initialize CAN BUS with baud rate of 125 kbps and arbitration rate of 500k
		// This should be in while loop because MCP2518
		// needs some time to initialize and start function
		// properly.
	{
		Serial.println("CAN init fail, retry..."); // Print information message
		delay(100);
	}
	Serial.println("CAN init ok!");

	for (int i = 0; i < MAX_DATA_SIZE; i++) // Fill buffer with ascending numbers 
	{
		stmp[i] = i;
	}
#endif

#ifdef ENABLE_LORA

	Serial.println("Initializing LoRa");

	hwConfig.CHIP_TYPE = SX1262_CHIP;		  // Example uses an eByte E22 module with an SX1262
	hwConfig.PIN_LORA_RESET = E22_RESET; // LORA RESET
	hwConfig.PIN_LORA_NSS = E22_CS;	  // LORA SPI CS
	hwConfig.PIN_LORA_SCLK = SPI_SCK;	  // LORA SPI CLK
	hwConfig.PIN_LORA_MISO = SPI_MISO;	  // LORA SPI MISO
	hwConfig.PIN_LORA_DIO_1 = E22_DI01; // LORA DIO_1
	hwConfig.PIN_LORA_BUSY = E22_BUSY;	  // LORA SPI BUSY
	hwConfig.PIN_LORA_MOSI = SPI_MOSI;	  // LORA SPI MOSI
	hwConfig.RADIO_RXEN = E22_RXEN;		  // LORA ANTENNA RX ENABLE
	hwConfig.USE_DIO2_ANT_SWITCH = true;	  // Example uses an CircuitRocks Alora RFM1262 which uses DIO2 pins as antenna control
	hwConfig.USE_DIO3_TCXO = false;			  // Example uses an CircuitRocks Alora RFM1262 which uses DIO3 to control oscillator voltage

	Serial.println("LoRa config set");

	uint32_t err_code = lora_hardware_init(hwConfig);
	if (err_code != 0)
	{
		Serial.printf("lora_hardware_init failed - %d\n", err_code);
		while(1) {};
	}
	Serial.println("Lora hardware init successful");
	
	RadioEvents.TxDone = OnTxDone;
	RadioEvents.RxDone = OnRxDone;
	RadioEvents.TxTimeout = OnTxTimeout;
	RadioEvents.RxTimeout = OnRxTimeout;
	RadioEvents.RxError = OnRxError;
	RadioEvents.CadDone = OnCadDone;
	Serial.println("Lora callbacks set");

	// Initialize the Radio
	Radio.Init(&RadioEvents);

	// Set Radio channel
	Radio.SetChannel(RF_FREQUENCY);
	Serial.println("Lora radio channel init successful");

	// Set Radio TX configuration
	Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
					  LORA_SPREADING_FACTOR, LORA_CODINGRATE,
					  LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
					  true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);

	// Set Radio RX configuration
	Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
					  LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
					  LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
					  0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

	Serial.println("Lora radio rx/tx config successful");

	Serial.println("Starting Radio.Rx");
	Radio.Rx(RX_TIMEOUT_VALUE);
	timeToSend = millis();

	Serial.println("LoRa Init Successful");
	


#endif
	#ifdef ENABLE_BAROMETER
		MS.init();
		Serial.println("barometer init successfully");
	#endif

	#ifdef ENABLE_HIGHG
		KX.beginSPI(KX134_CS, 100000);
		if (!KX.initialize(DEFAULT_SETTINGS)) {
			Serial.println("could not init highg");
			while(1);
		}
		if(!KX.setOutputDataRate(0xb)) {
			Serial.println("could not update data rate of highg");
			while(1);
		}
		KX.setRange(3);
		Serial.println("highg init successfully");
	#endif

	#ifdef ENABLE_LOWG
		Serial.println("Before sensor.start");
		sensor.begin();
		Serial.println("Initializing lowg");
		sensor.enableMeasurement();

		// if (sensor.isDeviceRecognized()){
		// 	Serial.println("Device is recognized");
		// 	sensor.initializeSensor(Adxl355::RANGE_VALUES::RANGE_2G, Adxl355::ODR_LPF::ODR_1000_AND_250);
		// 	Serial.println("Sensor is initialized");
		// 	if (Adxl355::RANGE_VALUES::RANGE_2G != sensor.getRange()){
		// 		Serial.println("could not set range lowg");
		// 		while(1);
		// 	}

		// 	if (Adxl355::ODR_LPF::ODR_4000_AND_1000 != sensor.getOdrLpf()){
		// 		Serial.println("could not set odrlpf lowg");
		// 		while(1);
		// 	}
		// }
		// else{
		// 	Serial.println("could not init lowg");
		// 	while(1);
		// }
   		// sensor.calibrateSensor(1);
		Serial.println("lowg init successfully");
	#endif

	#ifdef ENABLE_LOWGLSM
		if (!LSM.begin()) {
			Serial.println("could not init lowglsm");
			while(1);
		}
		Serial.println("lowglsm init successfully");
	#endif

	#ifdef ENABLE_MAGNETOMETER
		if (!LIS3MDL.begin_SPI(LIS3MDL_CS)){
			Serial.println("could not init magnetometer");
			while(1);
		}
		LIS3MDL.setOperationMode(LIS3MDL_CONTINUOUSMODE);
		LIS3MDL.setDataRate(LIS3MDL_DATARATE_5_HZ);
		LIS3MDL.setRange(LIS3MDL_RANGE_4_GAUSS);
		Serial.println("magnetometer init successfully");
	#endif

	#ifdef ENABLE_EMMC
		if(!SD_MMC.setPins(EMMC_CLK, EMMC_CMD, EMMC_D0)){
			Serial.println("Pin change failed!");
			return;
		}
		// if(!SD_MMC.begin()){

		if(!SD_MMC.begin("/sdcard", true, true, SDMMC_FREQ_52M, 5)){
			Serial.println("Card Mount Failed");
			;
		}
		uint8_t cardType = SD_MMC.cardType();

		if(cardType == CARD_NONE){
			Serial.println("No SD_MMC card attached");
		}

		Serial.print("SD_MMC Card Type: ");
		if(cardType == CARD_MMC){
			Serial.println("MMC");
		} else if(cardType == CARD_SD){
			Serial.println("SDSC");
		} else if(cardType == CARD_SDHC){
			Serial.println("SDHC");
		} else {
			Serial.println("UNKNOWN");
		}

		uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
		Serial.print("SD_MMC Card Size: ");
		Serial.print(cardSize);
		Serial.println("MB");
		File f = SD_MMC.open("/midas.txt", FILE_WRITE, true);
		auto m1 = micros();
		std::fill(buff, buff + 8192, 'q');
		for(int i = 0; i < 10; i++){
			f.write(buff, 8192);
		}
		Serial.println(micros() - m1);
		f.close();
		// listDir(SD_MMC, "/", 0);
		// createDir(SD_MMC, "/mydir");
		// listDir(SD_MMC, "/", 0);
		// removeDir(SD_MMC, "/mydir");
		// listDir(SD_MMC, "/", 2);
		// writeFile(SD_MMC, "/hello.txt", "Hello ");
		// appendFile(SD_MMC, "/hello.txt", "World!\n");
		// readFile(SD_MMC, "/hello.txt");
		// deleteFile(SD_MMC, "/foo.txt");
		// renameFile(SD_MMC, "/hello.txt", "/foo.txt");
		// readFile(SD_MMC, "/foo.txt");
		// testFileIO(SD_MMC, "/test.txt");
		Serial.printf("Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024));
		Serial.printf("Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));
	#endif

	#ifdef ENABLE_ADS
		if (!ADS7138Init()) {
        	Serial.println("could not init ads");
    	} else {
			Serial.println("ads init successfully");
		}
	#endif

	#ifdef ENABLE_GPIOEXP
		/*constexpr uint8_t GPIO0_ADDRESS = 0x74;
		constexpr uint8_t GPIO1_ADDRESS = 0x75;
		constexpr uint8_t GPIO2_ADDRESS = 0x77;
		constexpr uint8_t REG_OUTPUT0 = 0x2;
		constexpr uint8_t REG_OUTPUT1 = 0x3;
		uint8_t addrs[] = {GPIO0_ADDRESS, GPIO1_ADDRESS, GPIO2_ADDRESS};
		for(uint8_t addr : addrs){
			Wire.beginTransmission(GPIO0_ADDRESS);
			Wire.write(REG_OUTPUT0);
			if(!Wire.endTransmission()){
				return false;
			}
			int ct = Wire.requestFrom(GPIO0_ADDRESS, 1);
			if(ct != 1){
				return false;
			}
			int v = Wire.read();
			//REG_OUTPUT0 is set all ones on power up
			if(v != 0xff){
				return false;
			}
		}
		return true;*/

		if (!TCAL9539Init()) {
			Serial.println("Failed to initialize TCAL9539!");
			// while(1){ };
		}

		Serial.println("TCAL9539 initialized successfully!");

		for (int i = 0; i <= 017; i++) {
			gpioPinMode(GpioAddress(2, i), OUTPUT);
			gpioDigitalWrite(GpioAddress(2, i), LOW);
		}
		gpioPinMode(GpioAddress(1, 04), INPUT);
		Serial.println(gpioDigitalRead(GpioAddress(1, 04)).value);

	#endif

	#ifdef PYRO_TEST

		pinMode(BUZZER_PIN, OUTPUT);
		digitalWrite(BUZZER_PIN, LOW);
		ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);

		if (!TCAL9539Init()) {
			Serial.println("Failed to initialize TCAL9539!");
			// while(1){ };
		}

		Serial.println("TCAL9539 initialized successfully!");

		for (int i = 0; i <= 017; i++) {
			gpioPinMode(GpioAddress(2, i), OUTPUT);
			gpioDigitalWrite(GpioAddress(2, i), LOW);
		}

		for (int i = 0; i <= 017; i++) {
			gpioPinMode(GpioAddress(0, i), OUTPUT);
			gpioDigitalWrite(GpioAddress(0, i), LOW);
		}

	#endif

	#ifdef ENABLE_GPS
	if (myGNSS.begin() == false)
	{
		Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
		while (1);
	}

	myGNSS.setI2COutput(COM_TYPE_UBX | COM_TYPE_NMEA); //Set the I2C port to output both NMEA and UBX messages
	myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR

	//This will pipe all NMEA sentences to the serial port so we can see them
	myGNSS.setNMEAOutputPort(Serial);
	#endif
	// gpioPinMode(GpioAddress(1, 01), OUTPUT);

	// gpioDigitalWrite(GpioAddress(1, 01), HIGH); // Set the bno pin mode to 01

		
	#ifdef ENABLE_ORIENTATION

		/*if (!TCAL9539Init()) {
			Serial.println("Failed to initialize TCAL9539!");
			// while(1){ };
		}

		Serial.println("TCAL9539 initialized successfully!");*/
		Serial.println("Delaying");
		delay(1000);
		Serial.println("Delayed done!");
		if (!imu.begin_SPI(BNO086_CS, BNO086_INT)) {
			Serial.println("could not init orientation");
			while(1) {Serial.println("could not init orientation");}
		}
		Serial.println("BNO inited SPI");
		if (!imu.enableReport(SH2_ARVR_STABILIZED_RV, 5000)) {
			Serial.println("Could not enable stabilized remote vector");
			while(1) {Serial.println("Could not enable stabilized remote vector");}
		}
		Serial.println("orientation init successfully");
		
	#endif
	// Wire1.begin(PYRO_SDA, PYRO_SCL);
	#ifdef ENABLE_INA
	Wire.beginTransmission(0x44);
	Wire.write(0x3E);
	Wire.endTransmission();
	Wire.requestFrom(0x44, 2);
	Serial.println(Wire.read());
	Serial.println(Wire.read());

	// for (int i = 0; i < )
	byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
	// Serial.println("Scanning...");
    Wire.beginTransmission(address);
	// Serial.println("Scanning2...");
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
  delay(5000); 
	#endif
}

void loop() {

	#ifdef ENABLE_CAN
	auto err_code = CAN.sendMsgBuf(0x01, 0, CANFD::len2dlc(MAX_DATA_SIZE), stmp); // Send data in CAN network
	if (err_code != 0) {
		Serial.print("Failed: ");
		Serial.println(err_code);

	} else {
		Serial.println("CAN BUS sendMsgBuf ok!"); // Print message
	}
	// First parameter - which ID to set in frame (ID of transmitter)
	// Second parameter - Frame size (0 - Normal frame, 1 - Extended frame)
	// Third parameter - Length of buffer in bytes, but converted in Data Length Code
	// Fourth parameter - Buffer which contains data to send
	// delay(10); // Wait a bit for CAN module to send data
	// CAN.sendMsgBuf(0x04, 0, CANFD::len2dlc(MAX_DATA_SIZE), stmp); // Send data in CAN network
	// First parameter - which ID to set in frame (ID of transmitter)
	// Second parameter - Frame size (0 - Normal frame, 1 - Extended frame)
	// Third parameter - Length of buffer in bytes, but converted in Data Length Code
	// Fourth parameter - Buffer which contains data to send
	delay(1000); // Wait a bit not to overfill network
	#endif

	#ifdef MCU_TEST
		Serial.println("test");
	#endif

	#ifdef ENABLE_CHRISTMAS
		// just play the entire song
		// assume gpio is enabled this is for fun anyway

		Serial.println("Playing song!");

		for(unsigned i = 0; i < MERRY_CHRISTMAS_LENGTH; i++) {

			cur_light_state = !cur_light_state;

			gpioDigitalWrite(GpioAddress(2, 014), LOW);
			gpioDigitalWrite(GpioAddress(2, 016), LOW);
			gpioDigitalWrite(GpioAddress(2, 015), cur_light_state ? HIGH : LOW);
			gpioDigitalWrite(GpioAddress(2, 017), cur_light_state ? LOW : HIGH);

			Sound cur_sound = merry_christmas[i];
			ledcWriteTone(BUZZER_CHANNEL, cur_sound.frequency);
			delay(cur_sound.duration_ms);
		}

		Serial.println("Delaying 2s before playing again");
		delay(2000);
		

	#endif

	#ifdef ENABLE_GPIOEXP

		// gpioDigitalWrite(GpioAddress(2, 014), HIGH);
		gpioDigitalWrite(GpioAddress(2, 015), HIGH);
		delay(200);
		gpioDigitalWrite(GpioAddress(2, 015), LOW);
		gpioDigitalWrite(GpioAddress(2, 017), HIGH);
		delay(200);
		gpioDigitalWrite(GpioAddress(2, 017), LOW);
		// Serial.println("Looped high");
		// // gpioDigitalWrite(GpioAddress(2, 014), LOW);
		// gpioDigitalWrite(GpioAddress(2, 015), LOW);
		// gpioDigitalWrite(GpioAddress(2, 016), LOW);
		// gpioDigitalWrite(GpioAddress(2, 017), LOW);
		// Serial.println("Looped");
		// delay(500);
	#endif

	#ifdef PYRO_TEST

		// // force-disable all pyro
		// gpioDigitalWrite(GpioAddress(0, 06), LOW); // pyro disabled

		// gpioDigitalWrite(GpioAddress(0, 00), LOW);
		// gpioDigitalWrite(GpioAddress(0, 01), LOW);
		// gpioDigitalWrite(GpioAddress(0, 03), LOW);
		// gpioDigitalWrite(GpioAddress(0, 04), LOW);

		// // Red pin, no noise, 10s
		// gpioDigitalWrite(GpioAddress(2, 017), HIGH);
		// delay(5000);

		// // Red pin, noise, 5s

		// for(unsigned i = 0; i < 3; i++) {
			
		// 	ledcWriteTone(BUZZER_CHANNEL, 2000);
		// 	delay(100);
		// 	ledcWriteTone(BUZZER_CHANNEL, 0);
		// 	delay(900);	
			
		// }

		// // Red pin, fast noise, 2s

		// for(unsigned i = 0; i < 2*4; i++) {
		// 	ledcWriteTone(BUZZER_CHANNEL, 2000);
		// 	delay(150);
		// 	ledcWriteTone(BUZZER_CHANNEL, 0);
		// 	delay(100);
		// }

		// // fire pyro

		// gpioDigitalWrite(GpioAddress(0, 06), HIGH); // pyro enabled

		// gpioDigitalWrite(GpioAddress(0, 00), HIGH);
		// gpioDigitalWrite(GpioAddress(0, 01), HIGH);
		// gpioDigitalWrite(GpioAddress(0, 03), HIGH);
		// gpioDigitalWrite(GpioAddress(0, 04), HIGH);

		// delay(200);

		// gpioDigitalWrite(GpioAddress(0, 06), LOW); // pyro disabled

		// gpioDigitalWrite(GpioAddress(0, 00), LOW);
		// gpioDigitalWrite(GpioAddress(0, 01), LOW);
		// gpioDigitalWrite(GpioAddress(0, 03), LOW);
		// gpioDigitalWrite(GpioAddress(0, 04), LOW);

		// // no red pin for 5s
		// gpioDigitalWrite(GpioAddress(2, 017), LOW);
		// delay(5000);


		


		char* buf[255];

		gpioDigitalWrite(GpioAddress(0, 06), CUR_PYRO == 0 ? LOW : HIGH); // pyro enabled only if not 0
		gpioDigitalWrite(GpioAddress(0, 00), CUR_PYRO == 1 ? HIGH : LOW);
		gpioDigitalWrite(GpioAddress(0, 01), CUR_PYRO == 2 ? HIGH : LOW);
		gpioDigitalWrite(GpioAddress(0, 03), CUR_PYRO == 3 ? HIGH : LOW);
		gpioDigitalWrite(GpioAddress(0, 04), CUR_PYRO == 4 ? HIGH : LOW);

		gpioDigitalWrite(GpioAddress(2, 016), CUR_PYRO == 0 ? HIGH : LOW);

		size_t bytes_read = Serial.readBytesUntil('\n', (char*)&buf, 10);
		if(bytes_read > 0) {
			CUR_PYRO = (CUR_PYRO + 1) % 5;
			gpioDigitalWrite(GpioAddress(2, 015), HIGH);
			delay(50);
			gpioDigitalWrite(GpioAddress(2, 015), LOW);
			Serial.printf("Cur pyro: %d\n", CUR_PYRO);
		}



	#endif

	#ifdef ENABLE_BAROMETER
		MS.read(12);
		float pressure = static_cast<float>(MS.getPressure() * 0.01 + 26.03);
		float temperature = static_cast<float>(MS.getTemperature() * 0.01);
		float altitude = static_cast<float>(-log(pressure * 0.000987) * (temperature + 273.15) * 29.254);
		Serial.print("Pressure: ");
		Serial.print(pressure);
		Serial.print(" Temp: ");
		Serial.print(temperature);
		Serial.print(" Altitude: ");
		Serial.println(altitude);
	#endif

	#ifdef ENABLE_HIGHG
		auto data = KX.getAccelData();
		Serial.print("ax: ");
		Serial.print(data.xData);
		Serial.print(" ay: ");
		Serial.print(data.yData);
		Serial.print(" az: ");
		Serial.println(data.zData);
	#endif

	#ifdef ENABLE_LOWG
		auto data_adxl = sensor.getAccelerations();
		Serial.print("ax: ");
		Serial.print(data_adxl.x);
		Serial.print(" ay: ");
		Serial.print(data_adxl.y);
		Serial.print(" az: ");
		Serial.println(data_adxl.z);
	#endif

	#ifdef ENABLE_LOWGLSM
		float ax, ay, az, gx, gy, gz;
		LSM.readAcceleration(ax, ay, az);
		LSM.readGyroscope(gx, gy, gz);

		Serial.print("gx: ");
		Serial.print(gx);
		Serial.print(" gy: ");
		Serial.print(gy);
		Serial.print(" gz: ");
		Serial.print(gz);
		Serial.print(" ax: ");
		Serial.print(ax);
		Serial.print(" ay: ");
		Serial.print(ay);
		Serial.print(" az: ");
		Serial.println(az);
	#endif

	#ifdef ENABLE_MAGNETOMETER
		LIS3MDL.read();
		float mx = LIS3MDL.x_gauss;
		float my = LIS3MDL.y_gauss;
		float mz = LIS3MDL.z_gauss;
		Serial.print("mx: ");
		Serial.print(mx);
		Serial.print(" my: ");
		Serial.print(my);
		Serial.print(" mz: ");
		Serial.println(mz);
	#endif

	#ifdef ENABLE_ORIENTATION
		sh2_SensorValue_t event;
		Vec3 euler;
		if (imu.getSensorEvent(&event)) {
			switch (event.sensorId) {
				case SH2_ARVR_STABILIZED_RV:
					euler = quaternionToEulerRV(&event.un.arvrStabilizedRV, true);
				case SH2_GYRO_INTEGRATED_RV:
					euler = quaternionToEulerGI(&event.un.gyroIntegratedRV, true);
					break;
			}
			Serial.print("yaw: ");
			Serial.print(euler.y);
			Serial.print(" pitch: ");
			Serial.print(euler.z);
			Serial.print(" roll: ");
			Serial.println(euler.x);
		}
	#endif

	#ifdef ENABLE_ADS
		for (int i = 0; i < 8; i++) {
			Serial.print("Address ");
			Serial.print(i);
			Serial.print(": ");
			Serial.print(adcAnalogRead(ADCAddress{i}).value + ", ");
		}
		Serial.println();
	#endif

	#ifdef ENABLE_GPS
		myGNSS.checkUblox();
	    // GNSS_StatusTypeDef status =  teseo.update();
		// GPGGA_Info_t gpgga_message = teseo.getGPGGAData();
		// GPRMC_Info_t gprmc_message = teseo.getGPRMCData();
		// GSV_Info_t gsv_message = teseo.getGSVData();
		
		// double lat = gpgga_message.xyz.lat;
		// double lon = gpgga_message.xyz.lon;
		// float alt = gpgga_message.xyz.alt;
		// float v = gprmc_message.speed;
		// uint16_t sat_count = gpgga_message.sats;
		// double new_lat = floor(lat / 100.) + std::fmod(lat, 100.) / 60.;
		// double new_lon = floor(lon / 100.) + std::fmod(lon, 100.) / 60.;
		// //float n_lat = gpgga_message.xyz.lat / 100.f * ((gpgga_message.xyz.ns == 'N') ? 1. : -1.);
		// //float n_lon = gpgga_message.xyz.lon / 100.f * ((gpgga_message.xyz.ns == 'E') ? 1. : -1.);
		// Serial.print("Time: ");
		// Serial.print(gpgga_message.utc.hh);
		// Serial.print(":");
		// Serial.print(gpgga_message.utc.mm);
		// Serial.print(":");
		// Serial.print(gpgga_message.utc.ss);
		// Serial.print(" Satellite Fixes: ");
		// Serial.print(sat_count);
		// Serial.print("/");
		// Serial.print(gsv_message.tot_sats);
		// Serial.print(" Fix: ");
		// Serial.printf("%f, %f (%f, %f)", new_lat, new_lon, lat, lon);
		// // Serial.print("/");
		// // Serial.print(n_lon);
		// Serial.print(" Altitude: ");
		// Serial.print(alt);
		// Serial.print(" Velocity: ");
		// Serial.print(v);
		// Serial.print(" Status: ");
		// Serial.println(status);
	#endif

	#ifdef ENABLE_LORA

		Serial.println("Sending LoRa message");

		memcpy(TxdBuffer, PingMsg, sizeof(PingMsg));
		BufferSize = sizeof(PingMsg);
		Radio.Send(TxdBuffer, BufferSize); // Sends the PING

		delay(1000);

	#endif

	// Serial.println("Hello world!");
	delay(50);
}


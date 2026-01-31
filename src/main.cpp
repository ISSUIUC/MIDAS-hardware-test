/* basic testing script for midas board bringup */

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <FS.h>
#include <MicroNMEA.h>
#include <SD_MMC.h>

#include "pins.h"
#include "emmc_functions.h"
#include "TCAL9538.h"
#include "ads7138-q1.h"

#include <MS5611.h>
#include <SparkFun_MMC5983MA_Arduino_Library.h>
#include <SparkFun_u-blox_GNSS_v3.h>
#include <MicroNMEA.h> //http://librarymanager/All#MicroNMEA
#include <LoRaWan-Arduino.h>

#include <lsm6dsv320x.h> //should we use _reg file instead?

// SPISettings MMCSPISETTINGS = SPISettings(2000000, MSBFIRST, SPI_MODE0);

#define WAIT_FOR_SERIAL

// #define MCU_TEST
// #define I2C_SCAN
// #define ENABLE_BAROMETER
#define ENABLE_IMU
// #define ENABLE_MAGNETOMETER
// #define ENABLE_ADS
// #define ENABLE_GPIOEXP
// #define ENABLE_GPS
// #define ENABLE_LORA
// #define ENABLE_FLASH
// #define ENABLE_CHRISTMAS

// Please be careful
// This will init the gpio expander by itself
// #define PYRO_TEST

#ifdef ENABLE_IMU
	LSM6DSV320XClass LSM6DSV(SPI, LSM6DSV320X_CS, LSM_INT1);
#endif

#ifdef ENABLE_LORA
	hw_config hwConfig;
#endif

#ifdef ENABLE_BAROMETER
	MS5611 MS(MS5611_CS);
#endif

#ifdef ENABLE_MAGNETOMETER
	SFE_MMC5983MA MMC5983;
#endif

#ifdef ENABLE_GPS
SFE_UBLOX_GNSS myGNSS;
char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));
#endif

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

#ifdef MCU_TEST
	bool light_state = false;
#endif

#ifdef ENABLE_CHRISTMAS
// I wanted to be cute
#include <buzzer.h>

bool cur_light_state = false;

#endif

void setup() {
	Serial.begin(9600);

    Serial.println("HELLO");


	#ifdef WAIT_FOR_SERIAL
		while(!Serial) {};
		Serial.println("Serial ready");
	#endif


	delay(1000);

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

	Wire.begin(I2C_SDA, I2C_SCL);
	Serial.println("Initialized SPI");

	pinMode(LSM6DSV320X_CS, OUTPUT);
	pinMode(MMC5983_CS, OUTPUT);
	pinMode(MS5611_CS, OUTPUT);
	pinMode(E22_CS, OUTPUT);
	pinMode(LED_RED, OUTPUT);
	pinMode(LED_ORANGE, OUTPUT);
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_BLUE, OUTPUT);

	digitalWrite(MS5611_CS, HIGH);
	digitalWrite(LSM6DSV320X_CS, HIGH);
	digitalWrite(MMC5983_CS, HIGH);
	digitalWrite(E22_CS, HIGH);

#ifdef ENABLE_CHRISTMAS
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
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

	#ifdef ENABLE_IMU

		uint8_t whoami;
		LSM6DSV.device_id_get(&whoami);
		if(whoami != LSM6DSV320X_ID) { Serial.println("IMU Error. Cannot get the ID."); }

		else { Serial.println("IMU ID get."); }

        
		// the second parameter used to be normal instead of high-performance
		LSM6DSV.xl_setup(LSM6DSV320X_ODR_AT_7Hz5, LSM6DSV320X_XL_HIGH_PERFORMANCE_MD);
        LSM6DSV.gy_setup(LSM6DSV320X_ODR_AT_15Hz, LSM6DSV320X_GY_HIGH_PERFORMANCE_MD);
        
		// LSM6DSV.xl_full_scale_set(LSM6DSV320X_8g);
    	LSM6DSV.gy_full_scale_set(LSM6DSV320X_2000dps);
		
		// LSM6DSV_High_G_Enable
		LSM6DSV.hg_xl_data_rate_set(LSM6DSV320X_HG_XL_ODR_AT_960Hz, 1);
        
        LSM6DSV.sflp_enable_set(1);

	#endif

	#ifdef ENABLE_MAGNETOMETER
		// // if (!LIS3MDL.begin_SPI(LIS3MDL_CS)){
		// // 	Serial.println("could not init magnetometer");
		// // 	while(1);
		// // }
		// // LIS3MDL.setOperationMode(LIS3MDL_CONTINUOUSMODE);
		// // LIS3MDL.setDataRate(LIS3MDL_DATARATE_5_HZ);
		// // LIS3MDL.setRange(LIS3MDL_RANGE_4_GAUSS);
		// // Serial.println("magnetometer init successfully");

		while(MMC5983.begin(MMC5983_CS) == false) {
			Serial.println("Mag init failed");
			delay(200);
			MMC5983.softReset();
			delay(200);
		}
		// uint8_t rd = 0;
		// while (true) {

		// 	SPI.beginTransaction(MMCSPISETTINGS);
		// 	digitalWrite(MMC5983_CS, LOW);
		// 	SPI.transfer((0x80 | PROD_ID_REG)); // set msb for rd
		// 	rd = SPI.transfer(0);
		// 	digitalWrite(MMC5983_CS, HIGH);
		// 	SPI.endTransaction();

		// 	if(rd == 48) {
		// 		return;
		// 	}
		// 	delay(100);
		// }


		// }

		// sanity check
		int t = MMC5983.getTemperature();
		Serial.print("Reported die temp: ");
		Serial.print(t);
		Serial.println("C");

		// MMC5983.performSetOperation();
		// MMC5983.enableXChannel();
		// // MMC5983.enableYZChannels();
		// MMC5983.disableInterrupt();
		// MMC5983.disableContinuousMode();


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

		while (!TCAL9538Init(EXP_RST)) {
			Serial.println("Failed to initialize TCAL9538!");
			// while(1){ };
		}

		Serial.println("TCAL9538 initialized successfully!");

		for (int i = 0; i <= 7; i++) {
			gpioPinMode(GpioAddress(0, i), OUTPUT);
			gpioDigitalWrite(GpioAddress(0, i), LOW);
		}
		gpioPinMode(GpioAddress(0, 4), INPUT);
		Serial.println(gpioDigitalRead(GpioAddress(0, 4)).value);

	#endif

	#ifdef PYRO_TEST

		pinMode(BUZZER_PIN, OUTPUT);
		digitalWrite(BUZZER_PIN, LOW);

		if (!TCAL9538Init()) {
			Serial.println("Failed to initialize TCAL9538!");
			// while(1){ };
		}

		Serial.println("TCAL9538 initialized successfully!");

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
}

void loop() {

	
	#ifdef MCU_TEST
		Serial.println("hello world");

		light_state = !light_state;

		digitalWrite(LED_RED, light_state ? HIGH : LOW);
		digitalWrite(LED_ORANGE, light_state ? HIGH : LOW);
		digitalWrite(LED_GREEN, light_state ? HIGH : LOW);
		digitalWrite(LED_BLUE, light_state ? HIGH : LOW);

	#endif

	#ifdef I2C_SCAN
		Serial.println("Beginning I2C Scan:");
		for (int i = 0; i<128; i++){
			Wire.beginTransmission(i);
			if (!Wire.endTransmission()){
				Serial.print("Device found at: 0x");
				Serial.println(i, HEX);
			}
		}
		delay(1000);

	#endif

	#ifdef ENABLE_CHRISTMAS
		// just play the entire song
		// assume gpio is enabled this is for fun anyway

		Serial.println("Playing song!");

		for(unsigned i = 0; i < MERRY_CHRISTMAS_LENGTH; i++) {

			cur_light_state = !cur_light_state;

			digitalWrite(LED_BLUE, LOW);
			digitalWrite(LED_ORANGE, LOW);
			digitalWrite(LED_GREEN, cur_light_state ? HIGH : LOW);
			digitalWrite(LED_RED, cur_light_state ? LOW : HIGH);

			Sound cur_sound = merry_christmas[i];
			tone(BUZZER_PIN, cur_sound.frequency);
			delay(cur_sound.duration_ms);
			noTone(BUZZER_PIN);
			delay(10);
		}

		Serial.println("Delaying 2s before playing again");
		delay(2000);
		

	#endif

	#ifdef ENABLE_GPIOEXP
		gpioDigitalWrite(GpioAddress(0, 0), HIGH);
		gpioDigitalWrite(GpioAddress(0, 1), HIGH);
		gpioDigitalWrite(GpioAddress(0, 6), HIGH);
		gpioDigitalWrite(GpioAddress(0, 7), HIGH);
		Serial.println("high");
		delay(1000);
		gpioDigitalWrite(GpioAddress(0, 0), LOW);
		gpioDigitalWrite(GpioAddress(0, 1), LOW);
		gpioDigitalWrite(GpioAddress(0, 6), LOW);
		gpioDigitalWrite(GpioAddress(0, 7), LOW);
		Serial.println("low");
		delay(1000);
	#endif

	#ifdef PYRO_TEST

		char* buf[255];

		gpioDigitalWrite(GpioAddress(0, 03), CUR_PYRO == 0 ? LOW : HIGH); // pyro enabled only if not 0
		gpioDigitalWrite(GpioAddress(0, 00), CUR_PYRO == 1 ? HIGH : LOW);
		gpioDigitalWrite(GpioAddress(0, 01), CUR_PYRO == 2 ? HIGH : LOW);
		gpioDigitalWrite(GpioAddress(0, 07), CUR_PYRO == 3 ? HIGH : LOW);
		gpioDigitalWrite(GpioAddress(0, 06), CUR_PYRO == 4 ? HIGH : LOW);

		size_t bytes_read = Serial.readBytesUntil('\n', (char*)&buf, 10);
		if(bytes_read > 0) {
			CUR_PYRO = (CUR_PYRO + 1) % 5;
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

	#ifdef ENABLE_IMU

		int16_t raw_accel[3];
		int16_t raw_accel_hg[3];
		int16_t raw_ar[3];

		lsm6dsv320x_status_reg_t status = LSM6DSV.get_status();

        uint16_t val[4]; //make this uint16 array

       LSM6DSV.lsm6dsv320x_sflp_quaternion_raw_get(val); //send it thru this

		Serial.printf("SFLP Quaternion: 0x%lx\n", val); //display each value in vel --> Fix this line, it is currently nono

        LSM6DSV.sflp_gravity_raw_get((int16_t*)&val);
		Serial.printf("SFLP Gravity vector: 0x%lx\n", val);

        LSM6DSV.sflp_gbias_raw_get((int16_t*)&val);
		Serial.printf("SFLP Gyroscope Bias: 0x%lx\n", val);

		/*
		if(status.gda) {
			LSM6DSV.acceleration_raw_get(raw_accel);
			Serial.printf("LowG Acceleration\nX: %f\nY: %F\nZ: %f\n", LSM6DSV.from_fs2_to_mg(raw_accel[0])/1000, LSM6DSV.from_fs2_to_mg(raw_accel[1])/1000, LSM6DSV.from_fs2_to_mg(raw_accel[2])/1000);
		}*/
			

		LSM6DSV.hg_xl_full_scale_set(LSM6DSV320X_64g); //this line here should set it to 64gs
		//fs2tomg function only converts at 2g scale, we must find a way to do it in 64g scale. We can figure it out.

		if(status.xlhgda) {
			LSM6DSV.hg_acceleration_raw_get(raw_accel_hg);
			Serial.printf("HighG Acceleration\nX: %f\nY: %F\nZ: %f\n", LSM6DSV.from_fs2_to_mg(raw_accel_hg[0])/1000, LSM6DSV.from_fs2_to_mg(raw_accel_hg[1])/1000, LSM6DSV.from_fs2_to_mg(raw_accel_hg[2])/1000);
		}	
		
		/*
		if(status.gda) {
			LSM6DSV.angular_rate_raw_get(raw_ar);
			Serial.printf("Angular Rate\nX: %f\nY: %F\nZ: %f\n", LSM6DSV.from_fs2000_to_mdps(raw_ar[0])/1000, LSM6DSV.from_fs2000_to_mdps(raw_ar[1])/1000, LSM6DSV.from_fs2000_to_mdps(raw_ar[2])/1000);
		}*/

	#endif

	#ifdef ENABLE_MAGNETOMETER
		uint32_t cx, cy, cz;
		double X, Y, Z;

		MMC5983.getMeasurementXYZ(&cx, &cy, &cz);

		Serial.print("Mag measurement: ");


		double sf = (double)(1 << 17);
		X = ((double)cx - sf)/sf;
		Y = ((double)cy - sf)/sf;
		Z = ((double)cz - sf)/sf;
		Serial.print(X);
		Serial.print(" ");
		Serial.print(Y);
		Serial.print(" ");
		Serial.println(Z);
		

	#endif

	#ifdef ENABLE_ADS
		// for (int i = 0; i < 8; i++) {
		// 	Serial.print("Address ");
		// 	Serial.print(i);
		// 	Serial.print(": ");
		// 	Serial.println(adcAnalogRead(ADCAddress{i}).value);
		// }
		Serial.print("0: ");
		Serial.println((adcAnalogRead(ADCAddress{0}).value)/4096.0*6.55*3.3);
		Serial.print("1: ");
		Serial.println((adcAnalogRead(ADCAddress{1}).value)/4096.0*6.55*3.3);
		Serial.print("2: ");
		Serial.println((adcAnalogRead(ADCAddress{2}).value)/4096.0*6.60*3.3);
		Serial.print("3: ");
		Serial.println((adcAnalogRead(ADCAddress{3}).value)/4096.0*3.3);
		Serial.print("4: ");
		Serial.println((adcAnalogRead(ADCAddress{4}).value)/4096.0*6.55*3.3);
		Serial.print("5: ");
		Serial.println((adcAnalogRead(ADCAddress{5}).value)/4096.0*6.55*3.3);
		Serial.print("6: ");
		Serial.println((adcAnalogRead(ADCAddress{6}).value)/4096.0*2.00*3.3);
		Serial.print("7: ");
		Serial.println((adcAnalogRead(ADCAddress{7}).value)/4096.0*2.00*3.3);
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
	delay(1000);
}


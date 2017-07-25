/*
  LoRa Simple Client for Arduino :
  Support Devices: LoRa Shield + Arduino

  Example sketch showing how to create a simple messageing client,
  with the RH_RF95 class. RH_RF95 class does not provide for addressing or
  reliability, so you should only use RH_RF95 if you do not need the higher
  level messaging abilities.

  It is designed to work with the other example LoRa Simple Server

  modified 16 11 2016
  by Edwin Chen <support@dragino.com>
  Dragino Technology Co., Limited
*/

#include <SPI.h>
#include <RH_RF95.h>

#define echoPin 7 // Echo Pin
#define trigPin 4 // Trigger Pin

#define SENDER_ID 1

// Singleton instance of the radio driver
RH_RF95 rf95;
float frequency = 433.0;

enum sensor_t {
  HUMIDITY = 1,
  TEMPERATURE = 2,
  DUST = 3,
  ULTRASONIC = 4,
  SOUND = 5,
};

enum message_t {
  SENSOR_READING = 1,
};

struct {
  message_t type;
  byte sender_id;
  sensor_t sensor_type;
  long sensor_reading;
} message;

byte tx_buf[sizeof(message)];

const int pinAdc = A0;
const int B = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k

void setup()
{
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  Serial.println("Start LoRa Client");
  if (!rf95.init())
    Serial.println("init failed");
  // Setup ISM frequency
  rf95.setFrequency(frequency);
  // Setup Power,dBm
  rf95.setTxPower(13);
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  message.type = SENSOR_READING;
  message.sender_id = SENDER_ID;
  message.sensor_type = TEMPERATURE;
}

void loop()
{

    long temp = 0;
    temp = analogRead(pinAdc);


    float R = 1023.0/temp-1.0;
    R = R0*R;

    float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15;
    temperature=temperature*100;

  Serial.println("Sending to LoRa Server");
  // Send a message to LoRa Server

  message.sensor_reading = temperature;

  memcpy(tx_buf, &message, sizeof(tx_buf));

  rf95.send(tx_buf, sizeof(tx_buf));

  Serial.print("Sent: ");
  Serial.println(message.sensor_reading);

  rf95.waitPacketSent();

  delay(1000);
}



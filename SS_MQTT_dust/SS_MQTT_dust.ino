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

#define sensorPin A0

#define SENDER_ID 2

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

unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 3000; //sample 30s
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;

void send_packet(long reading, sensor_t type) {
  Serial.println("Sending to LoRa Server");

  message.sensor_reading = reading;
  message.sensor_type = type;

  memcpy(tx_buf, &message, sizeof(tx_buf));

  rf95.send(tx_buf, sizeof(tx_buf));

  Serial.print("Sent: ");
  Serial.print(message.sensor_reading);
  Serial.print(" sensor: ");
  Serial.println(message.sensor_type);

  rf95.waitPacketSent();
}

void setup()
{
  Serial.begin(9600);
  //while (!Serial) ; // Wait for serial port to be available
  Serial.println("Start LoRa Client");
  if (!rf95.init())
    Serial.println("init failed");
  // Setup ISM frequency
  rf95.setFrequency(frequency);
  // Setup Power,dBm
  rf95.setTxPower(13);
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // DUST sensor
  pinMode(8, INPUT);
  starttime = millis(); //get the current time;

  // Setup message
  message.type = SENSOR_READING;
  message.sender_id = SENDER_ID;
}

void loop()
{
  duration = pulseIn(sensorPin, LOW);
  lowpulseoccupancy = lowpulseoccupancy + duration;

  if ((millis() - starttime) >= sampletime_ms) //if the sampel time = = 30s
  {
    ratio = lowpulseoccupancy / (sampletime_ms * 10.0); // Integer percentage 0=&gt;100
    concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62; // using spec sheet curve
    Serial.print("concentration = ");
    Serial.print(concentration);
    Serial.println(" pcs/0.01cf");
    Serial.println("\n");
    lowpulseoccupancy = 0;
    starttime = millis();
  }

  send_packet(long(concentration * 10000), DUST);

  delay(5000);
}


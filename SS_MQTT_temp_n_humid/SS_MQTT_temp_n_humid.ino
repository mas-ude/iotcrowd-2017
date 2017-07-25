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

#define SENDER_ID 2

#define DHT11_PIN 2


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

byte dht11_data[5];
uint8_t len_dht11_data = sizeof(dht11_data);

byte read_dht11_dat()
{
  byte i = 0;
  byte result = 0;
  for (i = 0; i < 8; i++) {

    while (!(PINC & _BV(DHT11_PIN))); // wait for 50us
    delayMicroseconds(30);

    if (PINC & _BV(DHT11_PIN))
      result |= (1 << (7 - i));
    while ((PINC & _BV(DHT11_PIN))); // wait '1' finish
  }
  return result;
}

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

boolean get_temp_and_humid(byte* data, uint8_t len) {
  byte dht11_in;
  byte i;

  // start condition
  // 1. pull-down i/o pin from 18ms
  PORTC &= ~_BV(DHT11_PIN);
  delay(18);
  PORTC |= _BV(DHT11_PIN);
  delayMicroseconds(40);

  DDRC &= ~_BV(DHT11_PIN);
  delayMicroseconds(40);

  dht11_in = PINC & _BV(DHT11_PIN);

  if (dht11_in) {
    Serial.println("dht11 start condition 1 not met");
    return false;
  }
  delayMicroseconds(80);

  dht11_in = PINC & _BV(DHT11_PIN);

  if (!dht11_in) {
    Serial.println("dht11 start condition 2 not met");
    return false;
  }
  delayMicroseconds(80);
  // now ready for data reception
  for (i = 0; i < 5; i++) {
    data[i] = read_dht11_dat();
  }

  DDRC |= _BV(DHT11_PIN);
  PORTC |= _BV(DHT11_PIN);

  byte dht11_check_sum = data[0] + data[1] + data[2] + data[3];
  // check check_sum
  if (data[4] != dht11_check_sum) {
    Serial.println("DHT11 checksum error");
    return false;
  }

  return true;
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

  // Temp&Humid sensor
  DDRC |= _BV(DHT11_PIN);
  PORTC |= _BV(DHT11_PIN);

  // Setup message
  message.type = SENSOR_READING;
  message.sender_id = SENDER_ID;
}

void loop()
{
  boolean res = get_temp_and_humid(dht11_data, len_dht11_data);
  if (res) {
    byte dec = dht11_data[1];
    while (dec >= 100) {
      dec /= 10;
    }
    long humidity = dht11_data[0] * 100 + dec;
    send_packet(humidity, HUMIDITY);

    delay(500);

    dec = dht11_data[3];
    while (dec >= 100) {
      dec /= 10;
    }
    long temperature = dht11_data[2] * 100 + dec;
    send_packet(temperature, TEMPERATURE);
  }

  delay(5000);
}


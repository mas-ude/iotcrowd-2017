#include <Ethernet.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Console.h>
#include <RH_RF95.h>

#define BAUDRATE 115200

RH_RF95 rf95;
float frequency = 433.0;


const char* mqtt_server = "10.130.1.100";
const char* mqtt_port = "1883";
const char* mqtt_topic = "dragino";

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

// Buffers
uint8_t rx_buf[sizeof(message)];
uint8_t rx_buf_len = sizeof(rx_buf);
char msg_buffer[120];
uint8_t msg_len = sizeof(msg_buffer);

void setup() {
  Bridge.begin(BAUDRATE);

  //while (!Console) {}

  Console.println("Start Sketch");
  if (!rf95.init())
    Console.println("init failed");
  // Setup ISM frequency
  rf95.setFrequency(frequency);
  // Setup Power,dBm
  rf95.setTxPower(13);
  // Defaults BW Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  Console.print("Listening on frequency: ");
  Console.println(frequency);
}

void loop() {
  if (rf95.available())
  {
    rx_buf_len = sizeof(rx_buf);

    if (rf95.recv(rx_buf, &rx_buf_len)) {
      memcpy(&message, rx_buf, sizeof(message));

      // See message type
      if (message.type == SENSOR_READING) {
        msg_len = sprintf(msg_buffer, "{\"sender\":%d,\"sensor\":%d,\"reading\":%ld}",
                          message.sender_id, message.sensor_type, message.sensor_reading);
      }
      else {
        msg_len = sprintf(msg_buffer, "Sender %d sends unknown message type %d.", message.sender_id, message.type);
      }

      //RH_RF95::printBuffer("request: ", rx_buf, rx_buf_len);
      Console.println("Sending request via MQTT: ");
      //Console.print(value, DEC);
      Console.print("RSSI: ");
      Console.println(rf95.lastRssi(), DEC);

      Process p;
      p.begin("mosquitto_pub");
      p.addParameter("-h");
      p.addParameter(mqtt_server);
      p.addParameter("-p");
      p.addParameter(mqtt_port);
      p.addParameter("-t");
      p.addParameter(mqtt_topic);
      p.addParameter("-m");
      p.addParameter(msg_buffer);
      p.run();
    }
    else {
      Console.println("recv failed");
    }
  }
}


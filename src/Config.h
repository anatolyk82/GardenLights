#ifndef ESP_LIGHT_CONFIG_H
#define ESP_LIGHT_CONFIG_H

#include <string>

#define FIRMWARE_VERSION "0.1.0"                    /* Firmware version */

#define INTERVAL_PUBLISH_STATE 600000               /* Interval to send statistics to the mqtt broker */

/* MQTT Settings */
#define MQTT_TOPIC1_STATE "light/garden1"           /* state report MQTT topic */
#define MQTT_TOPIC1_SET "light/garden1/set"         /* command MQTT topic */

#define MQTT_TOPIC2_STATE "light/garden2"           /* state report MQTT topic */
#define MQTT_TOPIC2_SET "light/garden2/set"         /* command MQTT topic */

#define MQTT_TOPIC3_STATE "light/garden3"           /* state report MQTT topic */
#define MQTT_TOPIC3_SET "light/garden3/set"         /* command MQTT topic */

#define MQTT_TOPIC4_STATE "light/garden4"           /* state report MQTT topic */
#define MQTT_TOPIC4_SET "light/garden4/set"         /* command MQTT topic */

#define MQTT_TOPIC_STATUS "light/garden/status"     /* status MQTT topic: online/offline */
#define MQTT_STATUS_PAYLOAD_ON "online"
#define MQTT_STATUS_PAYLOAD_OFF "offline"

#define MQTT_KEEP_ALIVE_SECONDS 30

// Enable DS18B20 temperature sensor to measure
// the temperature inside the power supply.
#define ENABLE_TEMPERATURE_SENSOR 0

/* WiFi Manager settings */
#define WIFI_AP_NAME "GardenLights"
#define WIFI_AP_PASS "123456789"


/* Device settings */
#define DATA_PIN_1    D1
#define NUM_LEDS_1    4

#define DATA_PIN_2    D2
#define NUM_LEDS_2    6


/* Calculates uptime for the device */
inline char *uptime(unsigned long milli) {
  static char _return[32];
  unsigned long secs=milli/1000, mins=secs/60;
  unsigned int hours=mins/60, days=hours/24;
  milli-=secs*1000;
  secs-=mins*60;
  mins-=hours*60;
  hours-=days*24;
  sprintf(_return,"%dT%2.2d:%2.2d:%2.2d.%3.3d", (byte)days, (byte)hours, (byte)mins, (byte)secs, (int)milli);
  return _return;
}

#endif //ESP_LIGHT_CONFIG_H

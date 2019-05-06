#ifndef ESP_LIGHT_MQTT_CLIENT_H
#define ESP_LIGHT_MQTT_CLIENT_H

#include <AsyncMqttClient.h>      // https://github.com/marvinroger/async-mqtt-client + (https://github.com/me-no-dev/ESPAsyncTCP)
#include <ESP8266WiFi.h>          // https://github.com/esp8266/Arduino
#include <ArduinoJson.h>          // https://github.com/bblanchon/ArduinoJson (ver: 5.x)

#include <functional>
#include <vector>

#include "Config.h"

#if (ENABLE_TEMPERATURE_SENSOR == 1)
#include "TemperatureSensor.h"
#endif

/*
 * The structure of the JSON document:
 * {
 *   "state": "ON",
 *   "effect": "EffectName",
 *   "brightness": 255,
 *   "color": { "r": 255, "g": 255, "b": 255 },
 *   "transition": 10,
 *   "ip": "192.168.1.110",
 *   "mac": "11:22:33:44:55:66",
 *   "rssi": "-67",
 *   "uptime": "12:23:33"
 * }
 */
class LightDevice;

class DeviceMqttClient : public AsyncMqttClient
{
public:
  DeviceMqttClient();
  ~DeviceMqttClient();

  /*
   * Publish an MQTT message with a current state of the device.
   * It sends a json document as the payload of mqt message.
   */
  void publishDeviceState();

  /*
   * Set a reference to the device state structure.
   * This must be called in setup() before sending/receiving any data.
   */
  void setDevices(const std::vector<LightDevice*> &allDevices) {
    m_allDevices = allDevices;
  }

private:
  void onMqttConnect(bool sessionPresent);
  void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
  void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);

  std::vector<LightDevice*> m_allDevices;

#if (ENABLE_TEMPERATURE_SENSOR == 1)
  TemperatureSensor *m_tempSensor = nullptr;
#endif
};


#endif //ESP_LIGHT_MQTT_CLIENT_H

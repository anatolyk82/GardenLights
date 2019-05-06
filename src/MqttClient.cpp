#include "MqttClient.h"
#include "LightDevice.h"

DeviceMqttClient::DeviceMqttClient() : AsyncMqttClient()
{
  onConnect( std::bind(&DeviceMqttClient::onMqttConnect, this, std::placeholders::_1) );
  onDisconnect( std::bind(&DeviceMqttClient::onMqttDisconnect, this, std::placeholders::_1) );
  onMessage( std::bind(&DeviceMqttClient::onMqttMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6) );
#if (ENABLE_TEMPERATURE_SENSOR == 1)
  m_tempSensor = new TemperatureSensor(D6);
#endif
}

DeviceMqttClient::~DeviceMqttClient()
{
#if (ENABLE_TEMPERATURE_SENSOR == 1)
  delete m_tempSensor;
#endif
}

void DeviceMqttClient::onMqttConnect(bool sessionPresent)
{
  Serial.println("MQTT: Connected");
  Serial.printf("MQTT: Session present: %d\n", sessionPresent);

  /* Subscribe all command topics */
  for (LightDevice* dev : m_allDevices) {
    if (!dev->mqttCommandTopic().empty()) {
      Serial.printf("MQTT: Subscribing at QoS 0, topic: %s\n", dev->mqttCommandTopic().c_str());
      subscribe(dev->mqttCommandTopic().c_str(), 0);
    }
  }

  Serial.printf("MQTT: Publish online status: %s\n", MQTT_TOPIC_STATUS);
  publish(MQTT_TOPIC_STATUS, 1, true, MQTT_STATUS_PAYLOAD_ON);

  publishDeviceState();
}

void DeviceMqttClient::onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  Serial.println();
  Serial.print("MQTT: Disconnected: ");
  if (reason == AsyncMqttClientDisconnectReason::TCP_DISCONNECTED) {
    Serial.println("TCP disconnected");
  } else if (reason == AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION) {
    Serial.println("Unacceptable protocol version");
  } else if (reason == AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED) {
    Serial.println("Indentifier rejected");
  } else if (reason == AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE) {
    Serial.println("Server unavailable");
  } else if (reason == AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS) {
    Serial.println("Malformed credentials");
  } else if (reason == AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED) {
    Serial.println("Not authorized");
  } else {
    Serial.println("Unknown reason");
  }

  int8_t attemptToConnectCounter = 0;
  delay(3000);
  Serial.println("MQTT: Attemp to reconnect to the broker");
  while ( (connected() == false) && (attemptToConnectCounter < 100) ) {
    if (WiFi.isConnected()) {
      Serial.printf("MQTT: Attempt %d. Reconnecting to broker ... \n", attemptToConnectCounter);
      connect();
    } else {
      Serial.println("MQTT: WiFi is not connected. Wait.");
    }
    delay(3000);
    attemptToConnectCounter += 1;
  }
}


void DeviceMqttClient::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  Serial.println();
  Serial.println("MQTT: Message received.");
  Serial.printf("  topic: %s\n", topic);
  Serial.printf("  qos: %d\n", properties.qos);
  Serial.printf("  dup: %d\n", properties.dup);
  Serial.printf("  retain: %d\n", properties.retain);

  Serial.print("  payload: ");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(payload);
  json.printTo(Serial);
  Serial.println();

  /* Find device which is receiving this command */
  LightDevice* curr_dev = nullptr;
  for (LightDevice* dev : m_allDevices) {
    if (dev->mqttCommandTopic() == std::string(topic)) {
      curr_dev = dev;
      break;
    }
  }

  /* Set all received parameters to the device */
  Serial.print("MQTT: Parsing received JSON ... ");
  if (json.success()) {
    Serial.println("ok");
    if (json.containsKey("state")) {
      const char* state = json["state"];
      String stringState(state);
      curr_dev->setState( (stringState == "ON") );

      if (json.containsKey("brightness")) {
        curr_dev->setBrightness( json["brightness"] );
      }

      if (json.containsKey("color")) {
        curr_dev->setRed(json["color"]["r"]);
        curr_dev->setGreen(json["color"]["g"]);
        curr_dev->setBlue(json["color"]["b"]);
      }

      if (json.containsKey("effect")) {
        const char* effect = json["effect"];
        curr_dev->setEffect(std::string(effect));
      } else {
        curr_dev->setEffect("");
      }

      if (json.containsKey("transition")) {
        curr_dev->setTransition(json["transition"]);
      } else {
        curr_dev->setTransition(0);
      }

      /* Update actual state of the device*/
      curr_dev->updateDevice();

      this->publishDeviceState();
    }
  } else {
    Serial.println("failed");
  }
}


void DeviceMqttClient::publishDeviceState() {
  for (LightDevice* dev : m_allDevices) {
    const int BUFFER_SIZE = JSON_OBJECT_SIZE(20);
    StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

    JsonObject& root = jsonBuffer.createObject();

    // Light state: state, color, brightness
    root["state"] = dev->state() ? "ON" : "OFF";
    JsonObject& color = root.createNestedObject("color");
    color["r"] = dev->red();
    color["g"] = dev->green();
    color["b"] = dev->blue();
    root["brightness"] = dev->brightness();

    if ( dev->effect() != "" ) {
      root["effect"] = dev->effect().c_str();
    }/* else {
      root["effect"] = ""; //TODO: Is this really needed to send an empty string ?
    }*/

    if ( dev->transition() != 0 ) {
      root["transition"] = dev->transition();
    }

    // Additional parameters: IP-address, mac-address, RSSI, uptime, Firmware version
    // IP-address
    char ip[16];
    memset(ip, 0, 18);
    sprintf(ip, "%s", WiFi.localIP().toString().c_str());
    root["ip"] = ip;

    // MAC address
    uint8_t macAddr[6];
    WiFi.macAddress(macAddr);
    char mac[18];
    memset(mac, 0, 18);
    sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
    root["mac"] = mac;

    // RSSI
    char rssi[8];
    sprintf(rssi, "%d", WiFi.RSSI());
    root["rssi"] = rssi;

    // Uptime
    root["uptime"] = uptime( millis() );

    // Firmware version
    root["version"] = FIRMWARE_VERSION;

  #if (ENABLE_TEMPERATURE_SENSOR == 1)
    float t = m_tempSensor->requestTemperatureC();
    root["temperature"] = t;
  #endif

    char buffer[root.measureLength() + 1];
    root.printTo(buffer, sizeof(buffer));

    Serial.printf("\nMQTT: Publish state: %s %s\n", dev->mqttStateTopic().c_str(), buffer);

    publish(dev->mqttStateTopic().c_str(), 0, true, buffer);

    //yield();
  }
}

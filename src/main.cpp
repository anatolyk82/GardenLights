
#include <SimpleTimer.h>          // https://github.com/schinken/SimpleTimer

#include "Config.h"
#include "UiManager.h"
#include "MqttClient.h"
#include "LightDevice.h"

#include <vector>

/* Create a UI manager */
UiManager uiManager;

/* MQTT Client instance */
DeviceMqttClient mqttClient;

/* Device control object */
LightDevice *device1;
LightDevice *device2;

std::vector<LightDevice*> allDevices;

/* Timer to publish the current state */
SimpleTimer timer;

void publishDeviceStateTimer() {
  mqttClient.publishDeviceState();
}

void setup() {

  /* Init serial port */
  Serial.begin(115200);
  Serial.println();

  //clean FS, for testing
  //SPIFFS.format();

  /* Initialize all devices */
  device1 = new LightDevice(DATA_PIN_1, NUM_LEDS_1);
  device1->setMqttStateTopic(MQTT_TOPIC1_STATE);
  device1->setMqttCommandTopic(MQTT_TOPIC1_SET);
  allDevices.push_back(device1);

  device2 = new LightDevice(DATA_PIN_2, NUM_LEDS_2);
  device2->setMqttStateTopic(MQTT_TOPIC2_STATE);
  device2->setMqttCommandTopic(MQTT_TOPIC2_SET);
  allDevices.push_back(device2);

  /* Create UI and connect to WiFi */
  uiManager.initUIManager(false);

  /* Configure MQTT */
  mqttClient.setDevices( allDevices );
  int p = atoi( uiManager.mqttPort() );
  mqttClient.setServer( uiManager.mqttServer(), p );
  mqttClient.setCredentials( uiManager.mqttLogin(), uiManager.mqttPassword() );
  mqttClient.setKeepAlive( MQTT_KEEP_ALIVE_SECONDS );
  mqttClient.setWill( MQTT_TOPIC_STATUS, 1, true, MQTT_STATUS_PAYLOAD_OFF ); //topic, QoS, retain, payload
  String string_client_id( uiManager.mqttClientId() );
  string_client_id.trim();
  if (string_client_id != String("")) {
    mqttClient.setClientId( uiManager.mqttClientId() );
  }

  /* Set a callback to update the actual state of the device when an mqtt command is received */
  //mqttClient.onMessageReveived( std::bind(&LightDevice::updateDeviceState, &device) );

  /* Connect the MQTT client to the broker */
  int8_t attemptToConnectToMQTT = 0;
  Serial.println("MQTT: Connect to the broker");
  while ( (mqttClient.connected() == false) && (attemptToConnectToMQTT < 5) ) {
    if (WiFi.isConnected()) {
      Serial.printf("MQTT: Attempt %d. Connecting to broker [%s:%d]: login: [%s] password: [%s] ... \n", attemptToConnectToMQTT, uiManager.mqttServer(), p, uiManager.mqttLogin(), uiManager.mqttPassword() );
      mqttClient.connect();
    } else {
      //attemptToConnectToMQTT = 0;
      Serial.println("MQTT: WiFi is not connected. Try to reconnect WiFi.");
      WiFi.reconnect();
    }
    delay(3000);
    attemptToConnectToMQTT += 1;
  }

  /* If there is still no connection here, restart the device */
  if (!WiFi.isConnected()) {
    Serial.println("setup(): WiFi is not connected. Reset the device to initiate connection again.");
    ESP.restart();
  }

  if (!mqttClient.connected()) {
    Serial.println("setup(): The device is not connected to MQTT. Reset the device to initiate connection again.");
    ESP.restart();
  }

  /* Publish device state periodicly */
  timer.setInterval(INTERVAL_PUBLISH_STATE, publishDeviceStateTimer);
}


void loop() {
  timer.run();

  device1->run();
  device2->run();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("loop(): WiFi is not connected. Reset the device to initiate connection again.");
    ESP.restart();
  }
}

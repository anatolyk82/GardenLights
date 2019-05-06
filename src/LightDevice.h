#ifndef ESP_LIGHT_DEVICE_CONTROL_H
#define ESP_LIGHT_DEVICE_CONTROL_H

#define FASTLED_ALLOW_INTERRUPTS 0 //Avoid flickering ?
#include <FastLED.h>

#include <map>
#include "Config.h"

// TODO: make these as properties of the class
#define LED_TYPE    WS2811
#define COLOR_ORDER  RGB

struct ChaosEffectHelper {
  uint8_t hue;
  uint8_t saturation;
  uint8_t brightness;
  uint8_t maxBrightness;
  bool up;
};

/*
 * This class may differ from one device to another
 * depending on how the device is controled.
 */
class LightDevice
{
public:
  LightDevice(const uint8_t pin, const uint16_t num_leds);
  LightDevice(const LightDevice&) = delete;
  ~LightDevice();

  /*
   * The function applies all changes set by the setters.
   */
  void updateDevice();

  /*
   * Run the device.
   * It must be called in loop()
   */
  void run();

  /* Effects */
  void efSparkles();
  void efColorLoop();
  void efChaos();

  /* Getters */
  bool state() const { return m_state; }
  uint8_t red() const { return m_red; }
  uint8_t green() const { return m_green; }
  uint8_t blue() const { return m_blue; }
  uint8_t brightness() const { return m_brightness; }
  //CRGB* leds() const { return m_leds; } //TODO: Make it priviate ?
  uint8_t pin() const { return m_pin; }
  uint16_t numLEDs() const { return m_num_leds; }
  int transition() const { return m_transition; }
  std::string effect() const { return m_effect; }
  std::string mqttStateTopic() const { return m_mqtt_state_topic; }
  std::string mqttCommandTopic() const { return m_mqtt_command_topic; }

  /* Setters */
  void setState(const bool state) { m_state = state; }
  void setRed(const uint8_t red) { m_red = red; }
  void setGreen(const uint8_t green) { m_green = green; }
  void setBlue(const uint8_t blue) { m_blue = blue; }
  void setBrightness(const uint8_t brightness) { m_brightness = brightness; }
  void setTransition(const int transition) { m_transition = transition; }
  void setEffect(const std::string& effect) { m_effect = effect; }
  void setMqttStateTopic(const std::string& mqtt_state_topic) { m_mqtt_state_topic = mqtt_state_topic; }
  void setMqttCommandTopic(const std::string& mqtt_command_topic) { m_mqtt_command_topic = mqtt_command_topic; }

private:
  void transitionEffect();
  float m_current_brightness;
  bool m_inTransition = false;

  void setColor() const;

  /* Effects */
  uint8_t ef_hue = 0;
  ChaosEffectHelper *m_chaosEffectHelper = nullptr;

  std::map< std::string, std::function<void(void)> > m_lightEffectsList = {
    {"Sparkles",       std::bind(&LightDevice::efSparkles, this)},
    {"Colorloop",      std::bind(&LightDevice::efColorLoop, this)},
    {"Chaos",          std::bind(&LightDevice::efChaos, this)},
  };

  /* Properties */
  bool m_state = false;
  uint8_t m_red = 255;
  uint8_t m_green = 255;
  uint8_t m_blue = 255;
  uint8_t m_brightness = 128;
  int m_transition;
  std::string m_effect = "";

  CRGB *m_leds = nullptr;
  const uint8_t m_pin;
  const uint16_t m_num_leds;

  std::string m_mqtt_state_topic = "";
  std::string m_mqtt_command_topic = "";
};

#endif //ESP_LIGHT_DEVICE_CONTROL_H

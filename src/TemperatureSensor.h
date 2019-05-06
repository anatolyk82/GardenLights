#ifndef ESP_LIGHT_DEVICE_TEMPERATURE_H
#define ESP_LIGHT_DEVICE_TEMPERATURE_H

#include <OneWire.h>  // Control 1-Wire protocol (DS18S20, DS18B20, DS2408 and etc)
#include <DallasTemperature.h>  // Arduino Library for Dallas Temperature ICs (DS18B20, DS18S20, DS1822, DS1820)

class TemperatureSensor
{
public:
  TemperatureSensor(uint8_t sensorPin);
  TemperatureSensor(const TemperatureSensor&) = delete;
  ~TemperatureSensor();

  float requestTemperatureC(); // Request the temp in Celsius
  float requestTemperatureF(); // Request the temp in Fahrenheit

private:
  OneWire *m_oneWire =  nullptr;
  DallasTemperature *m_sensor = nullptr;
  DeviceAddress insideThermometer;
};

#endif //ESP_LIGHT_DEVICE_TEMPERATURE_H

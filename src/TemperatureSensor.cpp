#include "TemperatureSensor.h"


TemperatureSensor::TemperatureSensor(uint8_t sensorPin)
{
  m_oneWire = new OneWire(sensorPin);
  m_sensor = new DallasTemperature(m_oneWire);

  m_sensor->begin();
  m_sensor->getAddress(insideThermometer, 0);

  m_sensor->setResolution(insideThermometer, 9);
}

TemperatureSensor::~TemperatureSensor()
{
  delete m_sensor;
  delete m_oneWire;
}

float TemperatureSensor::requestTemperatureC()
{
  m_sensor->requestTemperatures();
  float tempC = m_sensor->getTempC(insideThermometer);
  m_isTemperatureCorrect = (tempC != DEVICE_DISCONNECTED_C);
  if (m_isTemperatureCorrect) {
    m_tempC = tempC;
  }
  return m_tempC;
}

float TemperatureSensor::requestTemperatureF()
{
  return DallasTemperature::toFahrenheit( this->requestTemperatureC() );
}

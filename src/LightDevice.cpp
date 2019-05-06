#include "LightDevice.h"

LightDevice::LightDevice(const uint8_t pin, const uint16_t num_leds) :
  m_pin(pin),
  m_num_leds(num_leds)
{
  m_leds = new CRGB[m_num_leds];
  //FastLED.addLeds<LED_TYPE, D0, COLOR_ORDER>(m_leds, m_num_leds).setCorrection(TypicalLEDStrip);
  switch(pin) {
    case D0:
      FastLED.addLeds<LED_TYPE, D0, COLOR_ORDER>(m_leds, m_num_leds);
      break;
    case D1:
      FastLED.addLeds<LED_TYPE, D1, COLOR_ORDER>(m_leds, m_num_leds);
      break;
    case D2:
      FastLED.addLeds<LED_TYPE, D2, COLOR_ORDER>(m_leds, m_num_leds);
      break;
    case D3:
      FastLED.addLeds<LED_TYPE, D3, COLOR_ORDER>(m_leds, m_num_leds);
      break;
    case D4:
      FastLED.addLeds<LED_TYPE, D4, COLOR_ORDER>(m_leds, m_num_leds);
      break;
    case D5:
      FastLED.addLeds<LED_TYPE, D5, COLOR_ORDER>(m_leds, m_num_leds);
      break;
    case D6:
      FastLED.addLeds<LED_TYPE, D6, COLOR_ORDER>(m_leds, m_num_leds);
      break;
    case D7:
      FastLED.addLeds<LED_TYPE, D7, COLOR_ORDER>(m_leds, m_num_leds);
      break;
    case D8:
      FastLED.addLeds<LED_TYPE, D8, COLOR_ORDER>(m_leds, m_num_leds);
      break;
  };
  FastLED.clear();
  FastLED.show();

  m_chaosEffectHelper = new ChaosEffectHelper[num_leds];
}


LightDevice::~LightDevice()
{
  delete [] m_leds;
  delete [] m_chaosEffectHelper;
}


void LightDevice::setColor() const
{
  byte realRed = map(m_red, 0, 255, 0, m_brightness);
  byte realGreen = map(m_green, 0, 255, 0, m_brightness);
  byte realBlue = map(m_blue, 0, 255, 0, m_brightness);
  fill_solid( m_leds, m_num_leds, CRGB(realRed, realGreen, realBlue) );
}


void LightDevice::updateDevice()
{
  Serial.printf("LightDevice: Update state. Transition: %d\n", m_transition);

  if (m_transition != 0) {
    m_current_brightness = m_state ? 0 : m_brightness;
    m_inTransition = true;
    return;
  }

  if (m_state) {
    Serial.printf("LightDevice: red:[%d], green:[%d], blue:[%d]\n", m_red, m_green, m_blue);
    Serial.printf("LightDevice: brightness:[%d]\n", m_brightness);

    if ( (m_effect != "") && (m_lightEffectsList.find(m_effect) != m_lightEffectsList.end()) ) {
      FastLED.clear();
    } else {
      this->setColor();
    }
  } else {
    Serial.println("LightDevice: Switch the device off");
    m_effect = "";
    FastLED.clear();
  }
  FastLED.show();
}


void LightDevice::transitionEffect() {
  if (m_state) { // Transition to ON
    m_current_brightness = m_current_brightness + (50.0/1000.0)*(float)m_brightness/(float)m_transition;
    if ( m_current_brightness >= m_brightness) { // Stop transition
      m_transition = 0;
      m_current_brightness = m_brightness;
      m_inTransition = false;
    }
  } else { // Transition to OFF
    m_current_brightness = m_current_brightness - (50.0/1000.0)*(float)m_brightness/(float)m_transition;
     if ( m_current_brightness <= 0) { // Stop transition
      m_transition = 0;
      m_current_brightness = 0;
      m_inTransition = false;
    }
  }
  this->setColor();
  FastLED.delay(50);
}


void LightDevice::run()
{
  if ( m_inTransition ) {
    this->transitionEffect();
  } else {
    if (m_lightEffectsList.find(m_effect) != m_lightEffectsList.end()) {
      m_lightEffectsList[ m_effect ]();
    } else {
      m_effect = "";
    }
  }

  FastLED.show();
}


/* --------------------------------------------------------------- */

void LightDevice::efSparkles()
{
  byte randomPixel = random(0, m_num_leds);
  // With a probability 25% if a pixel is black, generate a new one.
  uint32_t pixelColor = (((uint32_t)m_leds[randomPixel].red << 16) | ((long)m_leds[randomPixel].green << 8 ) | (long)m_leds[randomPixel].blue);
  if ( (pixelColor == 0) && (random(0,3) == 0) ) {
    m_leds[randomPixel] = CHSV(random(0, 255), 255, random(150, 255) );
  }
  for (byte j=0; j<m_num_leds; j++) {
    m_leds[j].fadeToBlackBy(1);
  }
  FastLED.delay(50);
}

void LightDevice::efColorLoop()
{
  fill_solid( m_leds, m_num_leds, CHSV( ef_hue, 255, m_brightness) );
  ef_hue += 1;
  FastLED.delay(500); //50
}

void LightDevice::efChaos()
{
  for (int i = 0; i < m_num_leds; i++) {
    if ( (m_chaosEffectHelper[i].up == false) && (m_chaosEffectHelper[i].brightness == 0) ) {
      m_chaosEffectHelper[i].hue = random(0, 255);
      m_chaosEffectHelper[i].saturation = 255;
      m_chaosEffectHelper[i].brightness = 0;
      m_chaosEffectHelper[i].maxBrightness = random(100, 255);
      m_chaosEffectHelper[i].up = true;
    } else if ((m_chaosEffectHelper[i].up == true) && (m_chaosEffectHelper[i].brightness >= 0)) {
      m_chaosEffectHelper[i].brightness = m_chaosEffectHelper[i].brightness + 1;
      m_chaosEffectHelper[i].up = m_chaosEffectHelper[i].brightness < m_chaosEffectHelper[i].maxBrightness;
    } else if ((m_chaosEffectHelper[i].up == false) && (m_chaosEffectHelper[i].brightness > 0)) {
      m_chaosEffectHelper[i].brightness = m_chaosEffectHelper[i].brightness - 1;
    }
  }
  for (byte j=0; j<m_num_leds; j++) {
    m_leds[j] = CHSV(m_chaosEffectHelper[j].hue, m_chaosEffectHelper[j].saturation, m_chaosEffectHelper[j].brightness );
  }
  FastLED.delay(20);
}

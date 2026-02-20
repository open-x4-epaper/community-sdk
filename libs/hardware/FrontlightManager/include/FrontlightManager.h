#pragma once

#include <Arduino.h>

class FrontlightManager {
public:
  FrontlightManager();

  // Initialize pins and start the control task
  void begin();

  /**
   * Set overall brightness (0-100%).
   * 0 = Off
   * 100 = Full ON (PWM duty cycle 100%)
   */
  void setBrightness(uint8_t percentage);

  /**
   * Set Color Temperature (0-100%).
   * 0 = All Cool
   * 100 = All Warm
   * Values in between mix the time segments.
   */
  void setColorTemperature(uint8_t warmPercentage);

  // Run the safety test sequence. Returns true if circuit is closed/valid.
  bool testCircuit();

  // The task function needs to be static or friend to be called by xTaskCreate
  static void frontlightTask(void *param);

private:
  // Pin Definitions
  static constexpr int PIN_LED_PWR = 17; // Active LOW (supplies 3.3V)
  static constexpr int PIN_WARM = 6;     // Gate for Warm LED
  static constexpr int PIN_COOL = 7;     // Gate for Cool LED
  static constexpr int PIN_PWM = 5;      // SHDN pin (Active HIGH for ON)
  static constexpr int PIN_SENSE = 18;   // Active LOW (senses feedback)

  // State
  volatile uint8_t brightness; // 0-100
  volatile uint8_t colorTemp;  // 0-100 (0=Cool, 100=Warm)
  volatile bool isEnabled;

  TaskHandle_t taskHandle;
};

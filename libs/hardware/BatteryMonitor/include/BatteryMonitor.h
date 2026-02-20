#pragma once
#include <cstdint>

class BatteryMonitor {
 public:
  // Optional divider multiplier parameter defaults to 2.0
  // statusPin: Pin connected to MCP73832 STAT pin (Open Drain). -1 if unused.
  explicit BatteryMonitor(uint8_t adcPin = 4, float dividerMultiplier = 2.0f, int8_t statusPin = 8);

  // Read voltage and return percentage (0-100)
  uint16_t readPercentage() const;

  // Read the battery voltage in millivolts (accounts for divider)
  uint16_t readMillivolts() const;

  // Read raw millivolts from ADC (doesn't account for divider)
  uint16_t readRawMillivolts() const;

  // Read the battery voltage in volts (accounts for divider)
  double readVolts() const;

  // Check if charging (Status Pin LOW = Charging/Done, HIGH-Z = Standby)
  // Assuming pullup on pin, LOW means charging/complete.
  bool isCharging() const;

  // Percentage (0-100) from a millivolt value
  static uint16_t percentageFromMillivolts(uint16_t millivolts);

  // Calibrate a raw ADC reading and return millivolts
  static uint16_t millivoltsFromRawAdc(uint16_t adc_raw);

 private:
  uint8_t _adcPin;
  float _dividerMultiplier;
  int8_t _statusPin;
};

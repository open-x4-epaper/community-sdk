#include "BatteryMonitor.h"
#include <Arduino.h>

inline float min(const float a, const float b) { return a < b ? a : b; }
inline float max(const float a, const float b) { return a > b ? a : b; }

BatteryMonitor::BatteryMonitor(uint8_t adcPin, float dividerMultiplier)
  : _adcPin(adcPin), _dividerMultiplier(dividerMultiplier)
{
}

uint16_t BatteryMonitor::readPercentage() const
{
    return percentageFromMillivolts(readMillivolts());
}

uint16_t BatteryMonitor::readMillivolts() const
{
    const uint16_t raw = readRawMillivolts();
    return static_cast<uint16_t>(raw * _dividerMultiplier);
}

uint16_t BatteryMonitor::readRawMillivolts() const
{
    return analogReadMilliVolts(_adcPin);
}

double BatteryMonitor::readVolts() const
{
    return static_cast<double>(readMillivolts()) / 1000.0;
}

uint16_t BatteryMonitor::percentageFromMillivolts(uint16_t millivolts)
{
    double volts = millivolts / 1000.0;
    // Polynomial derived from LiPo samples
    double y = -144.9390 * volts * volts * volts +
               1655.8629 * volts * volts -
               6158.8520 * volts +
               7501.3202;

    // Clamp to [0,100] and round
    y = max(y, 0.0);
    y = min(y, 100.0);
    y = round(y);
    return static_cast<int>(y);
}

uint16_t BatteryMonitor::millivoltsFromRawAdc(uint16_t adc_raw)
{
    (void)adc_raw;
    // In Arduino Core 3.x, use analogReadMilliVolts() directly instead
    // of manual ADC calibration. This function is kept for API compatibility.
    return 0;
}

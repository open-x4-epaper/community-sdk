#include "FrontlightManager.h"
#include <Arduino.h>

FrontlightManager::FrontlightManager()
    : brightness(0), colorTemp(50), isEnabled(false), taskHandle(NULL) {}

void FrontlightManager::begin() {
  pinMode(PIN_LED_PWR, OUTPUT);
  pinMode(PIN_WARM, OUTPUT);
  pinMode(PIN_COOL, OUTPUT);
  pinMode(PIN_PWM, OUTPUT); // SHDN
  pinMode(PIN_SENSE, INPUT_PULLUP);

  // Default state: Off
  digitalWrite(PIN_LED_PWR, HIGH); // OFF (Active Low)
  digitalWrite(PIN_WARM, LOW);
  digitalWrite(PIN_COOL, LOW);
  digitalWrite(PIN_PWM, LOW); // Shutdown

  // Perform self-test on boot? Or user manually calls it?
  // Let's print a message that we are ready.
  Serial.println("[Frontlight] Initialized (OFF)");

  if (testCircuit()) {
    Serial.println("[Frontlight] Circuit Test PASSED");
  } else {
    Serial.println("[Frontlight] Circuit Test FAILED (or no load)");
  }

  // Create the high-priority task for 100Hz Bit-Banging
  // 100Hz = 10ms period.
  // We need reasonably good timing, so priority should be high.
  // Using minimal stack size since logic is simple.
  xTaskCreatePinnedToCore(FrontlightManager::frontlightTask, "FrontlightTask",
                          2048, this,
                          5, // Priority (Higher than Display/Input)
                          &taskHandle,
                          1 // Core 1 (App Core)
  );
}

bool FrontlightManager::testCircuit() {
  Serial.println("[Frontlight] Testing Circuit...");

  // Sequence:
  // 1. Enable Main Power
  digitalWrite(PIN_LED_PWR, LOW); // ON
  delayMicroseconds(100);

  // 2. Enable One toggle (Warm)
  digitalWrite(PIN_WARM, HIGH);
  digitalWrite(PIN_COOL, LOW);
  delayMicroseconds(50);

  // 3. Enable Driver
  digitalWrite(PIN_PWM, HIGH);
  delayMicroseconds(600); // Wait for current to establish (Diagnostic: ~486us)

  // 4. Read Sense
  bool passed = (digitalRead(PIN_SENSE) == LOW); // LOW means Valid Feedback

  // 5. Cleanup
  digitalWrite(PIN_PWM, LOW);
  digitalWrite(PIN_WARM, LOW);
  digitalWrite(PIN_COOL, LOW);
  digitalWrite(PIN_LED_PWR, HIGH); // OFF

  return passed;
}

void FrontlightManager::setBrightness(uint8_t p) {
  if (p > 100)
    p = 100;
  brightness = p;
}

void FrontlightManager::setColorTemperature(uint8_t p) {
  if (p > 100)
    p = 100;
  colorTemp = p;
}

void FrontlightManager::frontlightTask(void *param) {
  FrontlightManager *self = (FrontlightManager *)param;

  const unsigned long periodMicros = 10000; // 100Hz = 10ms = 10,000us

  for (;;) {
    unsigned long startFrame = micros();

    // Capture state atomically-ish
    int b = self->brightness;
    int t = self->colorTemp;

    if (b == 0) {
      // OFF State
      digitalWrite(PIN_LED_PWR, HIGH); // Disable Power
      digitalWrite(PIN_PWM, LOW);
      digitalWrite(PIN_WARM, LOW);
      digitalWrite(PIN_COOL, LOW);

      // Sleep until next frame
      unsigned long now = micros();
      long delayUs = periodMicros - (now - startFrame);
      if (delayUs > 0) {
        vTaskDelay(pdMS_TO_TICKS(delayUs / 1000));
        // Better to just wait for next tick if off
      } else {
        vTaskDelay(1);
      }
      continue;
    }

    // ON State
    digitalWrite(PIN_LED_PWR, LOW); // Enable Power
    // Note: WarmUp required? AP3012 is fast.

    // Calculate Timings
    // Total ON time = (brightness / 100.0) * periodMicros
    // Warm Portion = (t / 100.0) * Total ON time
    // Cool Portion = Total ON time - Warm Portion

    unsigned long totalOnTime = (b * periodMicros) / 100;
    unsigned long warmTime = (t * totalOnTime) / 100;
    unsigned long coolTime = totalOnTime - warmTime;
    unsigned long offTime = periodMicros - totalOnTime;

    // Safety: Make-Before-Break logic
    // We want to ensure at least one toggle is valid if PWM is ON.

    // Sequence Execution

    // PHASE 1: WARM
    if (warmTime > 0) {
      // Ensure Warm is ON, Cool is OFF
      digitalWrite(PIN_WARM, HIGH);
      digitalWrite(PIN_COOL,
                   LOW); // Break Cool? Or maybe keep it? Cleanest is separate.
      // Actually, if we want to avoid "both off", we set new first then clear
      // old. But here we are starting fresh frame (usually from Off).

      digitalWrite(PIN_PWM, HIGH); // Driver ON

      // Busy wait for precision
      unsigned long s = micros();
      while (micros() - s < warmTime)
        ;
    }

    // PHASE 2: COOL
    // If we transition from Warm to Cool, we must handle the transition
    // carefully.
    if (coolTime > 0) {
      // Turn Cool ON FIRST
      digitalWrite(PIN_COOL, HIGH);
      // Then Turn Warm OFF (if it was on)
      if (warmTime > 0) {
        digitalWrite(PIN_WARM, LOW);
      }

      // Ensure Driver is ON (if we skipped warm phase)
      digitalWrite(PIN_PWM, HIGH);

      unsigned long s = micros();
      while (micros() - s < coolTime)
        ;
    }

    // PHASE 3: OFF
    digitalWrite(PIN_PWM, LOW); // Driver OFF
    // Toggles can be anything, but safe to clear
    digitalWrite(PIN_WARM, LOW);
    digitalWrite(PIN_COOL, LOW);

    unsigned long now = micros();
    unsigned long elapsed = now - startFrame;
    if (elapsed < periodMicros) {
      unsigned long rem = periodMicros - elapsed;
      // If remaining time is large (>2ms), yield to RTOS
      if (rem > 2000) {
        vTaskDelay(pdMS_TO_TICKS(rem / 1000));
      } else {
        // Busy wait the rest to maintain 100Hz stability
        while ((micros() - startFrame) < periodMicros)
          ;
      }
    }
  }
}

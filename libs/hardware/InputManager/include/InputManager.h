#pragma once

#include <Arduino.h>

class InputManager {
 public:
  InputManager();
  void begin();
  uint16_t getState();

  /**
   * Updates the button states. Should be called regularly in the main loop.
   */
  void update();

  /**
   * Returns true if the button was being held at the time of the last #update()
   * call.
   *
   * @param buttonIndex the button indexes
   * @return the button current press state
   */
  bool isPressed(uint8_t buttonIndex) const;

  /**
   * Returns true if the button went from unpressed to pressed between the last
   * two #update() calls.
   *
   * This differs from #isPressed() in that pressing and holding a button will
   * cause this function to return true after the first #update() call, but
   * false on subsequent calls, whereas #isPressed() will continue to return
   * true.
   *
   * @param buttonIndex
   * @return the button pressed state
   */
  bool wasPressed(uint8_t buttonIndex) const;

  /**
   * Returns true if any button started being pressed between the last two
   * #update() calls
   *
   * @return true if any button started being pressed between the last two
   * #update() calls
   */
  bool wasAnyPressed() const;

  /**
   * Returns true if the button went from pressed to unpressed between the last
   * two #update() calls
   *
   * @param buttonIndex the button indexes
   * @return the button release state
   */
  bool wasReleased(uint8_t buttonIndex) const;

  /**
   * Returns true if any button was released between the last two #update()
   * calls
   *
   * @return  true if any button was released between the last two #update()
   * calls
   */
  bool wasAnyReleased() const;

  /**
   * Returns the time between any button starting to be depressed and all
   * buttons between released
   *
   * @return duration in milliseconds
   */
  unsigned long getHeldTime() const;

  // Button indices
  static constexpr uint8_t BTN_BACK = 0;
  static constexpr uint8_t BTN_CONFIRM = 1;
  static constexpr uint8_t BTN_LEFT = 2;
  static constexpr uint8_t BTN_RIGHT = 3;
  static constexpr uint8_t BTN_UP = 4;
  static constexpr uint8_t BTN_DOWN = 5;
  static constexpr uint8_t BTN_UNKNOWN_1 = 6;
  static constexpr uint8_t BTN_UNKNOWN_2 = 7;
  static constexpr uint8_t BTN_POWER = 8;

  // Pins
  static constexpr int BUTTON_ADC_PIN_1 = 1;
  static constexpr int BUTTON_ADC_PIN_2 = 2;
  static constexpr int POWER_BUTTON_PIN = 3;

  // Power button methods
  bool isPowerButtonPressed() const;

  // Button names
  static const char* getButtonName(uint8_t buttonIndex);

 private:
  struct ButtonMap {
    int id;
    int min;
    int max;
  };

  int getButtonFromMap(int adcValue, const ButtonMap* map, int numButtons);

  uint16_t currentState;
  uint16_t lastState;
  uint16_t pressedEvents;
  uint16_t releasedEvents;
  unsigned long lastDebounceTime;
  unsigned long buttonPressStart;
  unsigned long buttonPressFinish;

  static constexpr int NUM_BUTTONS_1 = 4;
  static const ButtonMap MAP_1[];

  static constexpr int NUM_BUTTONS_2 = 4;
  static const ButtonMap MAP_2[];

  static constexpr int ADC_NO_BUTTON = 3800;  // Legacy / Unused in new map logic
  static constexpr unsigned long DEBOUNCE_DELAY = 5;

  static const char* BUTTON_NAMES[];
};

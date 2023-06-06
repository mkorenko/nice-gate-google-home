#include <WiFi.h>

#include <ArduinoOTA.h>
#include <string>

#include "src/esp-firebase-gh/esp-firebase-gh.h"

#include "credentials.h"

#define SCA_PIN 1
#define CLOSE_PIN 2
#define OPEN_PIN 3

std::string machine_local_ip;
uint8_t current_state = 0;
// 0 - closed
// 1 - opening
// 2 - opened
// 3 - closing

void setup() {
  pinMode(SCA_PIN, INPUT);
  pinMode(CLOSE_PIN, OUTPUT);
  pinMode(OPEN_PIN, OUTPUT);

  wifi_setup();
  gh_setup();
}

uint8_t _prev_state = 4;
bool _prev_opening = false;
inline void set_state(uint8_t state) {
  if (_prev_state == state) {
    return;
  }

  if (state == 1 && !_prev_opening) {
    _prev_opening = true;
    return;
  }
  _prev_opening = false;

  _prev_state = state;
  current_state = state;
  gh_report_device_state();
}

uint8_t _prev_sca_state = 3;
uint8_t sca_ticks = 0;
// uint8_t last_sca_ticks = 0;
bool _measuring_sca = false;
unsigned long _measuring_sca_started = 0;

void sca_loop() {
  uint8_t sca_state = digitalRead(SCA_PIN);
  unsigned long _millis = millis();

  if (_prev_sca_state != sca_state) {
    _prev_sca_state = sca_state;

    if (!_measuring_sca) {
      _measuring_sca = true;
      _measuring_sca_started = _millis;
    }

    sca_ticks++;
  }

  uint8_t gate_state;
  if (_measuring_sca && (_millis - _measuring_sca_started > 600)) {
    // last_sca_ticks = sca_ticks;
    if (sca_ticks == 2) {
      set_state(1);
    } else if (sca_ticks > 2) {
      set_state(3);
    }

    sca_ticks = 0;
    _measuring_sca = false;
  }

  if (!_measuring_sca && (_millis - _measuring_sca_started > 1500)) {
    set_state(sca_state ? 2 : 0);
  }
}

void loop() {
  gh_loop();
  sca_loop();
  wifi_loop();
}

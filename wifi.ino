inline void wifi_setup() {
  WiFi.persistent(false);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);

  uint8_t wifi_tries = 10;

  while (WiFi.status() != WL_CONNECTED) {
    if (wifi_tries-- == 0) {
      ESP.restart();
    }

    delay(1000);
  }

  // WiFi.setAutoReconnect(true);

  machine_local_ip = std::string(WiFi.localIP().toString().c_str());

  ArduinoOTA.setPassword(ota_password);
  ArduinoOTA.begin();

  _last_activity_at = millis();
}

inline void wifi_loop() {
  if (WiFi.status() != WL_CONNECTED) {
    ESP.restart();
  }
  if (millis() - _last_activity_at > 1800000) {
    ESP.restart();
  }
  ArduinoOTA.handle();
}

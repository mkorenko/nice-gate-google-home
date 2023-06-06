FirebaseEspGh firebase_esp_gh;

void gh_on_device_state_request(
    FirebaseJson *gh_state,
    FirebaseJson *gh_notifications,
    FirebaseJson *custom_state
) {
  // gh state
  gh_state->add(
      "openPercent",
      (current_state == 0 || current_state == 3) ? 0 : 100
  );

  FirebaseJson current_run_cycle_en;
  current_run_cycle_en.add(
      "currentCycle",
      _gh_get_cycle_label(current_state)
  );
  current_run_cycle_en.add("lang", "en");
  FirebaseJsonArray current_run_cycle;
  current_run_cycle.add(current_run_cycle_en);
  gh_state->add("currentRunCycle", current_run_cycle);

  // custom state
  custom_state->add("local_ip", machine_local_ip);
  custom_state->add("current_state", current_state);
  // custom_state->add("last_sca_ticks", last_sca_ticks);
  custom_state->set("timestamp/.sv", "timestamp");
}

void gh_on_command(
  FirebaseJson *result,
  std::string &command,
  FirebaseJson &params
) {
  if (command == "action.devices.commands.OpenClose") {
    return _gh_on_openclose(result, params);
    return;
  }

  result->add("error_code", "hardError");
}

inline void _gh_on_openclose(FirebaseJson *result, FirebaseJson &params) {
  FirebaseJsonData _open_percent;
  params.get(_open_percent, "openPercent");
  if (!_open_percent.success) {
    result->add("error_code", "hardError");
    return;
  }
  uint8_t open_percent = _open_percent.to<uint8_t>();

  if (open_percent == 0) {
    if (current_state == 0) {
      result->add("error_code", "alreadyClosed");
      return;
    }
    if (current_state == 3) {
      result->add("error_code", "alreadyInState");
      return;
    }
    // close
    digitalWrite(CLOSE_PIN, HIGH);
    delay(200);
    digitalWrite(CLOSE_PIN, LOW);
    result->add("openPercent", 0);
  } else {
    if (current_state == 1) {
      result->add("error_code", "alreadyInState");
      return;
    }
    // open
    digitalWrite(OPEN_PIN, HIGH);
    delay(200);
    digitalWrite(OPEN_PIN, LOW);
    result->add("openPercent", 100);
  }
}

inline void gh_report_device_state() {
  firebase_esp_gh.report_device_state();
}

inline void gh_setup() {
  firebase_esp_gh.begin(
      firebase_api_key,
      firebase_user_email,
      firebase_user_password,
      firebase_db_url,
      firebase_device_id,
      gh_on_command,
      gh_on_device_state_request
  );
}

inline void gh_loop() {
  firebase_esp_gh.loop();
}

inline std::string _gh_get_cycle_label(uint8_t &current_state) {
  if (current_state == 0) {
    return "Closed";
  }
  if (current_state == 1) {
    return "Opening";
  }
  if (current_state == 2) {
    return "Opened";
  }
  if (current_state == 3) {
    return "Closing";
  }
  return "Unknown";
}

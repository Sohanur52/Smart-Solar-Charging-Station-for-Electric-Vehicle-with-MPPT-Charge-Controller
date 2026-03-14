void load() {
  // Read button state for local control
  pinValue1 = digitalRead(buttonSelect);  // Using select button as local load control

  // Debounce and cycle through load modes
  if (pinValue1 == HIGH) {
    delay(30);
    if (pinValue1 == HIGH) {
      dem++;
      if (dem > 2) dem = 0;
      saveSettings();  // Save the current mode
    }
  }

  // Apply load control based on mode
  if (dem == 0) {
    digitalWrite(Load, LOW);  // Mode 0: Always OFF
  } else if (dem == 1) {
    digitalWrite(Load, HIGH); // Mode 1: Always ON
  } else if (dem == 2) {
    // Mode 2: Remote control via MQTT/web
    if (pinValue2 == 1) {
      digitalWrite(Load, HIGH);
    } else {
      digitalWrite(Load, LOW);
    }
  }
}
#include <HTTPClient.h>

// Simple remote access handler
void Remote_Access_Setup() {
  Serial.println("Remote Access: Web server is primary method");
  
  // Add additional status endpoint
  server.on("/status", HTTP_GET, []() {
    String status = "System Status:\n";
    status += "Solar: " + String(voltageInput, 1) + "V\n";
    status += "Battery: " + String(voltageOutput, 2) + "V\n";
    status += "SOC: " + String(batteryPercent) + "%\n";
    status += "Load: " + String(digitalRead(Load) ? "ON" : "OFF");
    server.send(200, "text/plain", status);
  });
}

void Remote_Access_Loop() {
  // Keep empty - all handled by web server
}

// Simple status for external access
void handleRemoteStatus() {
  String json = "{\"vi\":" + String(voltageInput,1) + 
                ",\"vo\":" + String(voltageOutput,2) + 
                ",\"soc\":" + String(batteryPercent) + 
                ",\"temp\":" + String(temperature,1) + 
                ",\"load\":" + String(digitalRead(Load)) + 
                ",\"ip\":\"" + WiFi.localIP().toString() + "\"}";
  server.send(200, "application/json", json);
}
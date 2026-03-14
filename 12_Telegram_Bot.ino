//=================================== TELEGRAM BOT ===================================//
// This file handles all Telegram bot functionality for remote control and monitoring //
// FULLY PUBLIC - NO AUTHENTICATION REQUIRED                                         //
// WORKING VERSION - No underlines in commands                                       //
//====================================================================================//

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// ==================== TELEGRAM CONFIGURATION ====================
#define BOT_TOKEN "8246771785:AAHLrFLzBxZsz5QTMa09eCOH-Zoeh0qOq_0"

const unsigned long BOT_MTBS = 1000; // Check for messages every 1 second
unsigned long bot_lasttime = 0;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// ==================== EXTERNAL VARIABLES ====================
// All variables from main file that Telegram bot needs
extern float voltageInput;
extern float voltageOutput;
extern float currentInput;
extern float currentOutput;
extern float powerInputAdjusted;  // This matches your web dashboard Solar Power box!
extern int batteryPercent;
extern int temperature;
extern int PWM;
extern float Wh;
extern int pinValue2;
extern int dem;
extern float electricalPrice;
extern int temperatureFan;

// Function prototype
extern void saveSettings();

// Pin definition (make sure this matches your main file)
#define Load 12

// ==================== FORWARD DECLARATIONS ====================
String getSystemStatus();
String getHelpMessage();

// ==================== INITIALIZATION ====================
void initTelegramBot() {
  Serial.println("Initializing Telegram Bot...");
  
  secured_client.setInsecure();
  
  // Don't send welcome message on startup - wait for /start
  
  Serial.println("✅ Telegram Bot initialized successfully!");
}

// ==================== MESSAGE HANDLER ====================
void checkTelegramBot() {
  if (millis() - bot_lasttime > BOT_MTBS) {
    bot_lasttime = millis();
    
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    
    while (numNewMessages) {
      Serial.println("📨 Got response");
      
      for (int i = 0; i < numNewMessages; i++) {
        String chat_id = String(bot.messages[i].chat_id);
        String text = bot.messages[i].text;
        String from_name = bot.messages[i].from_name;
        
        if (from_name == "") from_name = "Guest";
        
        Serial.println("From: " + from_name);
        Serial.println("Chat ID: " + chat_id);
        Serial.println("Message: " + text);
        
        // Process commands - NO UNDERSCORES IN COMMAND NAMES
        if (text == "/start") {
          String welcome = "👋 Welcome " + from_name + " to MPPT Solar Monitor!\n\n";
          welcome += getHelpMessage();
          bot.sendMessage(chat_id, welcome, "");
        }
        else if (text == "/help") {
          bot.sendMessage(chat_id, getHelpMessage(), "");
        }
        else if (text == "/status") {  // NO UNDERLINE HERE
          bot.sendMessage(chat_id, getSystemStatus(), "Markdown");
        }
        else if (text == "/solar") {
          String solarMsg = "☀️ *Solar Power Status*\n\n";
          solarMsg += "Solar Power: `" + String(powerInputAdjusted, 1) + " W` (includes ESP32)\n";
          solarMsg += "Solar Voltage: `" + String(voltageInput, 1) + " V`\n";
          solarMsg += "Solar Current: `" + String(currentInput, 2) + " A`";
          bot.sendMessage(chat_id, solarMsg, "Markdown");
        }
        else if (text == "/battery") {
          String batteryMsg = "🔋 *Battery Status*\n\n";
          batteryMsg += "Battery Voltage: `" + String(voltageOutput, 2) + " V`\n";
          batteryMsg += "Battery Current: `" + String(currentOutput, 2) + " A`\n";
          batteryMsg += "Battery Power: `" + String(voltageOutput * currentOutput, 1) + " W`\n";
          batteryMsg += "SOC: `" + String(batteryPercent) + "%`";
          bot.sendMessage(chat_id, batteryMsg, "Markdown");
        }
        else if (text == "/ip") {
          String ipMsg = "🌐 *Network Information*\n\n";
          ipMsg += "Local IP: `" + WiFi.localIP().toString() + "`\n";
          ipMsg += "Hotspot IP: `192.168.4.1`\n";
          ipMsg += "SSID: `" + WiFi.SSID() + "`\n";
          ipMsg += "Signal: " + String(WiFi.RSSI()) + " dBm";
          bot.sendMessage(chat_id, ipMsg, "Markdown");
        }
        else if (text == "/loadon") {
          pinValue2 = 1;
          dem = 2;
          bot.sendMessage(chat_id, "✅ *Load turned ON*", "Markdown");
          Serial.println("Load turned ON via Telegram");
        }
        else if (text == "/loadoff") {
          pinValue2 = 0;
          dem = 2;
          bot.sendMessage(chat_id, "✅ *Load turned OFF*", "Markdown");
          Serial.println("Load turned OFF via Telegram");
        }
        else if (text == "/stats") {
          float batteryPower = voltageOutput * currentOutput;
          float efficiency = 0.0;
          if (powerInputAdjusted > 0) {
            efficiency = (batteryPower / powerInputAdjusted) * 100.0;
          }
          
          String stats = "📈 *Detailed Statistics*\n\n";
          stats += "☀️ *Solar:* `" + String(powerInputAdjusted, 1) + " W`\n";
          stats += "🔋 *Battery:* `" + String(batteryPower, 1) + " W`\n";
          stats += "⚡ *Efficiency:* `" + String(efficiency, 1) + "%`\n";
          stats += "📊 *Energy Used:* `" + String(Wh, 1) + " Wh`\n";
          stats += "💰 *Saved:* `" + String(electricalPrice * (Wh/1000.0), 2) + " BDT`\n";
          stats += "🌡️ *Temperature:* `" + String(temperature, 1) + "°C`\n";
          stats += "💡 *Load:* `" + String(digitalRead(Load) ? "ON" : "OFF") + "`";
          bot.sendMessage(chat_id, stats, "Markdown");
        }
        else if (text.startsWith("/settemp")) {
          int spaceIndex = text.indexOf(' ');
          if (spaceIndex > 0) {
            int newTemp = text.substring(spaceIndex + 1).toInt();
            if (newTemp >= 30 && newTemp <= 90) {
              temperatureFan = newTemp;
              saveSettings();
              bot.sendMessage(chat_id, "✅ Fan trigger temperature set to " + String(newTemp) + "°C", "");
            } else {
              bot.sendMessage(chat_id, "❌ Temperature must be between 30-90°C", "");
            }
          } else {
            bot.sendMessage(chat_id, "Usage: /settemp <value>\nExample: /settemp 65", "");
          }
        }
        else {
          String unknownMsg = "❌ Unknown command. Send /help to see available commands.";
          bot.sendMessage(chat_id, unknownMsg, "");
        }
      }
      
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
  }
}

// ==================== HELPER FUNCTIONS ====================
String getSystemStatus() {
  float batteryPower = voltageOutput * currentOutput;
  float efficiency = 0.0;
  
  // Use powerInputAdjusted to match web dashboard
  if (powerInputAdjusted > 0) {
    efficiency = (batteryPower / powerInputAdjusted) * 100.0;
  }
  
  String status = "⚡ *MPPT System Status*\n";
  status += "═══════════════════\n";
  status += "☀️ *Solar Power:* `" + String(powerInputAdjusted, 1) + " W` (includes ESP32)\n";
  status += "   └─ Voltage: `" + String(voltageInput, 1) + " V`\n";
  status += "   └─ Current: `" + String(currentInput, 2) + " A`\n\n";
  
  status += "🔋 *Battery:* `" + String(batteryPower, 1) + " W`\n";
  status += "   └─ Voltage: `" + String(voltageOutput, 2) + " V`\n";
  status += "   └─ Current: `" + String(currentOutput, 2) + " A`\n";
  status += "   └─ SOC: `" + String(batteryPercent) + "%`\n\n";
  
  status += "🌡️ *Temperature:* `" + String(temperature, 1) + "°C`\n";
  status += "📊 *Efficiency:* `" + String(efficiency, 1) + "%`\n";
  status += "💡 *Load:* `" + String(digitalRead(Load) ? "ON" : "OFF") + "`\n";
  status += "📈 *Energy Used:* `" + String(Wh, 1) + " Wh`";
  
  return status;
}

String getHelpMessage() {
  String help = "📋 *Available Commands*\n\n";
  help += "🔹 `/status` - Complete system status\n";
  help += "🔹 `/solar` - Solar power only\n";
  help += "🔹 `/battery` - Battery status only\n";
  help += "🔹 `/stats` - Quick statistics\n";
  help += "🔹 `/ip` - Network information\n";
  help += "🔹 `/loadon` - Turn load ON\n";
  help += "🔹 `/loadoff` - Turn load OFF\n";
  help += "🔹 `/settemp <value>` - Set fan temp (30-90°C)\n";
  help += "🔹 `/help` - Show this message\n\n";
  help += "Example: `/settemp 65`";
  return help;
}
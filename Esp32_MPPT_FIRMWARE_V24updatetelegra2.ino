//================================ MPPT FIRMWARE LCD MENU INFO =====================================//
// The lines below are for the Firmware Version info displayed on the MPPT's LCD Menu Interface     //
//==================================================================================================//

int pinValue2 = 0; 

String 
firmwareInfo      = "V24  ",
firmwareDate      = "16/02/2026",
firmwareContactR1 = "Esp32",  
firmwareContactR2 = "Smart Mppt    ";        
           
//====================== ARDUINO LIBRARIES (ESP32 Compatible Libraries) ============================//
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <LiquidCrystal_I2C.h>
#include <WebServer.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <WiFiManager.h>
#include <TimeLib.h>


Adafruit_ADS1115 ads;  // ADC object
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD object
TaskHandle_t Core2;    // Task handle for second core
WebServer server(80);  // Web server
WiFiManager wm;
WiFiClient client;



// Pin Definitions
#define backflow_MOSFET 32
#define buck_IN         15
#define buck_EN         2
#define LED             27
#define FAN             13
#define ADC_ALERT       34
#define TempSensor      33
#define Load            12
#define buttonLeft      16
#define buttonRight     17
#define buttonBack      5
#define buttonSelect    18
#define TRIGGER_PIN     0

// EEPROM Addresses
#define EE_WH_ADDR 200
#define EE_PRICE_ADDR 204
#define HISTORY_START_ADDR 100

// Data Logging Structures
// Data structure for each minute
typedef struct {
  float voltageInput;
  float currentInput;
  float powerInput;
  float voltageOutput;
  float currentOutput;
  float temperature;
  time_t timestamp;
} MinuteData;

// Structure for each hour (60 minutes)
typedef struct {
  MinuteData minutes[60];  // 60 minutes per hour
} HourData;

// Structure for daily data (24 hours)
typedef struct {
  HourData hours[24];      // 24 hours per day
  time_t date;             // Date of this data
  bool valid;              // Whether this day has valid data
} DailyData;

#define MAX_DAYS 1
DailyData historicalData[MAX_DAYS];

// Global Variables
bool wm_nonblocking = true;
int currentHour = -1;
time_t lastDataSave = 0;
const int SAVE_INTERVAL = 60;
float electricalPrice = 9.50;
String currentTime = "00:01:00";
String currentDate = "16/02/2026";

// User Parameters
bool ADS1015_Mode = 0;
bool MPPT_Mode = 1;
bool output_Mode = 1;
bool disableFlashAutoLoad = 0;
bool enablePPWM = 1;
bool enableWiFi = 1;
bool enableFan = 1;

bool enableLCD = 1;

bool overrideFan = 0;
bool enableDynamicCooling = 0;

int serialTelemMode = 1;
int pwmResolution = 11;
int pwmChannel = 0;
int pwmFrequency = 39000;
int temperatureFan = 60;
int temperatureMax = 90;
int telemCounterReset = 0;
int errorTimeLimit = 1000;
int errorCountLimit = 5;
int millisRoutineInterval = 250;
int millisSerialInterval = 1;
int millisLCDInterval = 1000;
int millisWiFiInterval = 2000;
int millisLCDBackLInterval = 2000;
int backlightSleepMode = 0;
int baudRate = 115200;

float voltageBatteryMax = 27.3000;
float voltageBatteryMin = 10.4000;
float currentCharging = 30.0000;
float esp32PowerConsumption = 4.0;  // ESP32 idle power consumption in Watts
// Calibration Parameters
int avgCountVS = 3;
int avgCountCS = 4;
int avgCountTS = 500;
int ADC_GainSelect = 2;

float inVoltageDivRatio = 20.75;
float outVoltageDivRatio = 21.25;
float vOutSystemMax = 50.0000;
float cOutSystemMax = 30.0000;
float ntcResistance = 8000.00;
float voltageDropout = 1.0000;
float voltageBatteryThresh = 1.5000;
float currentInAbsolute = 31.0000;
float currentOutAbsolute = 50.0000;
float PPWM_margin = 99.5000;
float PWM_MaxDC = 97.0000;

float currentMidPoint = 2.5250;
float currentSens = 0.0000;
float currentSensV = 0.0660;
float vInSystemMin = 10.000;

// System Parameters
bool buckEnable = 0;
bool fanStatus = 0;
bool bypassEnable = 0;
bool chargingPause = 0;
bool lowPowerMode = 0;
bool buttonRightStatus = 0;
bool buttonLeftStatus = 0;
bool buttonBackStatus = 0;
bool buttonSelectStatus = 0;
bool buttonRightCommand = 0;
bool buttonLeftCommand = 0;
bool buttonBackCommand = 0;
bool buttonSelectCommand = 0;
bool settingMode = 0;
bool setMenuPage = 0;
bool boolTemp = 0;
bool flashMemLoad = 0;
bool confirmationMenu = 0;
bool WIFI = 0;
bool BNC = 0;
bool REC = 0;
bool FLV = 0;
bool IUV = 0;
bool IOV = 0;
bool IOC = 0;
bool OUV = 0;
bool OOV = 0;
bool OOC = 0;
bool OTE = 0;

int inputSource = 0;
int avgStoreTS = 0;
int temperature = 0;
int sampleStoreTS = 0;
int pwmMax = 0;
int pwmMaxLimited = 0;
int PWM = 0;
int PPWM = 0;
int batteryPercent = 0;
int errorCount = 0;
int menuPage = 0;
int subMenuPage = 0;
int ERR = 0;

int intTemp = 0;
int dem = 0;
int pinValue1 = 0;

float VSI = 0.0000;
float VSO = 0.0000;
float CSI = 0.0000;
float CSI_converted = 0.0000;
float TS = 0.0000;
float powerInput = 0.0000;
float powerInputAdjusted = 0.0000; 
float currentInputAdjusted = 0.0000;  
float powerInputPrev = 0.0000;
float powerOutput = 0.0000;
float energySavings = 0.0000;
float voltageInput = 0.0000;
float voltageInputPrev = 0.0000;
float voltageOutput = 0.0000;
float currentInput = 0.0000;
float currentOutput = 0.0000;
float TSlog = 0.0000;
float ADC_BitReso = 0.0000;
float daysRunning = 0.0000;
float Wh = 0.0000;
float kWh = 0.0000;
float MWh = 0.0000;
float loopTime = 0.0000;
float outputDeviation = 0.0000;
float buckEfficiency = 0.0000;
float floatTemp = 0.0000;
float vOutSystemMin = 0.0000;

unsigned long currentErrorMillis = 0;
unsigned long currentButtonMillis = 0;
unsigned long currentSerialMillis = 0;
unsigned long currentRoutineMillis = 0;
unsigned long currentLCDMillis = 0;
unsigned long currentLCDBackLMillis = 0;
unsigned long currentWiFiMillis = 0;
unsigned long currentMenuSetMillis = 0;
unsigned long prevButtonMillis = 0;
unsigned long prevSerialMillis = 0;
unsigned long prevRoutineMillis = 0;
unsigned long prevErrorMillis = 0;
unsigned long prevWiFiMillis = 0;
unsigned long prevLCDMillis = 0;
unsigned long prevLCDBackLMillis = 0;
unsigned long timeOn = 0;
unsigned long loopTimeStart = 0;
unsigned long loopTimeEnd = 0;
unsigned long secondsElapsed = 0;

// Function Prototypes
void handleRoot();
void handleData();
void handleControl();
void handleSetPrice();
void handleGetPrice();
void handleSetTime();
void Wireless_Telemetry();
void ADC_SetGain();
void buck_Disable();
void initializeFlashAutoload();
void Read_Sensors();
void Device_Protection();
void System_Processes();
void Charging_Algorithm();
void Onboard_Telemetry();
void LCD_Menu();
void load();
void Remote_Access_Setup();
void Remote_Access_Loop();
void handleRemoteStatus();
void handleSetAlgorithm();
void handleSetMode();
void handleSetMaxVoltage();
void handleSetMinVoltage();
void handleSetFanTemp();
void handleSetMaxCurrent();
void handleFactoryReset();
void handleLocalIP();
void handleStartNgrok();
void handleHistoricalData();
void saveCurrentData();
void saveHistoricalData();
void loadHistoricalData();
void saveFloatToEEPROM(int addr, float value);
float readFloatFromEEPROM(int addr);
void loadPersistentData();
void saveAutoloadSettings();
int getCurrentHour();
time_t getCurrentDate();
DailyData* getDailyDataSlot(time_t date);
bool isWithinLoggingHours();
void clockDisplay();
void initTime();
void coreTwo(void * pvParameters);
// Telegram Bot Functions
void initTelegramBot();
void checkTelegramBot();


//============================================================================//
// Time Functions
//============================================================================//

int getCurrentHour() {
  time_t now = time(nullptr);
  struct tm *timeinfo = localtime(&now);
  return timeinfo->tm_hour;
}

bool isWithinLoggingHours() {
  return true;
}

time_t getCurrentDate() {
  time_t now = time(nullptr);
  struct tm *timeinfo = localtime(&now);
  timeinfo->tm_hour = 0;
  timeinfo->tm_min = 0;
  timeinfo->tm_sec = 0;
  return mktime(timeinfo);
}

DailyData* getDailyDataSlot(time_t date) {
  for (int i = 0; i < MAX_DAYS; i++) {
    if (historicalData[i].valid && historicalData[i].date == date) {
      return &historicalData[i];
    }
  }
  
  for (int i = 0; i < MAX_DAYS; i++) {
    if (!historicalData[i].valid) {
      historicalData[i].date = date;
      historicalData[i].valid = true;
      return &historicalData[i];
    }
  }
  
  for (int i = 0; i < MAX_DAYS - 1; i++) {
    historicalData[i] = historicalData[i + 1];
  }
  historicalData[MAX_DAYS - 1].date = date;
  historicalData[MAX_DAYS - 1].valid = true;
  return &historicalData[MAX_DAYS - 1];
}


void initTime() {
  // Don't set a fixed date - use compile time or NTP
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println();
  
  struct tm *timeinfo = localtime(&now);
  
  Serial.println(asctime(timeinfo));
}




void clockDisplay() {
  currentTime = String(hour()) + ":" + minute() + ":" + second();
  currentDate = String(day()) + "/" + month() + "/" + year();
}

//============================================================================//
// Setup
//============================================================================//

void setup() {
  Serial.begin(baudRate);
  Serial.println("\n Starting");
  Serial.println("> Serial Initialized");

  Serial.println("Trying to connect to WiFi router...");
  bool res = wm.autoConnect("ESP32_MPPT_AP");
  
  if(!res) {
      Serial.println("Failed to connect to WiFi router");
      Serial.println("Creating hotspot instead...");
      WiFi.mode(WIFI_AP);
      WiFi.softAP("ESP32_MPPT_PRO", "12345678");
      Serial.print("Hotspot IP: ");
      Serial.println(WiFi.softAPIP());
  } else {
      Serial.println("WiFi Connected Successfully!");
      Serial.print("Router IP: ");
      Serial.println(WiFi.localIP());
      WiFi.mode(WIFI_AP_STA);
      WiFi.softAP("ESP32_MPPT_PRO", "12345678");
      Serial.print("Hotspot also available at: ");
      Serial.println(WiFi.softAPIP());
  }

  // Web Server Routes
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/control", handleControl);
  server.on("/setPrice", handleSetPrice);
  server.on("/getPrice", handleGetPrice);
  server.on("/setTime", handleSetTime);
  server.on("/historical", handleHistoricalData);
  server.on("/localip", handleLocalIP);
  server.on("/remote", handleRemoteStatus);
  server.on("/setAlgorithm", handleSetAlgorithm);
  server.on("/setMode", handleSetMode);
  server.on("/setMaxVoltage", handleSetMaxVoltage);
  server.on("/setMinVoltage", handleSetMinVoltage);
  server.on("/setMaxCurrent", handleSetMaxCurrent);
  server.on("/setFanTemp", handleSetFanTemp);
  server.on("/factoryReset", handleFactoryReset);
  server.begin();
  
  Serial.println("Remote dashboard enabled");
  Remote_Access_Setup();
  

  // GPIO Initialization
  pinMode(backflow_MOSFET, OUTPUT);
  pinMode(buck_EN, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(Load, OUTPUT);
  pinMode(TempSensor, INPUT);
  pinMode(ADC_ALERT, INPUT);
  pinMode(buttonLeft, INPUT);
  pinMode(buttonRight, INPUT);
  pinMode(buttonBack, INPUT);
  pinMode(buttonSelect, INPUT);
  
  // PWM Initialization
  ledcAttach(buck_IN, pwmFrequency, pwmResolution);
  ledcWrite(pwmChannel, 0);
  
  digitalWrite(buck_EN, LOW);
  digitalWrite(backflow_MOSFET, LOW);
  delay(500);
  
  pwmMax = pow(2, pwmResolution) - 1;
  pwmMaxLimited = (PWM_MaxDC * pwmMax) / 100.000;
  
  ledcWrite(0, 0);
  
  digitalWrite(buck_EN, LOW);
  digitalWrite(backflow_MOSFET, LOW);
  delay(100);
  
  pwmMax = pow(2, pwmResolution) - 1;
  pwmMaxLimited = (PWM_MaxDC * pwmMax) / 100.000;

  // ADC Initialization
  ADC_SetGain();
  ads.begin();

  buck_Disable();

  // Enable Dual Core
  xTaskCreatePinnedToCore(coreTwo, "coreTwo", 10000, NULL, 0, &Core2, 0);

  // EEPROM Initialization
  EEPROM.begin(512);
  Serial.println("> FLASH MEMORY: STORAGE INITIALIZED");
  loadPersistentData();
  initializeFlashAutoload();
  Serial.println("> FLASH MEMORY: SAVED DATA LOADED");

  // LCD Initialization
  if(enableLCD == 1) {
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("MPPT INITIALIZED");
    lcd.setCursor(0, 1);
    lcd.print("FIRMWARE ");
    lcd.print(firmwareInfo);
    delay(1000);
    lcd.clear();
  }
  // Initialize Telegram Bot
  //initTelegramBot();
  initTime();
  loadHistoricalData();

  Serial.println("> MPPT HAS INITIALIZED");


}

//============================================================================//
// Main Loop
//============================================================================//

void loop() {
  Read_Sensors();
  Device_Protection();
  System_Processes();
  Charging_Algorithm();
  Onboard_Telemetry();
  LCD_Menu();
  load();
  Remote_Access_Loop();

  server.handleClient();
  
  saveCurrentData();
  
  //checkTelegramBot();  // Changed from checkTelegramMessages()
  // checkAndSendAlerts();  
  
  static unsigned long lastWhSave = 0;
  if (millis() - lastWhSave > 1000) {
    lastWhSave = millis();
    EEPROM.put(EE_WH_ADDR, Wh);
    EEPROM.commit();
    Serial.println("Wh saved to EEPROM");
  }
  
  delay(1);
}

//============================================================================//
// Core 2 Function
//============================================================================//

void coreTwo(void * pvParameters) {
  // Initialize Telegram Bot on Core 2 (do this ONCE)
  initTelegramBot();
  
  while (1) {
    // Handle Telegram messages on Core 2
    checkTelegramBot();
    
    // Your existing wireless telemetry
    Wireless_Telemetry();
    server.handleClient();
    
    delay(10); // Small delay to prevent watchdog triggers
  }
}

//============================================================================//
// Data Logging Functions
//============================================================================//

void saveCurrentData() {
  static unsigned long lastSaveTime = 0;
  unsigned long now = millis();
  
  // Check if time is valid (after 2024)
  time_t currentTime = time(nullptr);
  if (currentTime < 1704067200) { // Before Jan 1, 2024
    Serial.println("Time not set yet - skipping data save");
    return; // Don't save if time isn't set
  }
  
  if (now - lastSaveTime < 60000) { // 1 minute
    return;
  }
  lastSaveTime = now;
  
  struct tm *timeinfo = localtime(&currentTime);
  int hourIndex = timeinfo->tm_hour;
  int minuteIndex = timeinfo->tm_min;
  
  time_t currentDate = getCurrentDate();
  
  // Get or create daily slot
  DailyData* dailyData = getDailyDataSlot(currentDate);
  
  // Save to minute slot
  dailyData->hours[hourIndex].minutes[minuteIndex].voltageInput = voltageInput;
  dailyData->hours[hourIndex].minutes[minuteIndex].currentInput = currentInput;
  dailyData->hours[hourIndex].minutes[minuteIndex].powerInput = powerInput;
  dailyData->hours[hourIndex].minutes[minuteIndex].voltageOutput = voltageOutput;
  dailyData->hours[hourIndex].minutes[minuteIndex].currentOutput = currentOutput;
  dailyData->hours[hourIndex].minutes[minuteIndex].temperature = temperature;
  dailyData->hours[hourIndex].minutes[minuteIndex].timestamp = currentTime;
  
  saveHistoricalData();
  
  
  Serial.print(hourIndex);
  Serial.print(":");
  Serial.println(minuteIndex);
}

void saveHistoricalData() {
  int addr = HISTORY_START_ADDR;
  
  Serial.println("=== SAVING TO EEPROM ===");
  
  for (int day = 0; day < MAX_DAYS; day++) {
    // Save valid flag
    EEPROM.write(addr++, historicalData[day].valid ? 1 : 0);
    
    // Save date
    uint32_t date = (uint32_t)historicalData[day].date;
    EEPROM.write(addr++, (date >> 24) & 0xFF);
    EEPROM.write(addr++, (date >> 16) & 0xFF);
    EEPROM.write(addr++, (date >> 8) & 0xFF);
    EEPROM.write(addr++, date & 0xFF);
    
    // For each hour (0-23)
    for (int hour = 0; hour < 24; hour++) {
      // For each minute (0-59)
      for (int minute = 0; minute < 60; minute++) {
        MinuteData* m = &historicalData[day].hours[hour].minutes[minute];
        
        // Save minute data
        saveFloatToEEPROM(addr, m->voltageInput); addr += 4;
        saveFloatToEEPROM(addr, m->currentInput); addr += 4;
        saveFloatToEEPROM(addr, m->powerInput); addr += 4;
        saveFloatToEEPROM(addr, m->voltageOutput); addr += 4;
        saveFloatToEEPROM(addr, m->currentOutput); addr += 4;
        saveFloatToEEPROM(addr, m->temperature); addr += 4;
        
        uint32_t ts = (uint32_t)m->timestamp;
        EEPROM.write(addr++, (ts >> 24) & 0xFF);
        EEPROM.write(addr++, (ts >> 16) & 0xFF);
        EEPROM.write(addr++, (ts >> 8) & 0xFF);
        EEPROM.write(addr++, ts & 0xFF);
      }
    }
  }
  
  EEPROM.commit();
  Serial.println("Historical data saved to EEPROM");
  Serial.println("========================");
}

void loadHistoricalData() {
  int addr = HISTORY_START_ADDR;
  time_t currentTime = time(nullptr);
  
  Serial.println("=== LOADING HISTORICAL DATA ===");
  
  for (int day = 0; day < MAX_DAYS; day++) {
    // Read valid flag
    historicalData[day].valid = (EEPROM.read(addr++) == 1);
    
    // Read date
    uint32_t date = 0;
    date = (date << 8) | EEPROM.read(addr++);
    date = (date << 8) | EEPROM.read(addr++);
    date = (date << 8) | EEPROM.read(addr++);
    date = (date << 8) | EEPROM.read(addr++);
    historicalData[day].date = (time_t)date;
    
    // Check if date is valid (within last 5 days)
    bool dateValid = false;
    if (historicalData[day].valid && historicalData[day].date > 0) {
      time_t dateDiff = currentTime - historicalData[day].date;
      if (dateDiff > 0 && dateDiff < 432000) { // Within last 5 days (432000 seconds)
        dateValid = true;
      }
    }
    
    if (!dateValid) {
      historicalData[day].valid = false;
      Serial.print("Day ");
      Serial.print(day);
      Serial.println(": Invalid/old data - ignoring");
    }
    
    // Read minute data (only if date is valid)
    for (int hour = 0; hour < 24; hour++) {
      for (int minute = 0; minute < 60; minute++) {
        MinuteData* m = &historicalData[day].hours[hour].minutes[minute];
        
        m->voltageInput = readFloatFromEEPROM(addr); addr += 4;
        m->currentInput = readFloatFromEEPROM(addr); addr += 4;
        m->powerInput = readFloatFromEEPROM(addr); addr += 4;
        m->voltageOutput = readFloatFromEEPROM(addr); addr += 4;
        m->currentOutput = readFloatFromEEPROM(addr); addr += 4;
        m->temperature = readFloatFromEEPROM(addr); addr += 4;
        
        uint32_t ts = 0;
        ts = (ts << 8) | EEPROM.read(addr++);
        ts = (ts << 8) | EEPROM.read(addr++);
        ts = (ts << 8) | EEPROM.read(addr++);
        ts = (ts << 8) | EEPROM.read(addr++);
        m->timestamp = (time_t)ts;
        
        // If date is invalid, clear all minute data
        if (!dateValid) {
          m->timestamp = 0;
          m->voltageInput = 0;
          m->currentInput = 0;
          m->powerInput = 0;
          m->voltageOutput = 0;
          m->currentOutput = 0;
          m->temperature = 0;
        }
      }
    }
  }
  
  
}

void saveFloatToEEPROM(int addr, float value) {
  byte* p = (byte*)(void*)&value;
  for (int i = 0; i < 4; i++) {
    EEPROM.write(addr + i, *p++);
  }
}

float readFloatFromEEPROM(int addr) {
  float value = 0.0;
  byte* p = (byte*)(void*)&value;
  for (int i = 0; i < 4; i++) {
    *p++ = EEPROM.read(addr + i);
  }
  return value;
}

void handleHistoricalData() {
  String json = "[";
  bool firstDay = true;
  int totalPoints = 0;
  time_t currentTime = time(nullptr);
  
 
  
  if (currentTime < 1704067200) {
    
    server.send(200, "application/json", "[]");
    return;
  }
  
  for (int day = 0; day < MAX_DAYS; day++) {
    if (!historicalData[day].valid) continue;
    
    // Direct validation without helper function
    if (historicalData[day].date == 0 || 
        historicalData[day].date < 1704067200 ||
        currentTime - historicalData[day].date > 432000) {
      continue;
    }
    
    if (!firstDay) json += ",";
    firstDay = false;
    
    char dateStr[20];
    struct tm *timeinfo = localtime(&historicalData[day].date);
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", timeinfo);
    
    json += "{\"date\":\"" + String(dateStr) + "\",";
    json += "\"data\":[";
    
    bool firstPoint = true;
    int dayPoints = 0;
    
    for (int hour = 0; hour < 24; hour++) {
      for (int minute = 0; minute < 60; minute++) {
        MinuteData* m = &historicalData[day].hours[hour].minutes[minute];
        
        // Direct validation for each point
        if (m->timestamp != 0 && 
            m->timestamp >= 1704067200 && 
            currentTime - m->timestamp <= 432000 &&
            m->timestamp <= currentTime + 3600) {
          
          if (!firstPoint) json += ",";
          firstPoint = false;
          
          char timeStr[6];
          sprintf(timeStr, "%02d:%02d", hour, minute);
          
          json += "{";
          json += "\"time\":\"" + String(timeStr) + "\",";
          json += "\"vi\":" + String(m->voltageInput, 1) + ",";
          json += "\"ii\":" + String(m->currentInput, 2) + ",";
          json += "\"pi\":" + String(m->powerInput, 1) + ",";
          json += "\"vo\":" + String(m->voltageOutput, 2) + ",";
          json += "\"io\":" + String(m->currentOutput, 2) + ",";
          json += "\"temp\":" + String(m->temperature, 1);
          json += "}";
          
          dayPoints++;
          totalPoints++;
        }
      }
    }
    
    json += "]}";
  }
  
  json += "]";
  
  if (totalPoints == 0) {
    json = "[]";
  }
  
  server.send(200, "application/json", json);
  
  Serial.print("Total points sent: ");
  Serial.println(totalPoints);
}

void loadPersistentData() {
  float savedWh;
  EEPROM.get(EE_WH_ADDR, savedWh);
  if (!isnan(savedWh) && savedWh >= 0) {
    Wh = savedWh;
  }
  
  float savedPrice;
  EEPROM.get(EE_PRICE_ADDR, savedPrice);
  if (!isnan(savedPrice) && savedPrice > 0) {
    electricalPrice = savedPrice;
  }
}

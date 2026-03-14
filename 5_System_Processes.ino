// Function prototypes - ADD THIS SECTION AT THE TOP
void resetVariables();
void saveCurrentData();
void saveHistoricalData();
bool isWithinLoggingHours();
void loadSettings();

void System_Processes(){
  ///////////////// FAN COOLING /////////////////
  if(enableFan==true){
    if(enableDynamicCooling==false){                                   //STATIC PWM COOLING MODE (2-PIN FAN - no need for hysteresis, temp data only refreshes after 'avgCountTS' or every 500 loop cycles)                       
      if(overrideFan==true){fanStatus=true;}                           //Force on fan
      else if(temperature>=temperatureFan){fanStatus=1;}               //Turn on fan when set fan temp reached
      else if(temperature<temperatureFan){fanStatus=0;}                //Turn off fan when set fan temp reached
      digitalWrite(FAN,fanStatus);                                     //Send a digital signal to the fan MOSFET
    }
    else{}                                                             //DYNAMIC PWM COOLING MODE (3-PIN FAN - coming soon)
  }
  else{digitalWrite(FAN,LOW);}                                         //Fan Disabled
  
  //////////// LOOP TIME STOPWATCH ////////////
  loopTimeStart = micros();                                            //Record Start Time
  loopTime = (loopTimeStart-loopTimeEnd)/1000.000;                     //Compute Loop Cycle Speed (mS)
  loopTimeEnd = micros();                                              //Record End Time

  ///////////// AUTO DATA RESET /////////////
  if(telemCounterReset==0){}                                           //Never Reset
  else if(telemCounterReset==1 && daysRunning>1)  {resetVariables();}  //Daily Reset
  else if(telemCounterReset==2 && daysRunning>7)  {resetVariables();}  //Weekly Reset
  else if(telemCounterReset==3 && daysRunning>30) {resetVariables();}  //Monthly Reset
  else if(telemCounterReset==4 && daysRunning>365){resetVariables();}  //Yearly Reset 

  ///////////// DATA LOGGING (6AM-6PM) /////////////
  static unsigned long lastDataLogTime = 0;
  unsigned long currentTime = millis();
  
  // Save data every hour (3600000 ms) between 6AM-6PM
  if (currentTime - lastDataLogTime >= 1000) {
    if (isWithinLoggingHours()) {
      saveCurrentData();
      Serial.println("Data saved to history");
    }
    lastDataLogTime = currentTime;
  }

  ///////////// LOW POWER MODE /////////////
  if(lowPowerMode==1){}   
  else{}      
}

void factoryReset(){
  EEPROM.write(0,1);  //STORE: Charging Algorithm (1 = MPPT Mode)
  EEPROM.write(12,1); //STORE: Charger/PSU Mode Selection (1 = Charger Mode)
  
  // Max Voltage: 12.0V
  EEPROM.write(1,12); //STORE: Max Battery Voltage (whole)
  EEPROM.write(2,0);  //STORE: Max Battery Voltage (decimal)
  
  // Min Voltage: 9.0V
  EEPROM.write(3,9);  //STORE: Min Battery Voltage (whole)
  EEPROM.write(4,0);  //STORE: Min Battery Voltage (decimal)
  
  // Max Current: 30.0A - FIXED TO MATCH saveSettings PATTERN
  int defaultCurrent = 30; // 30.0A
  EEPROM.write(5, defaultCurrent);  // Whole number part (30)
  EEPROM.write(6, 0);                // Decimal part (0)
  
  EEPROM.write(7,1);  //STORE: Fan Enable (Bool)
  EEPROM.write(8,60); //STORE: Fan Temp (Integer)
  EEPROM.write(9,90); //STORE: Shutdown Temp (Integer)
  EEPROM.write(10,1); //STORE: Enable WiFi (Boolean)
  EEPROM.write(11,1); //STORE: Enable autoload (on by default)
  EEPROM.write(13,0); //STORE: LCD backlight sleep timer (default: 0 = never)
  
  EEPROM.commit();
  loadSettings();
}

void loadSettings(){ 
  MPPT_Mode = EEPROM.read(0);
  output_Mode = EEPROM.read(12);
  
  // Load Max Voltage
  voltageBatteryMax = EEPROM.read(1) + (EEPROM.read(2) * 0.01);
  
  // Load Min Voltage
  voltageBatteryMin = EEPROM.read(3) + (EEPROM.read(4) * 0.01);
  
  // Load Max Current - EXACT same pattern
  currentCharging = EEPROM.read(5) + (EEPROM.read(6) * 0.01);
  
  enableFan = EEPROM.read(7);
  temperatureFan = EEPROM.read(8);
  temperatureMax = EEPROM.read(9);
  enableWiFi = EEPROM.read(10);
  flashMemLoad = EEPROM.read(11);
  backlightSleepMode = EEPROM.read(13);
}

void saveSettings(){
  EEPROM.write(0, MPPT_Mode);              // Algorithm
  EEPROM.write(12, output_Mode);           // Charge/PSU Mode
  
  // Save Max Voltage
  int maxV = voltageBatteryMax * 100;
  EEPROM.write(1, maxV / 100);
  EEPROM.write(2, maxV % 100);
  
  // Save Min Voltage
  int minV = voltageBatteryMin * 100;
  EEPROM.write(3, minV / 100);
  EEPROM.write(4, minV % 100);
  
  // Save Max Current - EXACT same pattern as voltage
  int maxC = currentCharging * 100;
  EEPROM.write(5, maxC / 100);
  EEPROM.write(6, maxC % 100);
  
  EEPROM.write(7, enableFan);
  EEPROM.write(8, temperatureFan);
  EEPROM.write(9, temperatureMax);
  EEPROM.write(10, enableWiFi);
  EEPROM.write(13, backlightSleepMode);
  
  EEPROM.commit();
}

void initializeFlashAutoload(){
  if(disableFlashAutoLoad==0){
    flashMemLoad = EEPROM.read(11);       //Load saved autoload (must be excluded from bulk save, uncomment under discretion) 
    if(flashMemLoad==1){loadSettings();}  //Load stored settings from flash memory  
  } 
}

// Clean up data older than 5 days
void cleanupOldData() {
  time_t fiveDaysAgo = time(nullptr) - (5 * 24 * 3600);
  
  for (int i = 0; i < MAX_DAYS; i++) {
    if (historicalData[i].valid && historicalData[i].date < fiveDaysAgo) {
      historicalData[i].valid = false;
    }
  }
  
  saveHistoricalData();
  Serial.println("Cleaned up old historical data");
}

void saveAutoloadSettings(){
  EEPROM.write(11,flashMemLoad);       //STORE: Enable autoload
  EEPROM.commit();                     //Saves setting changes to flash memory
}
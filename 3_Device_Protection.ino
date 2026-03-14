// Add these variables at the top of your file (global scope)
unsigned long lastBackflowChange = 0;
const unsigned long BACKFLOW_MIN_TIME = 2000; // 2 seconds minimum between changes
bool lastBypassState = 0;

void backflowControl(){                                                //PV BACKFLOW CONTROL (INPUT MOSFET) 
  bool newBypassEnable = bypassEnable; // Store new state
  
  if(output_Mode==0){
    newBypassEnable = 1;                                  //PSU MODE: Force backflow MOSFET on
  }
  else{                                                                //CHARGER MODE
    // Add hysteresis to prevent oscillation
    if(voltageInput > voltageOutput + voltageDropout + 0.5){     // Turn ON with 0.5V margin
      newBypassEnable = 1;
    }
    else if(voltageInput < voltageOutput + voltageDropout - 0.3){ // Turn OFF with 0.3V margin
      newBypassEnable = 0;
    }
    // If between thresholds, keep current state
  }
  
  // Only change state if:
  // 1. State is different AND
  // 2. Enough time has passed since last change
  if(newBypassEnable != lastBypassState && millis() - lastBackflowChange > BACKFLOW_MIN_TIME){
    bypassEnable = newBypassEnable;
    digitalWrite(backflow_MOSFET, bypassEnable);  // Signal backflow MOSFET GPIO pin
    lastBypassState = bypassEnable;
    lastBackflowChange = millis();
  }
}

void Device_Protection(){
  //ERROR COUNTER RESET
  currentRoutineMillis = millis();
  if(currentErrorMillis-prevErrorMillis>=errorTimeLimit){                                           //Run routine every millisErrorInterval (ms)
    prevErrorMillis = currentErrorMillis;                                                           //Store previous time
    if(errorCount<errorCountLimit){errorCount=0;}                                                   //Reset error count if it is below the limit before x milliseconds  
    else{}                                                                                          // TO ADD: sleep and charging pause if too many errors persists   
  } 
  
  //FAULT DETECTION     
  ERR = 0;                                                                                          //Reset local error counter
  backflowControl();                                                                                //Run backflow current protection protocol  
  
  // Remove the duplicate backflowControl() call below
  // (You're calling it twice in charger mode, which can cause issues)
  
  if(temperature>temperatureMax)                           {OTE=1;ERR++;errorCount++;}else{OTE=0;}  //OTE - OVERTEMPERATURE: System overheat detected
  if(currentInput>currentInAbsolute)                       {IOC=1;ERR++;errorCount++;}else{IOC=0;}  //IOC - INPUT  OVERCURRENT: Input current has reached absolute limit
  if(currentOutput>currentOutAbsolute)                     {OOC=1;ERR++;errorCount++;}else{OOC=0;}  //OOC - OUTPUT OVERCURRENT: Output current has reached absolute limit 
  if(voltageOutput>voltageBatteryMax+voltageBatteryThresh) {OOV=1;ERR++;errorCount++;}else{OOV=0;}  //OOV - OUTPUT OVERVOLTAGE: Output voltage has reached absolute limit                     
  if(voltageInput<vInSystemMin&&voltageOutput<vInSystemMin){FLV=1;ERR++;errorCount++;}else{FLV=0;}  //FLV - Fatally low system voltage (unable to resume operation)

  if(output_Mode==0){                                                                               //PSU MODE specific protection protocol
    REC = 0; BNC = 0;                                                                               //Clear recovery and battery not connected boolean identifiers
    if(voltageInput<voltageBatteryMax+voltageDropout){IUV=1;ERR++;errorCount++;}else{IUV=0;}        //IUV - INPUT UNDERVOLTAGE: Input voltage is below battery voltage (for psu mode only)                     
  }
  else{                                                                                             //Charger MODE specific protection protocol
    // backflowControl(); REMOVED - Already called above
    if(voltageOutput<vInSystemMin)                   {BNC=1;ERR++;}      else{BNC=0;}               //BNC - BATTERY NOT CONNECTED (for charger mode only, does not treat BNC as error when not under MPPT mode)
    if(voltageInput<voltageBatteryMax+voltageDropout){IUV=1;ERR++;REC=1;}else{IUV=0;}               //IUV - INPUT UNDERVOLTAGE: Input voltage is below max battery charging voltage (for charger mode only)     
  } 
}
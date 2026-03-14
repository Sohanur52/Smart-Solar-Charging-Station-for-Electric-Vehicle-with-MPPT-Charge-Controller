void buck_Enable(){                                                                  //Enable MPPT Buck Converter
  // Add small delay for stability
  delay(5);
  buckEnable = 1;
  digitalWrite(buck_EN,HIGH);
  digitalWrite(LED,HIGH);
}

void buck_Disable(){                                                                 //Disable MPPT Buck Converter
  buckEnable = 0; 
  digitalWrite(buck_EN,LOW);
  digitalWrite(LED,LOW);
  PWM = 0;
  ledcWrite(buck_IN, 0);  // Ensure PWM is zero
}   

void predictivePWM(){                                                                //PREDICTIVE PWM ALGORITHM 
  if(voltageInput<=0){PPWM=0;}                                                       //Prevents Indefinite Answer when voltageInput is zero
  else{PPWM =(PPWM_margin*pwmMax*voltageOutput)/(100.00*voltageInput);}              //Compute for predictive PWM Floor and store in variable
  PPWM = constrain(PPWM,0,pwmMaxLimited);
}   

void PWM_Modulation(){
  if(output_Mode==0){PWM = constrain(PWM,0,pwmMaxLimited);}                          //PSU MODE PWM = PWM OVERFLOW PROTECTION
  else{
    predictivePWM();                                                                 //Runs and computes for predictive pwm floor
    PWM = constrain(PWM,PPWM,pwmMaxLimited);                                         //CHARGER MODE PWM - limit floor to PPWM
  } 
  
  // Add minimum PWM to prevent buzzing (but only if PWM > 0)
  if (PWM > 0 && PWM < 50) {
    PWM = 50;  // Minimum PWM value to avoid subharmonic oscillation
  }
  
  // Add maximum PWM limit
  if (PWM > pwmMax * 0.95) {
    PWM = pwmMax * 0.95;  // Maximum 95% duty cycle
  }
  
  ledcWrite(buck_IN, PWM);
  buck_Enable();                                                                     //Turn on MPPT buck (IR2104)
}
     
void Charging_Algorithm(){
  // ============ CRITICAL CHECK ============
  // If only battery is present (no solar), disable everything
  if (voltageInput < voltageOutput + 2.0 && output_Mode == 1) {
    buck_Disable();
    digitalWrite(backflow_MOSFET, LOW);
    return;  // Exit immediately
  }
  
  if(ERR>0||chargingPause==1){
    buck_Disable();                                                                   //ERROR PRESENT - Turn off MPPT buck
  }
  else{
    if(REC==1){                                                                      //IUV RECOVERY
      REC=0;                                                                         //Reset IUV recovery boolean identifier 
      buck_Disable();                                                                //Disable buck before PPWM initialization
      lcd.setCursor(0,0);lcd.print("POWER SOURCE    ");                              
      lcd.setCursor(0,1);lcd.print("DETECTED        ");                              
      Serial.println("> Solar Panel Detected");                                      
                                  
      for(int i = 0; i<40; i++){Serial.print(".");delay(30);}                        
      Serial.println("");                                                            
      Read_Sensors();
      predictivePWM();
      PWM = PPWM; 
      lcd.clear();
    }  
    else{                                                                            //NO ERROR PRESENT              
      /////////////////////// CC-CV BUCK PSU ALGORITHM ////////////////////////////// 
      if(MPPT_Mode==0){                                                              //CC-CV PSU MODE
        int pwmChange = 0;
        
        // FASTER CHARGING - REMOVED RATE LIMITING
        if(currentOutput > currentCharging) {
          pwmChange = -5;  // Decrease faster if current is too high
        }
        else if(voltageOutput > voltageBatteryMax) {
          pwmChange = -5;  // Decrease faster if voltage is too high
        }
        else if(voltageOutput < voltageBatteryMax) {
          // Calculate voltage error
          float voltageError = voltageBatteryMax - voltageOutput;
          
          // AGGRESSIVE CHARGING: Bigger steps for larger errors
          if (voltageError > 2.0) {
            pwmChange = 15;  // Very far from target - increase fast
          }
          else if (voltageError > 1.0) {
            pwmChange = 10;  // Moderately far - medium increase
          }
          else if (voltageError > 0.5) {
            pwmChange = 8;   // Getting close - slower increase
          }
          else if (voltageError > 0.2) {
            pwmChange = 5;   // Near target - gentle increase
          }
          else {
            pwmChange = 3;   // Very close - fine tuning
          }
          
          // Boost if current is also below limit
          if (currentOutput < currentCharging * 0.8) {
            pwmChange = pwmChange * 1.5;  // Increase more aggressively
          }
        }
        
        // Apply PWM change
        PWM += pwmChange;
        
        // Apply limits
        PWM = constrain(PWM, 0, pwmMaxLimited);
        
        // Apply PWM modulation (with minimum PWM check inside)
        PWM_Modulation();
      }     
      /////////////////////// MPPT & CC-CV CHARGING ALGORITHM ///////////////////////  
      else{                                                                                                                                                         
        if(currentOutput > currentCharging) {
          PWM -= 5;  // Current too high - decrease fast
        }
        else if(voltageOutput > voltageBatteryMax) {
          PWM -= 5;  // Voltage too high - decrease fast
        }
        else {                                                                       //MPPT ALGORITHM
          // Add stability check to prevent hunting
          static int stableCount = 0;
          float powerChange = abs(powerInput - powerInputPrev);
          float voltageChange = abs(voltageInput - voltageInputPrev);
          
          // If power and voltage are stable, don't change PWM
          if (powerChange < 1.0 && voltageChange < 0.5 && stableCount < 5) {
            stableCount++;
          } else {
            stableCount = 0;
            
            int pwmStep = 3;  // Default step size
            
            // Larger steps when far from MPP
            if (abs(powerInput - powerInputPrev) > 5.0) {
              pwmStep = 8;
            }
            
            if(powerInput > powerInputPrev && voltageInput > voltageInputPrev)     {PWM -= pwmStep;}  //  ↑P ↑V ; →MPP  //D--
            else if(powerInput > powerInputPrev && voltageInput < voltageInputPrev){PWM += pwmStep;}  //  ↑P ↓V ; MPP←  //D++
            else if(powerInput < powerInputPrev && voltageInput > voltageInputPrev){PWM += pwmStep;}  //  ↓P ↑V ; MPP→  //D++
            else if(powerInput < powerInputPrev && voltageInput < voltageInputPrev){PWM -= pwmStep;}  //  ↓P ↓V ; ←MPP  //D--
            else if(voltageOutput < voltageBatteryMax) {
              // Battery not fully charged - try to increase
              float voltageError = voltageBatteryMax - voltageOutput;
              int increaseStep = 3;
              if (voltageError > 1.0) increaseStep = 8;
              PWM += increaseStep;
            }
            
            powerInputPrev   = powerInput;                                               //Store Previous Recorded Power
            voltageInputPrev = voltageInput;                                             //Store Previous Recorded Voltage        
          }
        }   
        
        // Apply limits and modulation
        PWM = constrain(PWM, 0, pwmMaxLimited);
        PWM_Modulation();                                                              //Set PWM signal to Buck PWM GPIO
      }  
    }
  }
}

int8_t getWifiQuality() {
  int32_t dbm = WiFi.RSSI();
  if (dbm <= -100) {
    return 0;
  } else if (dbm >= -50) {
    return 100;
  } else {
    return 2 * (dbm + 100);
  }
}
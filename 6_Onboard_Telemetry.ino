void Onboard_Telemetry(){    

  /////////////////////// USB SERIAL DATA TELEMETRY ////////////////////////   
  // 0 - Disable Serial
  // 1 - Display All
  // 2 - Display Essential Data
  // 3 - Display Numbers Only 

  currentSerialMillis = millis();
  if(currentSerialMillis-prevSerialMillis>=millisSerialInterval){   //Run routine every millisRoutineInterval (ms)
    prevSerialMillis = currentSerialMillis;                         //Store previous time

    if(serialTelemMode==0){}
//  else if(chargingPause==1){Serial.println("CHARGING PAUSED");}   // Charging paused message
    else if(serialTelemMode==2){                                    // 1 - Display All                           
      Serial.print(" ERR:");   Serial.print(ERR);
     
    }
    else if(serialTelemMode==1){ // 2 - Display Essential Data
      Serial.print(" PI:");    Serial.print(powerInput,0); 
      Serial.print(" PWM:");   Serial.print(PWM); 
      Serial.print(" PPWM:");  Serial.print(PPWM); 
      Serial.print(" VI:");    Serial.print(voltageInput,1); 
      Serial.print(" VO:");    Serial.print(voltageOutput,1); 
      Serial.print(" CI:");    Serial.print(currentInput,2); 
      Serial.print(" CO:");    Serial.print(currentOutput,2); 
      Serial.print(" Wh:");    Serial.print(Wh,2); 
      Serial.print(" Temp:");  Serial.print(temperature,1);  
      Serial.print(" EN:");    Serial.print(buckEnable);
      Serial.print(" FAN:");   Serial.print(fanStatus);
      Serial.print(" WiFi:");  Serial.print(WIFI); 
      Serial.print(" ");   
      Serial.print(" SOC:");   Serial.print(batteryPercent);Serial.print("%"); 
      Serial.print(" T:");     Serial.print(secondsElapsed); 
      Serial.print(" LoopT:"); Serial.print(loopTime,3);Serial.print("ms");  
      Serial.println("");    
    }  
    else if(serialTelemMode==3){ // 3 - Display Numbers Only 
      Serial.print(" ");       Serial.print(powerInput,0); 
      
    }  

  } 
}

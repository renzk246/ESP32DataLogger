//===== LCD STARTUP SCREEN =====//
void startupLCD(){
  int status;
  status = lcd.begin(LCD_COLS, LCD_ROWS);
  if (status){
    Serial.print("LCD Initialization Failes!!!");
    Serial.println(status);
    hd44780::fatalError(status);
  }
//  lcd.init();
//  lcd.backlight();
  lcd.setCursor(3,1);
  lcd.print("Boooting Up...");
  delay(2000);
  lcd.clear();
}
    

//===== LCD SENSOR DISPLAY =====//
void sensorDisplay(){
  if (millis() > previousTime1 + eventTime1){

    lcd.clear();
    
    // VOLTAGE
    lcd.setCursor(0,0);
    lcd.print("Volt");
    lcd.setCursor(0,1);
    lcd.print(voltageRMS);
  
    // CURRENT
    lcd.setCursor(7,0);
    lcd.print("Amp");
    lcd.setCursor(7,1);
    lcd.print(currentRMS);
  
    // POWER
    lcd.setCursor(13,0);
    lcd.print("Watt");
    lcd.setCursor(13,1);
    lcd.print(powerRMS);
  
    // RPM
    lcd.setCursor(0,2);
    lcd.print("RPM");
    lcd.setCursor(0,3);
    lcd.print(rpmAverage);
  
    // RECORDING
    lcd.setCursor(7,2);
    lcd.print("Rec");
    lcd.setCursor(7,3);
    if (recording){
      lcd.print("On");
    }
    else{
      lcd.print("Off");
    }

    // TIME RECORDING
    lcd.setCursor(13,2);
    lcd.print("Time");
    lcd.setCursor(13,3);
    lcd.print(timeRecording);

    previousTime1 = millis();
  }
}

void calibrationDisplay(){
  if (millis() > previousTime1 + eventTime1){
    lcd.clear();

    lcd.setCursor(2,0);
    lcd.print("Calibrating Mode");
    
    // VOLTAGE
    lcd.setCursor(0,1);
    lcd.print("Volt:");
    lcd.setCursor(6,1);
    lcd.print(voltageRMS);
  
    // CURRENT
    lcd.setCursor(0,2);
    lcd.print("Amp :");
    lcd.setCursor(6,2);
    lcd.print(currentRMS);
  
    // RPM
    lcd.setCursor(0,3);
    lcd.print("RPM :");
    lcd.setCursor(6,3);
    lcd.print(rpmAverage);

    previousTime1 = millis();
  }
}

//===== READ SENSORS (VOLTAGE and CURRENT) =====//
void signalCalculation(int voltage, float current, bool calibrated){
  float voltageSum = 0, currentSum = 0;
  voltageInputStats.input(voltage);
  
  if (millis() >= previousTime2 + eventTime2){
    if (calibrated){
      voltageCalculation = abs((voltageInputStats.sigma()) * voltageSlope - voltageIntercept);
      currentCalculation = (current / 1000) * currentCorrection;
    }
    else {
      voltageCalculation = abs((voltageInputStats.sigma()) - voltageIntercept);
      currentCalculation = (current / 1000);
    }
    
    if(sampleSelect < sampleCount){
      voltageSample[sampleSelect] = voltageCalculation;
      currentSample[sampleSelect] = currentCalculation;
      sampleSelect ++;
    }
    else{
      for (int x=0; x < sampleCount; x++){
        voltageSum += voltageSample[x]; 
        currentSum += currentSample[x];
      }
      voltageRMS = voltageSum / sampleCount;
      currentRMS = currentSum / sampleCount;

      if (voltageRMS < 20){
        voltageRMS = 0;
      }
      if (currentRMS < 0.05){
        currentRMS = 0;
      }
      
      // Reset
      sampleSelect = 0;
      voltageSum = 0;
      currentSum = 0;
    }
    previousTime2 = millis();
  }  
}

//==== RPM CALCULATION =====//
void rpmCalculation(){
  if (millis() >= previousTime3 + eventTime3){
    hallInterval0 = millis() - previousTime3;
    rpmAverage = ((hallCounter0 * 1000) / hallInterval0) * 60.0;
    rpmAverage /= magnetNumber;
    hallCounter0 = 0;
    previousTime3 = millis();
  }
}

//===== MAPPING FUNCTION =====//
float mapf(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
  float result;
  result = (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
  return result;
}

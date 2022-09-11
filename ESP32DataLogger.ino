#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header
#include <ACS712.h>
#include <Filters.h>


//=== PIN/MODULE ASSIGNEDMENT ===//
#define pinVoltage 34
#define pinCurrent 35
#define pinHall0 25
#define pinHall1 26
#define pinButton 4

hd44780_I2Cexp lcd;                                  // LCD
const int LCD_ROWS = 4;
const int LCD_COLS = 20;

ACS712 ACS(pinCurrent, 5.0, 4095, 185);              // ACS712 Current Sensor 5A w/ 185mV/A step.
File sdFile;                                         // SD Card


//=== VALUE INITIALIZATION ===//
int frequency = 60;                                  // AC Supply Frequency
double windowLength = 100/frequency;                 // How long to average the signal for FIlters library
float voltageIntercept = 2.0;                        // sensorReadValue needed to subtract/add when reading is not zero when there's no supply. Keep it zero.
double voltageSlope = 1.9565;                        // slope = actualValue / sensorReadValue, when slope is initially equal to 1.
double currentCorrection = 1.0;
RunningStatistics voltageInputStats;                 // In charge of calculating the RMS of the signal.
int magnetNumber = 2;                                // Number of magnets in a wheel.

String dataLoggerDir = "/DataLogger";                // Folder data logger files located.
String dataLoggerFile = "/data";                     // File name of the data in CSV format.                          
String settingsFile = "/settings.txt";               // Previous saved settings (Calibrations).
String newFile = "";


//=== VARIABLES ===//
float voltageRaw = 0, currentRaw = 0;                // Current, Voltage, and Power Variables
float voltageRMS = 0, currentRMS = 0;
float voltageCalculation = 0, currentCalculation = 0;
float powerRMS = 0;

int sampleSelect = 0, sampleCount = 4;              // Voltage and Current Samples (for stability)
float voltageSample[4], currentSample[4];

unsigned long timeRecording = 0;                     // SD Card Variables.
bool button = false;                    
bool recording = false;
bool calibrating = false;
bool isNewFileCreated = false;  

long rpmAverage = 0;                                 // RPM (3144 Hall Sensor) Variables
double hallCounter0 = 0;
unsigned long hallInterval0 = 0;
bool hallTrigger0 = true;


//=== EVENT TIMER ===//
const long eventTime0 = 10;                           // Timer for Debugging Display
const long eventTime1 = 1000;                        // Timer for LCD Display
const long eventTime2 = 250;                         // Timer for RMS Calculation (Sample Time)
const long eventTime3 = 1000;                        // Timer for RPM Calculation

unsigned long previousTime0 = 0;
unsigned long previousTime1 = 0;
unsigned long previousTime2 = 0;
unsigned long previousTime3 = 0;
unsigned long currentTime = 0;

const long buttonTime = 1000;                        // Time delay for button0 long-press function.
unsigned long pressedButtonTime = 0;
unsigned long releasedButtonTime = 0;
unsigned long durationButtonTime = 0;
int previousButtonState = LOW;

long recordInterval = 1000;                          // Time interval for data logging.
unsigned long previousRecordInterval = 0;

//=== DEBUG ===//
bool debug0 = true;                                  // Just keep it on because it can affects the ouput readings.

// INTERRUPT COUNTERT (RPM) //
void IRAM_ATTR isr(){
 hallCounter0++;
}

//======================= SETUP FUNCTION =======================//
void setup(){
  Serial.begin(115200);

  // PIN INITIALiZATIONS
  pinMode(pinCurrent, INPUT);
  pinMode(pinVoltage, INPUT);
  pinMode(pinButton, INPUT);
  pinMode(pinHall0, INPUT);
  pinMode(pinHall1, INPUT);

  // LCD INITIALIZATION
  startupLCD();

  // CHECK SD CARD
  initializeSD();
  readSettings();

  // FILTERS (STATISTICS)
  voltageInputStats.setWindowSecs(windowLength);

  // ACS712
  ACS.autoMidPoint();

  // RPM INTERRUPT PIN
  attachInterrupt(digitalPinToInterrupt(pinHall0), isr, FALLING);
}


//======================= MAIN FUNCTION ======================= //
void loop(){
  // READ SENSOR/MODULE SIGNAL
  voltageRaw = mapf(analogRead(pinVoltage), 0, 4095, 0, 1023);                        // Mapping the read value 0-4095 to 0-1023 because of the library was made based on Arduino signal.  
  currentRaw = ACS.mA_AC(frequency);
  button = digitalRead(pinButton);
  hallTrigger0 = digitalRead(pinHall0);
  
  
  // BUTTON STATE
  if ((previousButtonState == LOW) and (button == HIGH)){
    pressedButtonTime = millis();
  }
  else if ((previousButtonState == HIGH) and (button == LOW)){
    releasedButtonTime = millis();
    durationButtonTime = releasedButtonTime - pressedButtonTime;

    // Check button for quick/long press
    if (durationButtonTime < buttonTime){
      // Quick-Press
      if (!recording){
        calibrating = !calibrating;
      }  
    }
    else if (durationButtonTime > buttonTime){
      // Long Press
      if (!calibrating){
        recording = !recording;          
      } 
    }
  }
  previousButtonState = button;


  // RPM CALCULATION
  rpmCalculation();
  
  
  // LCD DISPLAY MODE SELECTION
  if (!calibrating){    
    signalCalculation(voltageRaw, currentRaw, true);
    powerRMS = voltageRMS * currentRMS;
    sensorDisplay(); 
  }
  else{ 
    signalCalculation(voltageRaw, currentRaw, false);
    powerRMS = voltageRMS * currentRMS;
    calibrationDisplay();
  }


  // RECORDING
  logData(recording);

  
  // DEBUGGING
  if (debug0){
    if (millis() > previousTime0 + eventTime0){
      Serial.print("Voltage RMS: ");
      Serial.print(voltageRMS);
      Serial.print(" - ");
      Serial.print("Voltage RAW: ");
      Serial.print(mapf(voltageRaw, 0, 1023, 0, 500));
      Serial.print(" - ");
      Serial.print("Current RMS: ");
      Serial.print(currentRMS);
      Serial.print(" - ");
      Serial.print("Power: ");
      Serial.print(powerRMS);
      Serial.print(" - ");
      Serial.print("Button: ");
      Serial.print(button);
      Serial.print(" - ");
      Serial.print("RPM: ");
      Serial.print(rpmAverage);
      Serial.print(" - ");
      Serial.print("Recording: ");
      Serial.print(recording);
      Serial.print(" - ");
      Serial.print("Time: ");
      Serial.print(timeRecording);
      Serial.print(" - ");
      Serial.print("FreeRam: ");
      Serial.println(ESP.getFreeHeap());

      previousTime0 = millis();
    }
  }
}

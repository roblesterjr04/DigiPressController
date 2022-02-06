#include <ArducamSSD1306.h>    // Modification of Adafruit_SSD1306 for ESP8266 compatibility
#include <Adafruit_GFX.h>   // Needs a little change in original Adafruit library (See README.txt file)
#include <Wire.h>           // For I2C comm, but needed for not getting compile error


#define LED_PIN 13
#define SWITCH_PIN 7
#define OLED_RESET  17

#define MAX_PRESSURE_POT A0
#define MAX_DIFF_POT     A1
#define PRESSURE_INPUT   A2
#define RELAY_STATE_1    8
#define RELAY_STATE_2    6
#define ON_PIN_STATE HIGH

#define SAMPLE_SIZE 5

#define CYCLE 250

#define SENSOR_LOW 0.5
#define SENSOR_HIGH 4.5

int maxSensorPSI = 0;
int maxDiff = 0;

#define PRINT_READINGS false

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

float cutoff;
float diff;

float pressure;

bool OK=true;

int pressVoltReadings[SAMPLE_SIZE];
int maxVoltReadings[SAMPLE_SIZE];
int diffVoltReadings[SAMPLE_SIZE];
int readIndex = 0;
int totalPressure = 0;
int totalMax = 0;
int totalDiff = 0;  
int dipSelect = 0;

ArducamSSD1306 lcd(OLED_RESET);

void setup() {
  // put your setup code here, to run once:

  //analogReference(INTERNAL);

  for (int i = 0; i < SAMPLE_SIZE; i++) {
    pressVoltReadings[i] = 0;
    maxVoltReadings[i] = 0;
    diffVoltReadings[i] = 0;
  }
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(SWITCH_PIN, OUTPUT);
  //pinMode(MAX_PRESSURE_POT, INPUT);
  //pinMode(MAX_DIFF_POT, INPUT);
  //pinMode(PRESSURE_INPUT, INPUT);
  pinMode(RELAY_STATE_1, INPUT);
  pinMode(RELAY_STATE_2, INPUT_PULLUP);

  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);

  int S1 = digitalRead(2) * 1;
  int S2 = digitalRead(3) * 2;
  int S3 = digitalRead(4) * 4;
  int S4 = digitalRead(5) * 8;

  dipSelect = S1 + S2 + S3 + S4;

  switch(dipSelect) {
    case 1:
      maxSensorPSI = 10;
      break;
    case 2:
      maxSensorPSI = 30;
      break;
    case 3:
      maxSensorPSI = 100;
      break;
    case 4:
      maxSensorPSI = 200;
      break;
    case 5:
      maxSensorPSI = 300;
      break;
    case 6:
      maxSensorPSI = 400;
      break;
    case 7:
      maxSensorPSI = 500;
      break;
  }

  maxDiff = maxSensorPSI * .2;

  Serial.begin(9600);

  Serial.println("*** DIGIPRESS ***");
  Serial.print("SENSOR : ");
  Serial.println(maxSensorPSI);
  
  lcd.begin();
  lcd.clearDisplay();
  lcd.setTextSize(1);
  lcd.setTextColor(WHITE);

  lcd.clearDisplay();
  lcd.setCursor(0,0);
  lcd.setTextSize(2);
  lcd.println("DIGI-PRESS");
  //lcd.setCursor(0,20);
  lcd.setTextSize(1);
  lcd.println("Firmware 1.0.2");
  lcd.println("By Rob Lester");
  lcd.display();
  delay(2000);  

  digitalWrite(LED_PIN, HIGH);
}

void loop() {

  delay(CYCLE);

  averagePressure();

  //Serial.println(maxDiff);

  cutoff = readPSI();
  diff = readDiff();

  pressure = readPressure();

  if (!checkSensor()) {
    toggleRelayOff();
    Serial.println("FAULT");
    OK = false;
  } else {
    OK = true;
  }
  
  if (pressure >= cutoff && powerState()) {
    //Serial.println("Setting reached, power off.");
    //printReadings();
    toggleRelayOff();
  }

  if (pressure < cutoff - diff && !powerState()) {
    //Serial.println("Below differential, power on.");
    //printReadings();
    toggleRelayOn();
  }

  printReadings();
  updateLCD();

}

void averagePressure() {
  totalPressure = totalPressure - pressVoltReadings[readIndex];
  totalMax = totalMax - maxVoltReadings[readIndex];
  totalDiff = totalDiff = diffVoltReadings[readIndex];
  // read from the sensor:
  pressVoltReadings[readIndex] = analogRead(PRESSURE_INPUT);
  maxVoltReadings[readIndex] = analogRead(MAX_PRESSURE_POT);
  diffVoltReadings[readIndex] = analogRead(MAX_DIFF_POT);

  //Serial.println(analogRead(MAX_PRESSURE_POT));
  
  // add the reading to the total:
  totalPressure = totalPressure + pressVoltReadings[readIndex];
  totalMax = totalMax + maxVoltReadings[readIndex];
  totalDiff = totalDiff + diffVoltReadings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= SAMPLE_SIZE) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }
}

float readPSI() {
  //float potLevel = totalMax / SAMPLE_SIZE;
  float potLevel = analogRead(MAX_PRESSURE_POT);
  float psi = (potLevel * maxSensorPSI) / 1023; //maxPressurePSI
  return psi;
}

float readDiff() {
  //float potLevel = totalDiff / SAMPLE_SIZE;
  float potLevel = analogRead(MAX_DIFF_POT);
  float psi = (potLevel * maxDiff) / 1023; //maxDiff
  return psi;
}

void toggleRelay() {
  digitalWrite(SWITCH_PIN, !powerState());
}

void toggleRelayOn() {
  if (!OK) return;
  digitalWrite(SWITCH_PIN, HIGH);
}

void toggleRelayOff() {
  digitalWrite(SWITCH_PIN, LOW);
}

bool powerState() {
  return digitalRead(SWITCH_PIN);
}

float readPressure() {
  
  float readingLevel = totalPressure / SAMPLE_SIZE;
  
  //Serial.println(readingLevel);
  float pressureZero = (SENSOR_LOW * 1023) / 5;
  float pressureMax = (SENSOR_HIGH * 1023) / 5;

  float psi = ((readingLevel - pressureZero) * maxSensorPSI) / (pressureMax - pressureZero);
  return psi;
}

void updateLCD() {
  lcd.clearDisplay();
  lcd.setCursor(0,0);
  lcd.setTextSize(2);
  if (OK) {
    if (powerState()) {
      lcd.println("RUNNING");
    } else {
      lcd.println("STOPPED");
    }
  } else {
    lcd.println("FAULT");
  }
  lcd.setTextSize(1);
  lcd.print("   Cutoff PSI:");
  lcd.println(cutoff);
  lcd.print("     Diff PSI:");
  lcd.println(diff);
  lcd.setTextSize(2);
  
  lcd.print(pressure);
  lcd.println(" PSI");
  if (!digitalRead(RELAY_STATE_1) || digitalRead(RELAY_STATE_2)) {
    lcd.setTextSize(1);
    lcd.println("Relay circuit low.");
  }
  lcd.display();
}

void printReadings() {

  float v = (float(analogRead(PRESSURE_INPUT)) * 5) / 1023;
  
  Serial.print("Max PSI: ");
  Serial.print(cutoff);
  Serial.print("  Diff: ");
  Serial.print(diff);
  Serial.print("  Reading: ");
  Serial.print(pressure);
  Serial.print("  Input Voltage: ");
  Serial.println(v);
}

bool checkSensor() {
  int inlet = analogRead(PRESSURE_INPUT);
  //bool relay1 = digitalRead(RELAY_STATE_1);
  //bool relay2 = !digitalRead(RELAY_STATE_2);
  return inlet < 1000 && inlet > 85 && pressure < maxSensorPSI && cutoff >= 0 && diff > 0.1;
}

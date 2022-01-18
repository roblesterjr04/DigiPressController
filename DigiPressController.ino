

#define LED_PIN 13
#define SWITCH_PIN 12

#define MAX_PRESSURE_POT A0
#define MAX_DIFF_POT     A1
#define PRESSURE_INPUT   A4

#define ON_PIN_STATE HIGH

#define MAX_PSI 5
#define MAX_DIFF 2

#define CYCLE 250

#define SENSOR_PSI_MAX 6
#define SENSOR_LOW 0.5
#define SENSOR_HIGH 4.5

#define PRINT_READINGS false

int readingDebounce = 0;

float psi;
float diff;

float currentPressure;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_PIN, OUTPUT);
  pinMode(SWITCH_PIN, OUTPUT);
  pinMode(MAX_PRESSURE_POT, INPUT);
  pinMode(MAX_DIFF_POT, INPUT);
  pinMode(PRESSURE_INPUT, INPUT);

  Serial.begin(9600);
  Serial.println("***************************************");
  Serial.println("* Digital Pressure Control Relay v1.0 *");
  Serial.println("***************************************");
  Serial.println(" ");

  digitalWrite(LED_PIN, HIGH);
}

void loop() {

  delay(CYCLE);

  psi = readPSI();
  diff = readDiff();

  currentPressure = readPressure();

  if (currentPressure >= psi && powerState()) {
    Serial.println("Setting reached, power off.");
    printReadings();
    toggleRelayOff();
  }

  if (currentPressure < psi - diff && !powerState()) {
    Serial.println("Below differential, power on.");
    printReadings();
    toggleRelayOn();
  }


  if (PRINT_READINGS) {
    printReadings();
  }

  //toggleRelay();
}

float readPSI() {
  float potLevel = analogRead(MAX_PRESSURE_POT);

  float psi = (potLevel * MAX_PSI) / 1024;
  return psi;
}

float readDiff() {
  float potLevel = analogRead(MAX_DIFF_POT);

  float psi = (potLevel * MAX_DIFF) / 1024;
  return psi;
}

void toggleRelay() {
  digitalWrite(SWITCH_PIN, !powerState());
}

void toggleRelayOn() {
  digitalWrite(SWITCH_PIN, HIGH);
}

void toggleRelayOff() {
  digitalWrite(SWITCH_PIN, LOW);
}

bool powerState() {
  return digitalRead(SWITCH_PIN);
}

float readPressure() {
  
  float readingLevel = analogRead(PRESSURE_INPUT);
  float pressureZero = (SENSOR_LOW * 1024) / 5;
  float pressureMax = (SENSOR_HIGH * 1024) / 5;

  float psi = ((readingLevel - pressureZero) * SENSOR_PSI_MAX) / (pressureMax - pressureZero);
  return psi;
}

void printReadings() {
  Serial.print("Max PSI: ");
  Serial.print(psi);
  Serial.print("  Diff: ");
  Serial.print(diff);
  Serial.print("  Reading: ");
  Serial.println(currentPressure);
}

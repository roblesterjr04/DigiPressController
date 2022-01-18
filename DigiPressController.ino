

#define LED_PIN 12

#define MAX_PRESSURE_POT A0
#define MAX_DIFF_POT     A1

#define MAX_PSI 5
#define MAX_DIFF 2

#define CYCLE 250

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_PIN, OUTPUT);
  pinMode(MAX_PRESSURE_POT, INPUT);

  Serial.begin(9600);

  digitalWrite(LED_PIN, HIGH);
}

void loop() {

  delay(CYCLE);

  float psi = readPSI();
  float diff = readDiff();

  Serial.print("Max PSI: ");
  Serial.print(psi);
  Serial.print("  Diff: ");
  Serial.println(diff);
}

float readPSI() {
  float potLevel = analogRead(MAX_PRESSURE_POT);

  float psi = (potLevel * MAX_PSI) / 1023;
  return psi;
}

float readDiff() {
  float potLevel = analogRead(MAX_DIFF_POT);

  float psi = (potLevel * MAX_DIFF) / 1023;
  return psi;
}

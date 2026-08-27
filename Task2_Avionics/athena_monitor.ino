/*
 * Task 2: Odysseus Onboard State Machine
 * System: Athena Navigation & Avionics Monitoring System
 * Name: Siddarth Varma Kalidindi
 * ID: 2026B5PS0896H
 */

#include <LiquidCrystal.h>

// ============================================================================
// SECTION 1: HARDWARE PIN DEFINITIONS & INITIALIZATION
// ============================================================================

// LCD Pins: RS -> Pin 12, EN -> Pin 11, D4 -> Pin 5, D5 -> Pin 4, D6 -> Pin 3, D7 -> Pin 2
const int RS = 12, EN = 11, D4 = 5, D5 = 4, D6 = 3, D7 = 2;
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

// Digital & Analog I/O Pins
const int SWITCH_PIN = 7;     // Anchor Override Slide Switch Pin
const int TRIG_PIN = 8;      // HC-SR04 Ultrasonic Sensor Trigger Pin
const int ECHO_PIN = 9;      // HC-SR04 Ultrasonic Sensor Echo Pin
const int LDR_PIN = A0;      // Photoresistor Analog Sensor Pin
const int LED_PIN = 6;       // Storm Warning LED Pin
const int BUZZER_PIN = 10;   // Charybdis / Wreck Warning Piezo Buzzer Pin

// ============================================================================
// SECTION 2: OPERATIONAL THRESHOLDS & SYSTEM PARAMETERS
// ============================================================================

const int STORM_LIGHT_THRESHOLD = 500;   // Light level < 500 triggers STORM state
const float CHARYBDIS_DIST_CM = 100.0;    // Sonar distance < 100cm triggers CHARYBDIS state
const unsigned long WRECK_TIMEOUT = 5000;  // 5 seconds (5000ms) danger timeout to trigger WRECKED

// ============================================================================
// SECTION 3: SYSTEM STATES & GLOBAL VARIABLES
// ============================================================================

// Finite State Machine System States
enum SystemState {
  OPEN_SEA,
  ANCHOR_DROPPED,
  STORM,
  CHARYBDIS,
  WRECKED
};

SystemState currentState = OPEN_SEA;
SystemState previousState = OPEN_SEA;

// Safety Timer Variables
unsigned long dangerStartTime = 0;
bool dangerTimerActive = false;

// ============================================================================
// SECTION 4: HELPER FUNCTIONS & SENSOR ACQUISITION
// ============================================================================

/*
 * Reads current distance from the 4-pin HC-SR04 Ultrasonic Sensor in centimeters.
 * Sends a 10 microsecond pulse to TRIG_PIN and measures return pulse on ECHO_PIN.
 * Returns 999.0 if out of range or timeout occurs.
 */
float readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
  if (duration == 0) return 999.0;
  return duration * 0.0343 / 2.0; // Speed of sound conversion (cm/us)
}

/*
 * Controls visual (LED) and acoustic (Buzzer) indicators according to current state.
 * Uses non-blocking millis() timing for blinking effects to maintain state reactivity.
 */
void handleOutputs() {
  static unsigned long lastBlink = 0;
  static bool ledState = LOW;

  switch (currentState) {
    case OPEN_SEA:
    case ANCHOR_DROPPED:
      digitalWrite(LED_PIN, LOW);
      noTone(BUZZER_PIN);
      break;

    case STORM:
      noTone(BUZZER_PIN);
      // Non-blocking LED flashing every 250ms during Storm
      if (millis() - lastBlink >= 250) {
        lastBlink = millis();
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
      }
      break;

    case CHARYBDIS:
      digitalWrite(LED_PIN, LOW);
      tone(BUZZER_PIN, 1000); // Continuous 1kHz alert pitch for Charybdis
      break;

    case WRECKED:
      digitalWrite(LED_PIN, HIGH); // Solid ON for Wrecked state
      tone(BUZZER_PIN, 300);       // Low warning frequency for Wrecked state
      break;
  }
}

/*
 * Updates LCD display text. Executes ONLY on state transitions to prevent flicker.
 */
void updateDisplay() {
  if (currentState == previousState) return;
  previousState = currentState;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("STATE:");
  lcd.setCursor(0, 1);
  
  switch (currentState) {
    case OPEN_SEA:       lcd.print("OPEN SEA"); break;
    case ANCHOR_DROPPED: lcd.print("ANCHOR DROPPED"); break;
    case STORM:          lcd.print("STORM ALERT!"); break;
    case CHARYBDIS:      lcd.print("CHARYBDIS NEAR!"); break;
    case WRECKED:        lcd.print("** WRECKED **"); break;
  }
}

// ============================================================================
// SECTION 5: ARDUINO SETUP
// ============================================================================

void setup() {
  // Pin Mode Setup
  pinMode(SWITCH_PIN, INPUT_PULLUP); // Slide switch uses internal pull-up resistor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Screen Setup & Warmup Window
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("System Booting...");
  delay(2000); // Delay allows Tinkercad components to establish valid baseline values
  
  dangerStartTime = millis();
}

// ============================================================================
// SECTION 6: MAIN CONTROL LOOP & STATE MACHINE
// ============================================================================

void loop() {
  // Lock state machine processing if the ship is already WRECKED
  if (currentState == WRECKED) {
    handleOutputs();
    updateDisplay();
    return;
  }

  // --- 6.1: Input Reading ---
  bool isAnchorDropped = (digitalRead(SWITCH_PIN) == LOW); // Switch grounded = Anchor Dropped
  int lightLevel = analogRead(LDR_PIN);
  float distanceCM = readDistanceCM();

  // --- 6.2: Hazard Evaluation ---
  bool stormCondition = (lightLevel < STORM_LIGHT_THRESHOLD);
  bool charybdisCondition = (distanceCM > 2.0 && distanceCM < CHARYBDIS_DIST_CM);
  bool inDanger = (stormCondition || charybdisCondition);

  // --- 6.3: State Machine Logic & Priority Resolution ---
  if (isAnchorDropped) {
    currentState = ANCHOR_DROPPED;
    dangerTimerActive = false; // Anchor state resets ongoing safety timer
  } else {
    if (currentState == ANCHOR_DROPPED) {
      currentState = OPEN_SEA;
    }

    if (!inDanger) {
      currentState = OPEN_SEA;
      dangerTimerActive = false; // Safe status clears continuous safety timer
    } else {
      // Precedence: CHARYBDIS alerts take output priority over STORM
      if (charybdisCondition) {
        currentState = CHARYBDIS;
      } else if (stormCondition) {
        currentState = STORM;
      }

      // Continuous 5-Second Wreck Safety Timer Logic
      if (!dangerTimerActive) {
        dangerStartTime = millis();
        dangerTimerActive = true;
      } else if (millis() - dangerStartTime >= WRECK_TIMEOUT) {
        currentState = WRECKED;
      }
    }
  }

  // --- 6.4: Execute Hardware & Display Handlers ---
  handleOutputs();
  updateDisplay();
  delay(50); // Loop pacing
}
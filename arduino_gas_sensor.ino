/*
 * SMART CREATION - SYSTÈME D'ALERTE DE GAZ
 * Arduino Uno + Capteur de Gaz MQ-2/MQ-5
 *
 * Ce code détecte le gaz et envoie une alerte à l'application Qt
 * via communication série (USB).
 *
 * Matériel nécessaire:
 * - Arduino Uno
 * - Capteur de gaz MQ-2 ou MQ-5
 * - Écran LCD 16x2 avec I2C
 * - Buzzer actif
 * - LED rouge
 *
 * Connexions:
 * - Capteur de gaz: A0
 * - Buzzer: Pin 8
 * - LED rouge: Pin 13
 * - LCD I2C: SDA (A4), SCL (A5)
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialisation LCD I2C (adresse 0x27, 16 colonnes, 2 lignes)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pins
const int GAS_SENSOR_PIN = A0;
const int BUZZER_PIN = 8;
const int LED_PIN = 13;

// Seuil de détection de gaz (à ajuster selon votre capteur)
const int GAS_THRESHOLD = 400;

// Variables
int gasValue = 0;
bool alertActive = false;
unsigned long lastAlertTime = 0;
const unsigned long ALERT_COOLDOWN = 60000; // 60 secondes entre deux alertes

void setup() {
  // Initialisation de la communication série
  Serial.begin(9600);

  // Configuration des pins
  pinMode(GAS_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // Initialisation du LCD
  lcd.init();
  lcd.backlight();

  // Message de démarrage
  lcd.setCursor(0, 0);
  lcd.print("SMART CREATION");
  lcd.setCursor(0, 1);
  lcd.print("Capteur Gaz OK");

  Serial.println("=== SYSTÈME D'ALERTE GAZ INITIALISÉ ===");
  Serial.println("En attente de détection...");

  delay(2000);
  lcd.clear();
}

void loop() {
  // Lecture du capteur de gaz
  gasValue = analogRead(GAS_SENSOR_PIN);

  // Affichage de la valeur sur le LCD
  lcd.setCursor(0, 0);
  lcd.print("Niveau Gaz:");
  lcd.setCursor(0, 1);
  lcd.print(gasValue);
  lcd.print("  ");

  // Envoi périodique de la valeur au PC
  Serial.print("GAZ_NIVEAU:");
  Serial.println(gasValue);

  // Détection de gaz
  if (gasValue > GAS_THRESHOLD) {
    // Vérifier le cooldown pour éviter trop d'alertes
    unsigned long currentTime = millis();

    if (!alertActive || (currentTime - lastAlertTime > ALERT_COOLDOWN)) {
      triggerGasAlert();
      lastAlertTime = currentTime;
      alertActive = true;
    }

    // Maintenir le buzzer et la LED allumés
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);

  } else {
    // Niveau de gaz normal
    if (alertActive) {
      Serial.println("GAZ_NORMAL");
      alertActive = false;
    }

    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
  }

  delay(500); // Délai de 500ms entre chaque lecture
}

/**
 * Fonction déclenchée quand du gaz est détecté
 */
void triggerGasAlert() {
  Serial.println("=== ALERTE GAZ ===");
  Serial.println("GAS_ALERT"); // Message clé pour l'application Qt
  Serial.println("DANGER");    // Message alternatif
  Serial.print("Niveau détecté: ");
  Serial.println(gasValue);

  // Affichage LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("!!! DANGER !!!");
  lcd.setCursor(0, 1);
  lcd.print("GAZ DETECTE!");

  // Alarme sonore (3 bips courts)
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }

  Serial.println("=== Alerte envoyée à Smart Creation ===");
}

/**
 * Fonction de calibration du capteur
 * Appelez cette fonction au démarrage dans setup() si nécessaire
 */
void calibrateGasSensor() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibration...");

  Serial.println("Calibration du capteur de gaz en cours...");
  Serial.println("Assurez-vous que le capteur est dans un environnement sain.");

  // Attendre 20 secondes pour la calibration
  for (int i = 20; i > 0; i--) {
    lcd.setCursor(0, 1);
    lcd.print(i);
    lcd.print(" secondes  ");
    delay(1000);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibration OK");
  Serial.println("Calibration terminée!");

  delay(2000);
}

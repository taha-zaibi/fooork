/*
 * SMART CREATION - SYSTÈME D'ALERTE DE GAZ (Contrôlé par Qt)
 * Arduino Uno + Capteur de Gaz MQ-2/MQ-5
 *
 * ARCHITECTURE:
 * - Arduino: Lit le capteur et envoie la valeur à Qt via Serial
 * - Qt: Analyse la valeur et décide si c'est dangereux
 * - Qt: Envoie la commande "ALARM_ON" ou "ALARM_OFF" à Arduino
 * - Arduino: Déclenche buzzer/LCD selon les commandes de Qt
 *
 * Matériel nécessaire:
 * - Arduino Uno
 * - Capteur de gaz MQ-2 ou MQ-5
 * - Écran LCD 16x2 avec I2C (optionnel)
 * - Buzzer actif (optionnel)
 * - LED rouge (optionnel)
 *
 * Connexions:
 * - Capteur de gaz MQ-2: A0 (VCC→5V, GND→GND)
 * - Buzzer: Pin 8 (GND→GND)
 * - LED rouge: Pin 13 avec résistance 220Ω
 * - LCD I2C: SDA (A4), SCL (A5), VCC→5V, GND→GND
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialisation LCD I2C (adresse 0x27, 16 colonnes, 2 lignes)
// Si votre LCD ne fonctionne pas, essayez l'adresse 0x3F
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pins
const int GAS_SENSOR_PIN = A0;
const int BUZZER_PIN = 8;
const int LED_PIN = 13;

// Variables
int gasValue = 0;
bool alarmActive = false;
String incomingCommand = "";

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
  lcd.print("Systeme OK");

  Serial.println("=== ARDUINO PRET ===");
  Serial.println("En attente des commandes Qt...");

  delay(2000);
  lcd.clear();
}

void loop() {
  // 1. LECTURE DU CAPTEUR DE GAZ
  gasValue = analogRead(GAS_SENSOR_PIN);

  // 2. ENVOI DE LA VALEUR À QT (toutes les 500ms)
  Serial.print("GAS_LEVEL:");
  Serial.println(gasValue);

  // 3. AFFICHAGE SUR LCD
  if (alarmActive) {
    // Mode Alarme - Affichage clignotant
    lcd.setCursor(0, 0);
    lcd.print("!!! DANGER !!!");
    lcd.setCursor(0, 1);
    lcd.print("GAZ: ");
    lcd.print(gasValue);
    lcd.print("     ");
  } else {
    // Mode Normal - Affichage niveau
    lcd.setCursor(0, 0);
    lcd.print("Niveau Gaz:");
    lcd.print("    ");
    lcd.setCursor(0, 1);
    lcd.print(gasValue);
    lcd.print("  (Normal)");
  }

  // 4. ÉCOUTER LES COMMANDES DE QT
  if (Serial.available() > 0) {
    incomingCommand = Serial.readStringUntil('\n');
    incomingCommand.trim();

    // Commande d'activation de l'alarme
    if (incomingCommand == "ALARM_ON") {
      Serial.println("Commande reçue: ALARM_ON");
      activateAlarm();
    }
    // Commande de désactivation de l'alarme
    else if (incomingCommand == "ALARM_OFF") {
      Serial.println("Commande reçue: ALARM_OFF");
      deactivateAlarm();
    }
  }

  // 5. MAINTENIR L'ALARME SI ACTIVE
  if (alarmActive) {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
  }

  delay(500); // Délai de 500ms entre chaque lecture
}

/**
 * Active l'alarme (appelé par commande Qt)
 */
void activateAlarm() {
  if (!alarmActive) {
    Serial.println(">>> ACTIVATION ALARME <<<");
    alarmActive = true;

    // Séquence d'alarme: 5 bips rapides
    for (int i = 0; i < 5; i++) {
      digitalWrite(BUZZER_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }

    // Maintenir buzzer et LED allumés
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);

    // Affichage LCD d'urgence
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("!!! DANGER !!!");
    lcd.setCursor(0, 1);
    lcd.print("GAZ DETECTE!");
  }
}

/**
 * Désactive l'alarme (appelé par commande Qt)
 */
void deactivateAlarm() {
  if (alarmActive) {
    Serial.println(">>> DESACTIVATION ALARME <<<");
    alarmActive = false;

    // Éteindre buzzer et LED
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);

    // Message LCD retour à la normale
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Retour Normal");
    delay(1000);
    lcd.clear();
  }
}

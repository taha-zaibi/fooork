/*
 * SMART CREATION - TEST SIMPLE ARDUINO
 *
 * Ce code envoie automatiquement "GAS_ALERT" toutes les 10 secondes
 * pour tester la connexion avec l'application Qt SANS capteur physique.
 *
 * Matériel nécessaire:
 * - Arduino Uno seulement (connecté via USB)
 *
 * UTILISATION:
 * 1. Téléversez ce code sur l'Arduino
 * 2. Lancez l'application Qt
 * 3. Toutes les 10 secondes, un SMS sera envoyé automatiquement
 */

void setup() {
  // Initialisation de la communication série à 9600 bauds
  Serial.begin(9600);

  // Attendre que la connexion série soit établie
  delay(1000);

  Serial.println("=== SMART CREATION - TEST ARDUINO ===");
  Serial.println("Connexion établie avec succès!");
  Serial.println("Envoi d'alertes de test toutes les 10 secondes...");
  Serial.println();

  // LED intégrée (pin 13)
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // Afficher un message de statut
  Serial.println("--- Test en cours ---");
  Serial.print("Temps écoulé: ");
  Serial.print(millis() / 1000);
  Serial.println(" secondes");

  // Clignoter la LED pour montrer que ça fonctionne
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);

  // Attendre 10 secondes
  delay(10000);

  // ENVOYER L'ALERTE GAZ
  Serial.println();
  Serial.println("=== ENVOI ALERTE TEST ===");
  Serial.println("GAS_ALERT");
  Serial.println("DANGER");
  Serial.println("========================");
  Serial.println();

  // Clignoter rapidement la LED pour indiquer l'envoi
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}

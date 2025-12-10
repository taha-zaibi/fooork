# 🚨 Système d'Alerte de Gaz Arduino - Smart Creation

## Description

Ce système intègre un capteur de gaz Arduino avec l'application Smart Creation pour envoyer automatiquement des SMS d'alerte à tous les employés en cas de détection de gaz dangereux.

## 🔧 Matériel Requis

- **Arduino Uno**
- **Capteur de gaz MQ-2 ou MQ-5**
- **Écran LCD 16x2 avec module I2C**
- **Buzzer actif (5V)**
- **LED rouge**
- **Câble USB** (pour connecter Arduino au PC)
- **Breadboard et fils de connexion**

## 📌 Schéma de Connexion

### Capteur de Gaz MQ-2/MQ-5:
- VCC → 5V Arduino
- GND → GND Arduino
- A0  → A0 Arduino

### LCD I2C (16x2):
- VCC → 5V Arduino
- GND → GND Arduino
- SDA → A4 Arduino
- SCL → A5 Arduino

### Buzzer:
- Pin + → Pin 8 Arduino (avec résistance 220Ω)
- Pin - → GND Arduino

### LED Rouge:
- Anode (+) → Pin 13 Arduino (avec résistance 220Ω)
- Cathode (-) → GND Arduino

## 💻 Installation Arduino

### 1. Installation des bibliothèques

Ouvrez l'IDE Arduino et installez:
- **LiquidCrystal_I2C** (Outils → Gérer les bibliothèques → Rechercher "LiquidCrystal I2C")
- **Wire** (bibliothèque incluse par défaut)

### 2. Upload du code

1. Ouvrez le fichier `arduino_gas_sensor.ino` dans l'IDE Arduino
2. Sélectionnez votre carte: **Outils → Type de carte → Arduino Uno**
3. Sélectionnez le port COM: **Outils → Port → COM X**
4. Cliquez sur **Téléverser** (→)

### 3. Calibration du capteur

**⚠️ IMPORTANT**: Les capteurs MQ-2/MQ-5 nécessitent une période de chauffe de 24-48h pour une précision optimale.

- Placez le capteur dans un environnement sain (sans gaz)
- Attendez que la lecture se stabilise
- Notez la valeur de base (généralement entre 100-300)
- Ajustez `GAS_THRESHOLD` dans le code (valeur de base + 100-200)

## 🖥️ Configuration Qt Application

### Code déjà intégré:

Le système d'alerte a été ajouté à l'application Smart Creation:

1. **Communication série automatique**: L'application détecte automatiquement l'Arduino au démarrage
2. **Détection d'alerte**: Écoute les messages "GAS_ALERT" ou "GAZ_DETECTE" de l'Arduino
3. **Envoi SMS automatique**: Envoie des alertes à tous les employés de la base de données

### Fichiers modifiés:

- `smart_cretion.pro` - Ajout du module serialport
- `smart_creation.h` - Déclarations des fonctions Arduino
- `smart_creation.cpp` - Implémentation complète du système

## 🚀 Utilisation

### Démarrage automatique:

1. Connectez l'Arduino au PC via USB
2. Lancez l'application Smart Creation
3. L'Arduino sera détecté automatiquement
4. Le système est maintenant actif!

### Test manuel:

Si vous voulez tester le système sans Arduino:

1. Dans l'interface Qt, ajoutez un bouton avec `objectName` = `btn_test_gas_alert`
2. Cliquez sur ce bouton pour simuler une alerte
3. Confirmez l'envoi des SMS de test

### Fonctionnement:

Quand du gaz est détecté:
1. ✅ Arduino affiche "GAZ DETECTE!" sur le LCD
2. ✅ Buzzer émet 3 bips d'alarme
3. ✅ LED rouge s'allume
4. ✅ Message "GAS_ALERT" envoyé via série
5. ✅ Application Qt reçoit l'alerte
6. ✅ SMS envoyés automatiquement à tous les employés
7. ✅ Message de confirmation affiché

## 📱 Format du SMS d'Alerte

```
🚨 ALERTE URGENTE! 🚨
Un capteur de gaz a détecté un danger dans les locaux de Smart Creation.
Veuillez évacuer immédiatement les lieux et contacter les secours!
⚠️ NE PAS RETOURNER AVANT L'AUTORISATION ⚠️
```

## 🔍 Dépannage

### Arduino non détecté:
- Vérifiez que le câble USB est bien connecté
- Vérifiez dans le Gestionnaire de périphériques (Windows) que l'Arduino apparaît
- Essayez un autre port USB
- Réinstallez les drivers Arduino

### Pas de lecture du capteur:
- Vérifiez les connexions (VCC, GND, A0)
- Attendez 2-3 minutes de chauffe du capteur
- Vérifiez avec le moniteur série Arduino (9600 bauds)

### SMS non envoyés:
- Vérifiez les identifiants Twilio dans `smart_creation.h`
- Vérifiez que les numéros d'employés sont au format tunisien (8 chiffres)
- Vérifiez votre connexion Internet

### Fausses alertes:
- Augmentez la valeur de `GAS_THRESHOLD` dans le code Arduino
- Éloignez le capteur des sources de chaleur
- Assurez-vous d'une bonne ventilation

## ⚙️ Configuration Avancée

### Ajuster le seuil de détection:

Dans `arduino_gas_sensor.ino`, ligne 28:
```cpp
const int GAS_THRESHOLD = 400; // Modifiez cette valeur
```

### Modifier le cooldown des alertes:

Ligne 34:
```cpp
const unsigned long ALERT_COOLDOWN = 60000; // 60 secondes
```

### Personnaliser le message SMS:

Dans `smart_creation.cpp`, fonction `sendGasAlertToAllEmployees()`, ligne 2383:
```cpp
QString alertMessage = "Votre message personnalisé";
```

## 📊 Logs et Debug

### Moniteur série Arduino:

Ouvrez le moniteur série (115200 bauds) pour voir:
- Niveau de gaz en temps réel
- Messages d'alerte
- État du système

### Console Qt (qDebug):

L'application affiche:
- Port Arduino détecté
- Messages reçus de l'Arduino
- Nombre de SMS envoyés
- Erreurs éventuelles

## 🔐 Sécurité

**⚠️ AVERTISSEMENT**: Ce système est conçu à des fins éducatives et de démonstration.

Pour un usage professionnel:
- Utilisez des capteurs certifiés industriels
- Ajoutez une alimentation de secours (batterie)
- Prévoyez une redondance (plusieurs capteurs)
- Intégrez avec un système d'alarme professionnel
- Testez régulièrement le système

## 📝 Notes

- Les capteurs MQ-2/MQ-5 détectent: GPL, méthane, butane, propane, fumée
- Sensibilité différente selon le type de gaz
- Précision ±50ppm (parties par million)
- Temps de réponse: < 10 secondes

## 🆘 Support

Pour toute question ou problème:
1. Vérifiez d'abord cette documentation
2. Consultez les logs Arduino et Qt
3. Testez chaque composant séparément
4. Vérifiez les connexions physiques

## 📜 Licence

Code fourni pour Smart Creation - Projet académique

#ifndef TWILIO_H
#define TWILIO_H

#include <string>
#include <vector>
#include <QDate>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

struct Influenceur {
    std::string nom;
    int telephone;
    QDate date_rdv;
};

class TwilioSMS : public QObject
{
    Q_OBJECT

private:
    std::string account_sid = "ACa8587dc430e794ca7e06906e6751d980";
    std::string api_key_sid = "SK17a395e96c984f58142f98536b156621";
    std::string api_key_secret = "p4AEmCSNwnMhRmiGMWxg2PpD5ewNE6fc";
    std::string from_number = "+1586354356";
    QNetworkAccessManager *networkManager;

public:
    TwilioSMS(QObject *parent = nullptr);
    ~TwilioSMS();

    bool sendSMS(const std::string& to_number, const std::string& message);

    // Méthode pour envoyer un message de bienvenue le jour du rendez-vous
    bool sendWelcomeMessage(int patient_phone_int, const std::string& patient_name = "");

    // Méthode qui parcourt la base de données et envoie les messages de bienvenue pour aujourd'hui
    void checkAndSendWelcomeMessages(const std::vector<Influenceur>& influenceurs);

    // Méthode pour convertir un INT en format international string
    std::string convertIntToInternational(int phone_int);

    // Méthode pour vérifier si c'est un numéro valide (commence par 55)
    bool isTunisianMobileNumber(int phone_int);

private slots:
    void onReplyFinished(QNetworkReply *reply);
};

#endif

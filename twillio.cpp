#include "twillio.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QByteArray>
#include <QEventLoop>
#include <QTimer>
#include <QDebug>

TwilioSMS::TwilioSMS(QObject *parent) : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &TwilioSMS::onReplyFinished);
}

TwilioSMS::~TwilioSMS()
{
    if (networkManager) {
        delete networkManager;
        networkManager = nullptr;
    }
}

bool TwilioSMS::sendSMS(const std::string& to_number, const std::string& message)
{
    // Préparer l'URL
    QUrl url(QString::fromStdString("https://api.twilio.com/2010-04-01/Accounts/" + account_sid + "/Messages.json"));

    // Préparer les données POST
    QUrlQuery postData;
    postData.addQueryItem("To", QString::fromStdString(to_number));
    postData.addQueryItem("From", QString::fromStdString(from_number));
    postData.addQueryItem("Body", QString::fromStdString(message));

    // Préparer la requête
    QNetworkRequest request(url);

    // Authentification Basic avec API Key
    QString auth = QString::fromStdString(api_key_sid + ":" + api_key_secret);
    QByteArray authData = auth.toUtf8().toBase64();
    request.setRawHeader("Authorization", "Basic " + authData);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    // Créer une boucle d'événements pour attendre la réponse (approche synchrone)
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    bool success = false;

    auto connection = connect(networkManager, &QNetworkAccessManager::finished,
                              [&](QNetworkReply *reply) {
                                  if (reply->error() == QNetworkReply::NoError) {
                                      std::cout << "SMS envoye avec succes a: " << to_number << std::endl;
                                      success = true;
                                  } else {
                                      std::cerr << "Erreur Twilio: " << reply->errorString().toStdString() << std::endl;
                                      QByteArray response = reply->readAll();
                                      std::cerr << "Reponse: " << response.toStdString() << std::endl;
                                      success = false;
                                  }
                                  reply->deleteLater();
                                  loop.quit();
                              });

    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(30000); // Timeout de 30 secondes

    networkManager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());

    loop.exec();

    disconnect(connection);

    return success;
}

void TwilioSMS::onReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();
}

std::string TwilioSMS::convertIntToInternational(int phone_int) {
    std::stringstream ss;
    ss << "+216" << std::setw(8) << std::setfill('0') << phone_int;
    return ss.str();
}

bool TwilioSMS::isTunisianMobileNumber(int phone_int) {

    return (phone_int >= 20000000 && phone_int <= 99999999);
}

bool TwilioSMS::sendWelcomeMessage(int patient_phone_int, const std::string& patient_name) {
    if (!isTunisianMobileNumber(patient_phone_int)) {
        std::cerr << "Numero non tunisien mobile ignore: " << patient_phone_int << std::endl;
        return false;
    }

    // Convertir le INT en format international
    std::string patient_phone = convertIntToInternational(patient_phone_int);

    std::string message;
    if (patient_name.empty()) {
        message = "Bonjour! Soyez le bienvenue, vous avez un rendez-vous avec nous aujourd'hui.";
    } else {
        message = "Bonjour " + patient_name + "! Soyez le bienvenue, vous avez un rendez-vous avec nous aujourd'hui.";
    }

    return sendSMS(patient_phone, message);
}

void TwilioSMS::checkAndSendWelcomeMessages(const std::vector<Influenceur>& influenceurs) {
    QDate today = QDate::currentDate();
    std::cout << "Recherche des rendez-vous pour aujourd'hui: " << today.toString("yyyy-MM-dd").toStdString() << std::endl;

    int messages_sent = 0;
    int errors = 0;

    for (const auto& influenceur : influenceurs) {
        if (influenceur.date_rdv == today) {
            std::cout << "Rendez-vous aujourd'hui pour: " << influenceur.nom
                      << " (" << influenceur.telephone << ")" << std::endl;

            if (sendWelcomeMessage(influenceur.telephone, influenceur.nom)) {
                messages_sent++;
            } else {
                errors++;
                std::cerr << "Erreur lors de l'envoi du message de bienvenue a: "
                          << influenceur.nom << std::endl;
            }
        }
    }

    std::cout << "Operation terminee. " << messages_sent
              << " messages de bienvenue envoyes, " << errors << " erreurs." << std::endl;
}

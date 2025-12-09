#include "chatbot.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QTimer>

const QString& getApiUrl() {
    static const QString url = "https://api.groq.com/openai/v1/chat/completions";
    return url;
}

const QString& getApiKey() {
    static const QString key = "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";
    return key;
}

const QString& getModel() {
    static const QString model = "llama-3.1-8b-instant";
    return model;
}

Chatbot::Chatbot(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &Chatbot::onReplyFinished);
}

Chatbot::~Chatbot()
{
}

void Chatbot::sendMessage(const QString &message)
{
    if (message.isEmpty()) {
        emit errorOccurred("Le message ne peut pas être vide");
        emit processingFinished();
        return;
    }

    m_lastMessage = message;

    QString msg = message.toLower().trimmed();
    if (msg.contains("interface") &&
        (msg.contains("influenceur") || msg.contains("matériel") ||
         msg.contains("personnel") || msg.contains("vidéo") ||
         msg.contains("sponsors"))) {

        QString fallbackResponse = getFallbackResponse(message);
        QTimer::singleShot(100, [this, fallbackResponse]() {
            emit responseReceived(fallbackResponse);
            emit processingFinished();
        });
        return;
    }
    QJsonObject jsonRequest;
    jsonRequest["model"] = getModel();

    QJsonArray messages;

    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = "Tu es un assistant IA utile et intelligent. Réponds de manière naturelle et conversationnelle en français. Sois précis et concis. Réponds toujours en français.";
    messages.append(systemMessage);

    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = message;
    messages.append(userMessage);

    jsonRequest["messages"] = messages;
    jsonRequest["max_tokens"] = 2048;
    jsonRequest["temperature"] = 0.7;
    jsonRequest["stream"] = false;
    jsonRequest["top_p"] = 0.9;

    QJsonDocument doc(jsonRequest);
    QByteArray jsonData = doc.toJson();

    QNetworkRequest request((QUrl(getApiUrl())));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(getApiKey()).toUtf8());
    request.setRawHeader("User-Agent", "SmartCreationApp/1.0");
    request.setRawHeader("Accept", "application/json");

    qDebug() << "Envoi de la requête à Groq API...";
    qDebug() << "Modèle:" << getModel();
    qDebug() << "Message:" << message;

    QNetworkReply *reply = m_networkManager->post(request, jsonData);
    QTimer::singleShot(15000, reply, &QNetworkReply::abort);
}

void Chatbot::onReplyFinished(QNetworkReply *reply)
{
    auto ensureCleanup = [this, reply]() {
        emit processingFinished();
        reply->deleteLater();
        qDebug() << "=== CHATBOT CLEANUP COMPLETE - processingFinished Émis ===";
    };

    if (reply->error() != QNetworkReply::NoError) {
        QString errorString = reply->errorString();
        qDebug() << "Erreur réseau:" << errorString;

        QByteArray errorData = reply->readAll();
        QString detailedError = errorString;

        if (!errorData.isEmpty()) {
            qDebug() << "Données d'erreur:" << errorData;
            QJsonParseError parseError;
            QJsonDocument errorDoc = QJsonDocument::fromJson(errorData, &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                QJsonObject errorObj = errorDoc.object();
                if (errorObj.contains("error")) {
                    QJsonObject apiError = errorObj["error"].toObject();
                    if (apiError.contains("message")) {
                        detailedError = apiError["message"].toString();
                    }
                }
            } else {
                detailedError += "\n" + QString(errorData);
            }
        }

        emit errorOccurred("Erreur: " + detailedError);
        ensureCleanup();
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray data = reply->readAll();

    if (statusCode != 200) {
        qDebug() << "Erreur HTTP:" << statusCode;
        qDebug() << "Réponse:" << data;

        QString errorMsg = QString("Erreur HTTP %1").arg(statusCode);
        if (!data.isEmpty()) {
            QJsonParseError parseError;
            QJsonDocument errorDoc = QJsonDocument::fromJson(data, &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                QJsonObject errorObj = errorDoc.object();
                if (errorObj.contains("error")) {
                    QJsonObject apiError = errorObj["error"].toObject();
                    if (apiError.contains("message")) {
                        errorMsg = apiError["message"].toString();
                    }
                }
            }
        }

        emit errorOccurred("Erreur API: " + errorMsg);
        ensureCleanup();
        return;
    }

    QString response = parseApiResponse(data);

    if (!response.isEmpty()) {
        emit responseReceived(response);
    } else {
        emit errorOccurred("Impossible de traiter la réponse de l'API");
    }

    ensureCleanup();
}

QString Chatbot::parseApiResponse(const QByteArray &data)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "Erreur JSON:" << parseError.errorString();
        return QString();
    }

    QJsonObject obj = doc.object();

    // Vérifier les erreurs API
    if (obj.contains("error")) {
        QJsonObject errorObj = obj["error"].toObject();
        QString errorMsg = errorObj["message"].toString();
        qDebug() << "Erreur API:" << errorMsg;
        return "❌ **Erreur API:** " + errorMsg;
    }

    // Extraire la réponse Groq
    if (obj.contains("choices")) {
        QJsonArray choices = obj["choices"].toArray();
        if (!choices.isEmpty()) {
            QJsonObject firstChoice = choices[0].toObject();
            if (firstChoice.contains("message")) {
                QJsonObject message = firstChoice["message"].toObject();
                if (message.contains("content")) {
                    QString response = message["content"].toString().trimmed();
                    qDebug() << "Réponse Groq reçue:" << response;

                    // Statistiques de l'API (optionnel)
                    if (obj.contains("usage")) {
                        QJsonObject usage = obj["usage"].toObject();
                        int promptTokens = usage["prompt_tokens"].toInt();
                        int completionTokens = usage["completion_tokens"].toInt();
                        qDebug() << "Tokens utilisés - Prompt:" << promptTokens << "Completion:" << completionTokens;
                    }

                    return response;
                }
            }
        }
    }

    qDebug() << "Format de réponse non reconnu";
    qDebug() << "Réponse brute:" << data;
    return QString();
}

QString Chatbot::getFallbackResponse(const QString &message)
{
    QString msg = message.toLower().trimmed();

    if (msg.contains("interface influenceur")) {
        return "Dans l'interface d'influenceur il y a :\n"
               "- Formulaire d'inscription\n"
               "- Barre de recherche selon l'id de l'influenceur \n"
               "- Trie Selon Nombre de suivres \n"
               "- Exportation en PDF \n"
               "- Modification les données \n"
               "- Statistique\n";
    }
    else if (msg.contains("interface matériel")) {
        return "Dans l'interface Materiel il y a :\n"
               "- Formulaire d'inscription\n"
               "- Barre de recherche  \n"
               "- Trie \n"
               "- Exportation en PDF \n"
               "- Modification les données \n"
               "- Statistique\n";
    }
    else if (msg.contains("interface personnel")) {
        return "Dans l'interface Personnel il y a :\n"
               "- Formulaire d'inscription\n"
               "- Barre de recherche  \n"
               "- Trie \n"
               "- Exportation en PDF \n"
               "- Modification les données \n"
               "- Statistique\n";
    }
    else if (msg.contains("interface video")) {
        return "Dans l'interface Vidéo il y a :\n"
               "- Formulaire d'inscription\n"
               "- Barre de recherche  \n"
               "- Trie \n"
               "- Exportation en PDF \n"
               "- Modification les données \n"
               "- Statistique\n";
    }
    else if (msg.contains("interface sponsors")) {
        return "Dans l'interface Sponsors il y a :\n"
               "- Formulaire d'inscription\n"
               "- Barre de recherche  \n"
               "- Trie \n"
               "- Exportation en PDF \n"
               "- Modification les données \n"
               "- Statistique\n";
    }

    return "Je ne peux pas répondre à cette question pour le moment";
}

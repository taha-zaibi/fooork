#ifndef CHATBOT_H
#define CHATBOT_H

#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

class Chatbot : public QObject
{
    Q_OBJECT

public:
    explicit Chatbot(QObject *parent = nullptr);
    ~Chatbot();

    void sendMessage(const QString &message);

signals:
    void responseReceived(const QString &response);
    void errorOccurred(const QString &error);
    void processingFinished();

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_networkManager;
    QString parseApiResponse(const QByteArray &data);
    QString getFallbackResponse(const QString &message);
    QString m_lastMessage;


};

#endif // CHATBOT_H

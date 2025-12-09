#ifndef ML_PREDICTOR_H
#define ML_PREDICTOR_H

#include <QDialog>
#include <QWidget>
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <QDate>
#include <QColor>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>

// Structure pour stocker les données de prédiction
struct VideoPrediction {
    int id;
    QString nom;
    int vues;
    int likes;
    double cout;
    QString categorie;
    QDate date;
    double tauxEngagement; // (vues + likes) / cout
    double scoreViral; // Vues * Likes / cout
    int vuesPredites;
    int likesPredits;
    QString tendance; // "🔥 En hausse", "📈 Stable", "📉 En déclin"
    QString alerte; // "VIRAL", "TENDANCE", "NORMAL", "ATTENTION"
    QColor couleurTendance;
};

class MLPredictor : public QDialog
{
    Q_OBJECT

public:
    explicit MLPredictor(QWidget *parent = nullptr);
    ~MLPredictor();

    // Méthode principale pour afficher le prédicteur
    void afficher();

private:
    // Méthodes privées
    QList<VideoPrediction> calculerPredictions();
    void appliquerModelePredictif(QList<VideoPrediction> &predictions,
                                  double moyenneVues,
                                  double moyenneLikes,
                                  double tauxCroissanceMoyen);
    void creerInterface(const QList<VideoPrediction> &predictions);
    void remplirTableau(QTableWidget *table, const QList<VideoPrediction> &predictions);
    QString genererStatistiquesHTML(const QList<VideoPrediction> &predictions,
                                    int totalVues,
                                    int videosVirales,
                                    int videosTendance,
                                    int videosAttention);

    // Widgets
    QTableWidget *tablePred;
    QLabel *statsLabel;
    QPushButton *btnFermer;
};

#endif // ML_PREDICTOR_H

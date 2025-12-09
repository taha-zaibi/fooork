#ifndef ROI_ANALYZER_H
#define ROI_ANALYZER_H

#include <QDialog>
#include <QWidget>
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>

// Structure pour stocker les données de ROI
struct VideoROI {
    int id;
    QString nom;
    int vues;
    int likes;
    double cout;
    QString categorie;
    int engagement;
    double roi;
    double cpv;
    double cpl;
};

class ROIAnalyzer : public QDialog
{
    Q_OBJECT

public:
    explicit ROIAnalyzer(QWidget *parent = nullptr);
    ~ROIAnalyzer();

    // Méthode principale pour afficher l'analyseur
    void afficher();

private:
    // Méthodes privées
    QList<VideoROI> calculerMetriques();
    void creerInterface(const QList<VideoROI> &videosROI);
    void remplirTableau(QTableWidget *table, const QList<VideoROI> &videosROI);
    QString genererStatistiquesHTML(const QList<VideoROI> &videosROI);

    // Widgets
    QTableWidget *tableROI;
    QLabel *statsLabel;
    QPushButton *btnFermer;
};

#endif // ROI_ANALYZER_H

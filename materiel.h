#ifndef MATERIEL_H
#define MATERIEL_H

#include <QString>
#include <QDate>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDebug>
#include <QTableView>
#include <QMap>
class Materiel
{
private:
    int ID_MATERIEL;
    QString NOM;
    QString TYPE;
    QString STATUT;
    int QTE_DISPO;
    QDate DATE_ACHAT;
    double PRIX;


public:
    Materiel();
    Materiel(int, QString, QString, QString, int, QDate, double);
    int getID_MATERIEL() { return ID_MATERIEL; }
    QString getNOM() { return NOM; }
    QString getTYPE() { return TYPE; }
    QString getSTATUT() { return STATUT; }
    int getQTE_DISPO() { return QTE_DISPO; }
    QDate getDATE_ACHAT() { return DATE_ACHAT; }
    double getPRIX() { return PRIX; }
    void setID_MATERIEL(int id) { ID_MATERIEL = id; }
    void setNOM(QString n) { NOM = n; }
    void setTYPE(QString t) { TYPE = t; }
    void setSTATUT(QString s) { STATUT = s; }
    void setQTE_DISPO(int q) { QTE_DISPO = q; }
    void setDATE_ACHAT(QDate d) { DATE_ACHAT = d; }
    void setPRIX(double p) { PRIX = p; }
    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(int);
    bool modifier();
    bool exporterPDF(QTableView *view, const QString &fileName);
    QSqlQueryModel* rechercher(const QString& idRecherche);
    QSqlQueryModel* trier(const QString& colonne, const QString& ordre);
    QMap<QString, int> compter_statut();
    QString getListeMaterielPourIA();
    QSqlQuery suggerer_materiel_remplacement(const QString& id_materiel_reserve);
    QString genererNotificationsMaintenance();
    static int getDureeMaintenance(const QString& type);
};
#endif // MATERIEL_H

#ifndef GPERSONNEL_H
#define GPERSONNEL_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDate>

class gpersonnel
{
private:
    int id_perso;
    QString email;
    QString mot_de_passe;
    QString nom;
    QString prenom;
    float salaire;
    int num_tel;
    QDate date_naissance;
    QString genre;
    QString domaine;
    QString nom_pere;
    QString color_pref;
    QString animal_pref;

public:
    gpersonnel();
    gpersonnel(int id_perso, QString email, QString mot_de_passe,
               QString nom, QString prenom, float salaire, int num_tel,
               QDate date_naissance, QString genre, QString domaine,
               QString nom_pere, QString color_pref, QString animal_pref);

    bool ajouter();
    bool supprimer(int id);
    bool modifier(int id_perso, QString email, QString mot_de_passe,
                  QString nom, QString prenom, float salaire, int num_tel,
                  QDate date_naissance, QString genre, QString domaine,
                  QString nom_pere, QString color_pref, QString animal_pref);

    QSqlQueryModel* afficher();
    bool existeID(int id);
    bool existeEmail(QString email);
    bool existeNum(int num);
    QSqlQueryModel* rechercher(QString texte);
    QSqlQueryModel* trier(QString critere);
    QSqlQueryModel* statistiquesDomaine();

    bool verifierConnexion(const QString &email, const QString &mdp,
                           int &id_out, QString &nom_out, QString &prenom_out);

    bool verifierQuestions(QString email,
                           QString color,
                           QString animal,
                           QString pere);
    bool changerMdp(QString email, QString nouveau_mdp);


};

#endif // GPERSONNEL_H

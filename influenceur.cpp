#include "influenceur.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

influenceur::influenceur() {}

influenceur::influenceur(int x,QString n, QString p, QString eml, QString usr, int tel, int suivre, QString gnr)
{
    id=x;
    nom=n;
    prenom=p;
    email=eml;
    username=usr;
    num_tel=tel;
    nb_suiv=suivre;
    genre=gnr;
}

bool influenceur::ajouter()
{
    QSqlQuery query;
    QString Id= QString::number(id);
    QString tel= QString::number(num_tel);
    QString suivre= QString::number(nb_suiv);
    query.prepare("INSERT INTO INFLUENCEUR (ID_INF,NOM_INF, PRENOM_INF, EMAIL_INF, USERNAME_INF, NUM_TEL_INF, NB_SUIV_INF, GENRE_INF) "
                  "VALUES (:id, :nom, :prenom, :email, :username, :num_tel, :nb_suiv, :genre)");
    query.bindValue(":id", Id);
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":email", email);
    query.bindValue(":username", username);
    query.bindValue(":num_tel", tel);
    query.bindValue(":nb_suiv", suivre);
    query.bindValue(":genre", genre);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text();
        qDebug() << "Query:" << query.lastQuery();
        return false;
    }
    return true;
}

QSqlQueryModel* influenceur::afficher()
{
    if (!QSqlDatabase::database().isOpen()) {
        qDebug() << "Database not open!";
        return nullptr;
    }
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT * FROM INFLUENCEUR");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Prenom"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Email"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Username"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Telephone"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Nb_Suivre"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Date_RDV"));
    model->setHeaderData(8, Qt::Horizontal, QObject::tr("Genre"));

    return model;
}
bool influenceur::supprimer(int id)
{
    QSqlQuery query;
    QString res=QString::number(id);
    query.prepare("Delete from influenceur where ID_INF=:id");
    query.bindValue(":id",res);
    return query.exec();
}

bool influenceur::modifier(int x, QString n, QString p, QString eml, QString usr, int tel, int suivre, QString gnr)
{
    QSqlQuery query;
    query.prepare("UPDATE influenceur SET nom_inf = :n, prenom_inf = :p, email_inf = :eml, username_inf = :usr, num_tel_inf = :tel, nb_suiv_inf = :suivre, genre_inf = :gnr WHERE id_inf = :x");
    query.addBindValue(n);
    query.addBindValue(p);
    query.addBindValue(eml);
    query.addBindValue(usr);
    query.addBindValue(tel);
    query.addBindValue(suivre);
    query.addBindValue(gnr);
    query.addBindValue(x);

    return query.exec();
}
QSqlQueryModel* influenceur::rechercher(const int& idRecherche)
{

    QSqlQueryModel* model = new QSqlQueryModel();
    QSqlQuery query;
    query.prepare("SELECT * FROM INFLUENCEUR WHERE ID_INF = :id");
    query.bindValue(":id", idRecherche);

    if (query.exec()) {
        model->setQuery(std::move(query));
        model->setHeaderData(0, Qt::Horizontal, "ID");
        model->setHeaderData(1, Qt::Horizontal, "Nom");
        model->setHeaderData(2, Qt::Horizontal, "Prenom");
        model->setHeaderData(3, Qt::Horizontal, "Email");
        model->setHeaderData(4, Qt::Horizontal, "Username");
        model->setHeaderData(5, Qt::Horizontal, "Telephone");
        model->setHeaderData(6, Qt::Horizontal, "NB_Suivre");
        model->setHeaderData(7, Qt::Horizontal, "Date_RDV");
        model->setHeaderData(8, Qt::Horizontal, "Genre");
        return model;
    } else {
        qDebug() << "Erreur SQL lors de la recherche :" << query.lastError().text();
        delete model;
        return nullptr;
    }
}
QSqlQueryModel* influenceur::trier()
{
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT * FROM INFLUENCEUR ORDER BY NB_SUIV_INF ASC");

    if (model->lastError().isValid()) {
        qDebug() << "Erreur SQL lors du tri :" << model->lastError().text();
        delete model;
        return nullptr;
    }
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Nom");
    model->setHeaderData(2, Qt::Horizontal, "Prenom");
    model->setHeaderData(3, Qt::Horizontal, "Email");
    model->setHeaderData(4, Qt::Horizontal, "Username");
    model->setHeaderData(5, Qt::Horizontal, "Telephone");
    model->setHeaderData(6, Qt::Horizontal, "NB_Suivre");
    model->setHeaderData(7, Qt::Horizontal, "Date_RDV");
    model->setHeaderData(8, Qt::Horizontal, "Genre");

    return model;
}

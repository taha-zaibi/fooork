#include "video.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

Video::Video()
{
    id_vid = 0;
    nom_video = "";
    nb_vues = 0;
    nb_likes = 0;
    date_vd = QDate::currentDate();
    categorie = "";
    cout = 0.0;
}

Video::Video(int id, QString nom, int vues, int likes, QDate date, QString cat, double cout_val)
{
    this->id_vid = id;
    this->nom_video = nom;
    this->nb_vues = vues;
    this->nb_likes = likes;
    this->date_vd = date;
    this->categorie = cat;
    this->cout = cout_val;
}

// =============== AJOUTER ===============
bool Video::ajouter()
{
    QSqlQuery query;

    // Requête préparée adaptée pour Oracle avec nouveau schéma
    query.prepare("INSERT INTO VIDEO (ID_VID, NOM_VID, CATEGORIE_VID, NB_VUE_VID, NB_LIKE_VID, DATE_VD, COUT_VID) "
                  "VALUES (:id, :nom, :cat, :vues, :likes, :date, :cout)");

    // Liaison des valeurs
    query.bindValue(":id", id_vid);
    query.bindValue(":nom", nom_video);
    query.bindValue(":cat", categorie);
    query.bindValue(":vues", nb_vues);
    query.bindValue(":likes", nb_likes);
    query.bindValue(":date", date_vd);
    query.bindValue(":cout", cout);

    // Exécution
    if(query.exec())
    {
        qDebug() << "Vidéo ajoutée avec succès - ID:" << id_vid;
        return true;
    }
    else
    {
        qDebug() << "Erreur d'ajout:" << query.lastError().text();
        return false;
    }
}

// =============== AFFICHER ===============
QSqlQueryModel* Video::afficher()
{
    QSqlQueryModel* model = new QSqlQueryModel();

    // Requête adaptée pour Oracle avec nouveau schéma
    model->setQuery("SELECT ID_VID, NOM_VID, CATEGORIE_VID, NB_VUE_VID, NB_LIKE_VID, "
                    "TO_CHAR(DATE_VD, 'DD/MM/YYYY') as DATE_VD, COUT_VID "
                    "FROM VIDEO ORDER BY ID_VID");

    // Définir les en-têtes des colonnes
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Id_video"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom_video"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Categorie"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Vues"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Likes"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Date"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Cout"));

    return model;
}

// =============== SUPPRIMER ===============
bool Video::supprimer(int id)
{
    QSqlQuery query;

    // D'abord supprimer les relations dans PRODUIRE (clé étrangère)
    query.prepare("DELETE FROM PRODUIRE WHERE ID_VD = :id");
    query.bindValue(":id", id);

    if(!query.exec())
    {
        qDebug() << "Avertissement lors de la suppression des relations PRODUIRE:" << query.lastError().text();
        // On continue quand même car il se peut qu'il n'y ait pas de relation
    }

    // Supprimer les relations dans COLLABORER (si elles existent)
    query.prepare("DELETE FROM COLLABORER WHERE ID_VD = :id");
    query.bindValue(":id", id);
    query.exec(); // On ignore les erreurs

    // Supprimer les relations dans REALISER (si elles existent)
    query.prepare("DELETE FROM REALISER WHERE ID_VIDEO = :id");
    query.bindValue(":id", id);
    query.exec(); // On ignore les erreurs

    // Supprimer les relations dans NECESSITER (si elles existent)
    query.prepare("DELETE FROM NECESSITER WHERE ID_VD = :id");
    query.bindValue(":id", id);
    query.exec(); // On ignore les erreurs

    // Maintenant supprimer la vidéo
    query.prepare("DELETE FROM VIDEO WHERE ID_VID = :id");
    query.bindValue(":id", id);

    if(query.exec())
    {
        qDebug() << "Vidéo supprimée avec succès - ID:" << id;
        return true;
    }
    else
    {
        qDebug() << "Erreur de suppression:" << query.lastError().text();
        return false;
    }
}

// =============== MODIFIER ===============
bool Video::modifier()
{
    QSqlQuery query;

    // Requête préparée avec nouveau schéma
    query.prepare("UPDATE VIDEO SET NOM_VID=:nom, CATEGORIE_VID=:cat, NB_VUE_VID=:vues, "
                  "NB_LIKE_VID=:likes, DATE_VD=:date, COUT_VID=:cout WHERE ID_VID=:id");

    query.bindValue(":id", id_vid);
    query.bindValue(":nom", nom_video);
    query.bindValue(":cat", categorie);
    query.bindValue(":vues", nb_vues);
    query.bindValue(":likes", nb_likes);
    query.bindValue(":date", date_vd);
    query.bindValue(":cout", cout);

    if(query.exec())
    {
        qDebug() << "Vidéo modifiée avec succès - ID:" << id_vid;
        return true;
    }
    else
    {
        qDebug() << "Erreur de modification:" << query.lastError().text();
        return false;
    }
}

// =============== RECHERCHER ===============
QSqlQueryModel* Video::rechercher(QString critere)
{
    QSqlQueryModel* model = new QSqlQueryModel();
    QSqlQuery query;

    // Recherche par nom ou ID avec nouveau schéma
    query.prepare("SELECT ID_VID, NOM_VID, CATEGORIE_VID, NB_VUE_VID, NB_LIKE_VID, "
                  "TO_CHAR(DATE_VD, 'DD/MM/YYYY') as DATE_VD, COUT_VID "
                  "FROM VIDEO WHERE UPPER(NOM_VID) LIKE :critere OR TO_CHAR(ID_VID) LIKE :critere "
                  "ORDER BY ID_VID");

    query.bindValue(":critere", "%" + critere.toUpper() + "%");
    query.exec();

    model->setQuery(std::move(query));

    // En-têtes
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Id_video"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom_video"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Categorie"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Vues"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Likes"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Date"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Cout"));

    return model;
}

// =============== VÉRIFIER EXISTENCE ===============
bool Video::existe(int id)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM VIDEO WHERE ID_VID = :id");
    query.bindValue(":id", id);

    if(query.exec() && query.next())
    {
        return query.value(0).toInt() > 0;
    }

    return false;
}

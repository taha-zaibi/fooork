#include "gpersonnel.h"
#include <QSqlError>
#include <QDebug>

gpersonnel::gpersonnel()
{
    id_perso = 0;
    salaire = 0.0f;
    num_tel = 0;
}

gpersonnel::gpersonnel(int id_perso, QString email, QString mot_de_passe,
                       QString nom, QString prenom, float salaire,
                       int num_tel, QDate date_naissance, QString genre,
                       QString domaine, QString nom_pere,
                       QString color_pref, QString animal_pref)
{
    this->id_perso = id_perso;
    this->email = email;
    this->mot_de_passe = mot_de_passe;
    this->nom = nom;
    this->prenom = prenom;
    this->salaire = salaire;
    this->num_tel = num_tel;
    this->date_naissance = date_naissance;
    this->genre = genre;
    this->domaine = domaine;
    this->nom_pere = nom_pere;
    this->color_pref = color_pref;
    this->animal_pref = animal_pref;
}

/*===============================
        AJOUT
================================*/
bool gpersonnel::ajouter()
{
    QSqlQuery query;

    query.prepare(
        "INSERT INTO PERSONNEL "
        "(ID_PERSO, EMAIL_PERSO, MOT_DE_PASSE_PERSO, NOM_PERSO, PRENOM_PERSO, "
        "SALAIRE_PERSO, NUM_TEL_PERSO, DATE_NAISSANCE_PERSO, GENRE_PERSO, "
        "DOMAINE_PERSO, NOM_PERE, COLOR_PREF, ANIMAL_PREF) "
        "VALUES (:id, :email, :mdp, :nom, :prenom, :salaire, "
        ":tel, TO_DATE(:date, 'YYYY-MM-DD'), "
        ":genre, :domaine, :nom_pere, :color_pref, :animal_pref)"
        );

    QString dateString = date_naissance.toString("yyyy-MM-dd");

    query.bindValue(":id", id_perso);
    query.bindValue(":email", email);
    query.bindValue(":mdp", mot_de_passe);
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":salaire", salaire);
    query.bindValue(":tel", num_tel);
    query.bindValue(":date", dateString);
    query.bindValue(":genre", genre);
    query.bindValue(":domaine", domaine);
    query.bindValue(":nom_pere", nom_pere);
    query.bindValue(":color_pref", color_pref);
    query.bindValue(":animal_pref", animal_pref);

    if (!query.exec()) {
        qDebug() << "❌ ERREUR AJOUT :" << query.lastError().text();
        qDebug() << "❌ REQUETE :" << query.lastQuery();
        return false;
    }

    return true;
}


/*===============================
        AFFICHAGE
================================*/
QSqlQueryModel* gpersonnel::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();

    model->setQuery(
        "SELECT ID_PERSO, EMAIL_PERSO, MOT_DE_PASSE_PERSO,"
        "NOM_PERSO, PRENOM_PERSO, SALAIRE_PERSO,"
        "NUM_TEL_PERSO, DATE_NAISSANCE_PERSO,"
        "GENRE_PERSO, DOMAINE_PERSO,"
        "NOM_PERE, COLOR_PREF, ANIMAL_PREF "
        "FROM PERSONNEL"
        );

    return model;
}

/*===============================
        MODIFICATION (CORRIGÉ)
================================*/
bool gpersonnel::modifier(int id_perso, QString email, QString mot_de_passe,
                          QString nom, QString prenom, float salaire,
                          int num_tel, QDate date_naissance, QString genre,
                          QString domaine, QString nom_pere,
                          QString color_pref, QString animal_pref)
{
    QSqlQuery query;

    query.prepare(
        "UPDATE PERSONNEL SET "
        "EMAIL_PERSO = :email, "
        "MOT_DE_PASSE_PERSO = :mdp, "
        "NOM_PERSO = :nom, "
        "PRENOM_PERSO = :prenom, "
        "SALAIRE_PERSO = :salaire, "
        "NUM_TEL_PERSO = :tel, "
        "DATE_NAISSANCE_PERSO = TO_DATE(:date, 'YYYY-MM-DD'), "
        "GENRE_PERSO = :genre, "
        "DOMAINE_PERSO = :domaine, "
        "NOM_PERE = :nom_pere, "
        "COLOR_PREF = :color_pref, "
        "ANIMAL_PREF = :animal_pref "
        "WHERE ID_PERSO = :id"
        );

    // Conversion de la QDate en string ISO compatible Oracle
    QString dateString = date_naissance.toString("yyyy-MM-dd");

    query.bindValue(":email", email);
    query.bindValue(":mdp", mot_de_passe);
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":salaire", salaire);
    query.bindValue(":tel", num_tel);
    query.bindValue(":date", dateString);
    query.bindValue(":genre", genre);
    query.bindValue(":domaine", domaine);
    query.bindValue(":nom_pere", nom_pere);
    query.bindValue(":color_pref", color_pref);
    query.bindValue(":animal_pref", animal_pref);
    query.bindValue(":id", id_perso);

    // DEBUG utile
    qDebug() << "🔵 DATE ENVOYÉE =" << dateString;

    if(!query.exec()) {
        qDebug() << "❌ ERREUR SQL =" << query.lastError().text();
        qDebug() << "❌ REQUETE =" << query.lastQuery();
        return false;
    }

    qDebug() << "✔️ Mise à jour effectuée avec succès";
    return true;
}


/*===============================
        SUPPRESSION
================================*/
bool gpersonnel::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM PERSONNEL WHERE ID_PERSO = :id");
    query.bindValue(":id", id);
    return query.exec();
}

/*===============================
        EXISTE ?
================================*/
bool gpersonnel::existeID(int id)
{
    QSqlQuery query;
    query.prepare("SELECT 1 FROM PERSONNEL WHERE ID_PERSO = :id");
    query.bindValue(":id", id);
    query.exec();
    return query.next();
}

bool gpersonnel::existeEmail(QString email)
{
    QSqlQuery query;
    query.prepare("SELECT 1 FROM PERSONNEL WHERE EMAIL_PERSO = :email");
    query.bindValue(":email", email);
    query.exec();
    return query.next();
}

bool gpersonnel::existeNum(int num)
{
    QSqlQuery query;
    query.prepare("SELECT 1 FROM PERSONNEL WHERE NUM_TEL_PERSO = :num");
    query.bindValue(":num", num);
    query.exec();
    return query.next();
}

/*===============================
        RECHERCHE
================================*/
QSqlQueryModel* gpersonnel::rechercher(QString texte)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QString search = "%" + texte + "%";

    model->setQuery(
        "SELECT * FROM PERSONNEL WHERE "
        "ID_PERSO LIKE '" + search + "' OR "
                   "NOM_PERSO LIKE '" + search + "' OR "
                   "PRENOM_PERSO LIKE '" + search + "' OR "
                   "EMAIL_PERSO LIKE '" + search + "'"
        );

    return model;
}

/*===============================
            TRI
================================*/
QSqlQueryModel* gpersonnel::trier(QString critere)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QString req = "SELECT * FROM PERSONNEL ORDER BY ";

    if (critere == "ID") req += "ID_PERSO ASC";
    else if (critere == "Nom") req += "NOM_PERSO ASC";
    else if (critere == "Prenom") req += "PRENOM_PERSO ASC";
    else if (critere == "Salaire") req += "SALAIRE_PERSO DESC";
    else if (critere == "Domaine") req += "DOMAINE_PERSO ASC";
    else req += "ID_PERSO ASC";

    model->setQuery(req);
    return model;
}

/*===============================
        STATISTIQUES
================================*/
QSqlQueryModel* gpersonnel::statistiquesDomaine()
{
    QSqlQueryModel *model = new QSqlQueryModel();

    model->setQuery(
        "SELECT DOMAINE_PERSO AS Domaine, COUNT(*) AS Total "
        "FROM PERSONNEL GROUP BY DOMAINE_PERSO ORDER BY Total DESC"
        );

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Domaine"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nombre d'employés"));

    return model;
}

/*===============================
        CONNEXION
================================*/
bool gpersonnel::verifierConnexion(const QString &email, const QString &mdp,
                                   int &id_out, QString &nom_out, QString &prenom_out)
{
    QSqlQuery query;
    query.prepare("SELECT ID_PERSO, NOM_PERSO, PRENOM_PERSO FROM PERSONNEL "
                  "WHERE EMAIL_PERSO = :email AND MOT_DE_PASSE_PERSO = :mdp");

    query.bindValue(":email", email);
    query.bindValue(":mdp", mdp);

    if (query.exec() && query.next()) {
        id_out = query.value(0).toInt();
        nom_out = query.value(1).toString();
        prenom_out = query.value(2).toString();
        return true;
    }

    return false;
}



bool gpersonnel::verifierQuestions(QString email,
                                   QString color,
                                   QString animal,
                                   QString pere)
{
    QSqlQuery query;
    query.prepare("SELECT 1 FROM PERSONNEL "
                  "WHERE EMAIL_PERSO = :email "
                  "AND COLOR_PREF = :color "
                  "AND ANIMAL_PREF = :animal "
                  "AND NOM_PERE = :pere");

    query.bindValue(":email", email);
    query.bindValue(":color", color);
    query.bindValue(":animal", animal);
    query.bindValue(":pere", pere);

    query.exec();
    return query.next();
}
bool gpersonnel::changerMdp(QString email, QString nouveau_mdp)
{
    QSqlQuery query;
    query.prepare("UPDATE PERSONNEL SET MOT_DE_PASSE_PERSO = :mdp WHERE EMAIL_PERSO = :email");

    query.bindValue(":mdp", nouveau_mdp);
    query.bindValue(":email", email);

    if (!query.exec()) {
        qDebug() << "Erreur changement mdp:" << query.lastError().text();
        return false;
    }

    return true;
}

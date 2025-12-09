#include "materiel.h"
#include <QPrinter>
#include <QPainter>
#include <QPageLayout>
#include <QSqlError>
Materiel::Materiel() : ID_MATERIEL(0), QTE_DISPO(0), PRIX(0.0) {}

Materiel::Materiel(int id, QString nom, QString type, QString statut, int qte, QDate date, double prix)
{
    this->ID_MATERIEL = id;
    this->NOM = nom;
    this->TYPE = type;
    this->STATUT = statut;
    this->QTE_DISPO = qte;
    this->DATE_ACHAT = date;
    this->PRIX = prix;
}

bool Materiel::ajouter()
{
    QSqlQuery query;
    QString res = QString::number(ID_MATERIEL);
    QString resQte = QString::number(QTE_DISPO);
    QString resPrix = QString::number(PRIX);
    query.prepare("INSERT INTO MATERIEL (ID_MAT, NOM_MAT, TYPE_MAT, STATUT_MAT, QTE_DISPO_MAT, DATE_ACHAT_MAT, PRIX_MAT) "
                  "VALUES (:id, :nom, :type, :statut, :qte_dispo, :date_achat, :prix)");

    query.bindValue(":id", res);
    query.bindValue(":nom", NOM);
    query.bindValue(":type", TYPE);
    query.bindValue(":statut", STATUT);
    query.bindValue(":qte_dispo", resQte);
    query.bindValue(":date_achat", DATE_ACHAT);
    query.bindValue(":prix", resPrix);

    return query.exec();
}
QSqlQueryModel* Materiel::afficher()
{
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT ID_MAT, NOM_MAT, TYPE_MAT, STATUT_MAT, QTE_DISPO_MAT, DATE_ACHAT_MAT, PRIX_MAT FROM MATERIEL");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Statut"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Quantité"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Date Achat"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Prix"));

    return model;
}
bool Materiel::supprimer(int id)
{
    QSqlQuery query;
    QString res = QString::number(id);
    query.prepare("DELETE FROM MATERIEL WHERE ID_MAT = :id");
    query.bindValue(":id", res);
    if (!query.exec()) {
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool Materiel::modifier()
{
    QSqlQuery query;
    query.prepare("UPDATE MATERIEL SET NOM_MAT=:nom, TYPE_MAT=:type, STATUT_MAT=:statut, "
                  "QTE_DISPO_MAT=:qte_dispo, DATE_ACHAT_MAT=:date_achat, PRIX_MAT=:prix "
                  "WHERE ID_MAT=:id");

    query.bindValue(":id", QString::number(ID_MATERIEL));
    query.bindValue(":nom", NOM);
    query.bindValue(":type", TYPE);
    query.bindValue(":statut", STATUT);
    query.bindValue(":qte_dispo", QString::number(QTE_DISPO));
    query.bindValue(":date_achat", DATE_ACHAT);
    query.bindValue(":prix", QString::number(PRIX));

    return query.exec();
}


bool Materiel::exporterPDF(QTableView *view, const QString &fileName)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageOrientation(QPageLayout::Landscape);
    QPainter painter(&printer);
    if (!view) return false;
    painter.scale(15, 15);
    view->render(&painter);
    painter.end();

    return true;
}



QSqlQueryModel* Materiel::rechercher(const QString& idRecherche)
{
    QSqlQueryModel* model = new QSqlQueryModel();
    QSqlQuery query;
    query.prepare("SELECT ID_MAT, NOM_MAT, TYPE_MAT, STATUT_MAT, QTE_DISPO_MAT, DATE_ACHAT_MAT, PRIX_MAT FROM MATERIEL WHERE ID_MAT = :id");
    query.bindValue(":id", idRecherche);

    if (query.exec()) {
        model->setQuery(std::move(query));
        return model;
    } else {
        qDebug() << "Erreur SQL lors de la recherche :" << query.lastError().text();
        delete model;
        return nullptr;
    }
}

QSqlQueryModel* Materiel::trier(const QString& colonne, const QString& ordre)
{
    QSqlQueryModel* model = new QSqlQueryModel();
    QSqlQuery query;
    QString req = "SELECT ID_MAT, NOM_MAT, TYPE_MAT, STATUT_MAT, QTE_DISPO_MAT, DATE_ACHAT_MAT, PRIX_MAT FROM MATERIEL ORDER BY " + colonne + " " + ordre;
    qDebug() << "Requête de Tri exécutée :" << req;

    if (query.exec(req)) {
        model->setQuery(std::move(query));
        return model;
    } else {
        qDebug() << "ERREUR SQL LORS DU TRI :" << query.lastError().text();
        delete model;
        return nullptr;
    }
}
QMap<QString, int> Materiel::compter_statut()
{
    QMap<QString, int> stats;
    QSqlQuery query;
    query.prepare("SELECT STATUT_MAT, COUNT(*) FROM MATERIEL GROUP BY STATUT_MAT");
    if (query.exec()) {
        while (query.next()) {
            QString statut = query.value(0).toString();
            int count = query.value(1).toInt();
            stats.insert(statut, count);
        }
    } else {
        qDebug() << "Erreur SQL dans compter_statut:" << query.lastError().text();
    }
    return stats;
}



QSqlQuery Materiel::suggerer_materiel_remplacement(const QString& id_materiel_reserve)
{
    QSqlQuery result_query;

    QSqlQuery initial_query;
    initial_query.prepare("SELECT TYPE_MAT FROM MATERIEL WHERE ID_MAT = :id");
    initial_query.bindValue(":id", id_materiel_reserve);

    if (!initial_query.exec() || !initial_query.next()) {
        qDebug() << "Erreur: ID de matériel initial non trouvé ou erreur SQL.";
        return result_query;
    }

    QString type = initial_query.value("TYPE_MAT").toString();

    result_query.prepare("SELECT ID_MAT, NOM_MAT, TYPE_MAT FROM MATERIEL "
                         "WHERE TYPE_MAT LIKE :type "
                         "  AND (STATUT_MAT = 'Disponible' OR STATUT_MAT = 'disponible') "
                         "  AND QTE_DISPO_MAT > 0 "
                         "  AND ID_MAT != :id_initial "
                         "  AND ROWNUM <= 1");

    result_query.bindValue(":type", type);
    result_query.bindValue(":id_initial", id_materiel_reserve);

    if (result_query.exec()) {

        if (result_query.next()) {
            qDebug() << "Suggestion trouvée par Type (ID: " << result_query.value("ID_MAT").toString() << ") : " << result_query.value("NOM_MAT").toString();
            result_query.previous();
            return result_query;
        }
    } else {
        qDebug() << "Erreur SQL CRITIQUE (suggerer_materiel_remplacement): " << result_query.lastError().text();
    }

    qDebug() << "Aucun remplacement disponible trouvé pour le type : " << type;
    return result_query;
}
int Materiel::getDureeMaintenance(const QString& type)
{
    static const QMap<QString, int> dureesMaintenance = {
        {"Caméra", 180},
        {"Ordinateur", 90},
        {"Audio", 120},
    };

    return dureesMaintenance.value(type, 60);
}

QString Materiel::genererNotificationsMaintenance()
{
    QString notifications;
    QSqlQuery query("SELECT NOM_MAT, TYPE_MAT, DATE_ACHAT_MAT FROM MATERIEL WHERE STATUT_MAT != 'en panne'");

    while (query.next()) {
        QString nom = query.value("NOM_MAT").toString();
        QString type = query.value("TYPE_MAT").toString();
        QDate dateAchat = query.value("DATE_ACHAT_MAT").toDate();

        if (dateAchat.isNull() || dateAchat.year() < 2000) continue;

        int dureeMaintenance = getDureeMaintenance(type);
        QDate prochaineMaintenance = dateAchat.addDays(dureeMaintenance);

        QDate dateActuelle = QDate::currentDate();
        if (dateActuelle >= prochaineMaintenance.addDays(-30)) {
            notifications += QString("Alerte: %1 (%2) - Maintenance requise avant le %3\n")
            .arg(nom).arg(type).arg(prochaineMaintenance.toString("dd/MM/yyyy"));
        }
    }

    if (notifications.isEmpty()) {
        return "Aucune maintenance urgente requise.";
    }
    return notifications;
}

QString Materiel::getListeMaterielPourIA()
{
    QString liste;
    QSqlQuery query;

    query.prepare("SELECT ID_MAT, NOM_MAT, TYPE_MAT, STATUT_MAT, QTE_DISPO_MAT, PRIX_MAT FROM MATERIEL");

    if (!query.exec()) {
        qDebug() << "Erreur SQL (getListeMaterielPourIA): " << query.lastError().text();


        return "Erreur lors de la récupération des données du matériel.";
    }

    while (query.next()) {
        QString id = query.value("ID_MAT").toString();
        QString nom = query.value("NOM_MAT").toString();
        QString type = query.value("TYPE_MAT").toString();
        QString statut = query.value("STATUT_MAT").toString();
        QString qte = query.value("QTE_DISPO_MAT").toString();
        QString prix = query.value("PRIX_MAT").toString();


        liste += QString("ID: %1, Nom: %2, Type: %3, Statut: %4, Qte: %5, Prix: %6\n")
                     .arg(id).arg(nom).arg(type).arg(statut).arg(qte).arg(prix);
    }

    if (liste.isEmpty()) {
        return "Aucun matériel trouvé dans la base de données.";
    }
    return liste;
}

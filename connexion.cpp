#include "connexion.h"
#include <QDebug>
#include<QSqlDatabase>
connexion::connexion() {
    db=QSqlDatabase::addDatabase("QODBC");
}
connexion::~connexion(){
    if(db.isOpen())
        db.close();
}
connexion& connexion::creatInstance(){
    static connexion instance;
    return instance;
}
bool connexion::createconnect()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("new_data_base");//inserer le nom de la source de données
    db.setUserName("hamouda");//inserer nom de l'utilisateur
    db.setPassword("ommi12345");//inserer mot de passe de cet utilisateur


    if (db.open())
    {
        qDebug() <<"felicitations vous etes connectez a la base de donne oracle";
        return true;
    }
    else {
        qDebug() << "Échec de la connexion :" << db.lastError().text();
        return false;
    }
}

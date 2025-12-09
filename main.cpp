#include "smart_creation.h"
#include "connexion.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Connexion DB via ta classe
    connexion& c = connexion::creatInstance();
    bool test = c.createconnect();

    smart_creation SC;

    if (test)
    {
        SC.show();
        QMessageBox::information(nullptr, QObject::tr("Connexion réussie"),
                                 QObject::tr("Connexion établie avec succès !"),
                                 QMessageBox::Ok);
    }
    else
    {
        QMessageBox::critical(nullptr, QObject::tr("Erreur de connexion"),
                              QObject::tr("Échec de connexion à la base de données."),
                              QMessageBox::Ok);
        return -1;
    }

    return a.exec();
}

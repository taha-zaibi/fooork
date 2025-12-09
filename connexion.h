#ifndef CONNEXION_H
#define CONNEXION_H
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
class connexion
{
public:
    static connexion& creatInstance();
    bool createconnect();
private:
    QSqlDatabase db;
    connexion();
    ~connexion();
    connexion(const connexion&) =delete;
    connexion& operator=(const connexion&)=delete;
};
#endif // CONNEXION_H

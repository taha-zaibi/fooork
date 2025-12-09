#ifndef INFLUENCEUR_H
#define INFLUENCEUR_H
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QString>
class influenceur
{
private:
    QString nom,prenom,username,email,genre;
    int id,nb_suiv,num_tel;

public:
    influenceur();
    influenceur(int,QString,QString,QString,QString,int,int,QString);
    //get
    int GetId(){return id;}
    QString getNom() {return nom;}
    QString getPrenom() {return prenom;}
    QString getEmail() {return email;}
    QString getUsername() {return username;}
    int getNb_Suiv(){return nb_suiv;}
    int getNum_tel(){return num_tel;}
    QString getGenre(){return genre;}
    //set
    void setId(int x){id=x;}
    void setNom(QString n){nom=n;}
    void setPrenom(QString n){prenom=n;}
    void setUsername(QString n){username=n;}
    void setNb_Suiv(int x){nb_suiv=x;}
    void setNum_tel(int x){num_tel=x;}
    void setGenre(QString g){genre=g;}

    //fonct
    bool ajouter();
    QSqlQueryModel *afficher();
    bool supprimer(int);
    bool modifier(int , QString , QString , QString , QString , int , int , QString );
    QSqlQueryModel* rechercher(const int &);
    QSqlQueryModel* trier();
};
#endif // INFLUENCEUR_H

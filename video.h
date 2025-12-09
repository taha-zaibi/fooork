#ifndef VIDEO_H
#define VIDEO_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDate>

class Video
{
private:
    int id_vid;
    QString nom_video;
    int nb_vues;
    int nb_likes;
    QDate date_vd;
    QString categorie;
    double cout;

public:
    // Constructeurs
    Video();
    Video(int id, QString nom, int vues, int likes, QDate date, QString cat, double cout_val);

    // Getters
    int getIdVid() const { return id_vid; }
    QString getNomVideo() const { return nom_video; }
    int getNbVues() const { return nb_vues; }
    int getNbLikes() const { return nb_likes; }
    QDate getDateVd() const { return date_vd; }
    QString getCategorie() const { return categorie; }
    double getCout() const { return cout; }

    // Setters
    void setIdVid(int id) { id_vid = id; }
    void setNomVideo(QString nom) { nom_video = nom; }
    void setNbVues(int vues) { nb_vues = vues; }
    void setNbLikes(int likes) { nb_likes = likes; }
    void setDateVd(QDate date) { date_vd = date; }
    void setCategorie(QString cat) { categorie = cat; }
    void setCout(double c) { cout = c; }

    // CRUD - Fonctionnalités de base
    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(int id);
    bool modifier();

    // Fonctionnalités avancées
    QSqlQueryModel* rechercher(QString critere);
    bool existe(int id);
};

#endif // VIDEO_H

#ifndef SMART_CREATION_H
#define SMART_CREATION_H
#include "roi_analyzer.h"
#include "ml_predictor.h"
#include "influenceur.h"
#include "materiel.h"
#include "qmainwindow.h"
#include "chatbot.h"
#include "gpersonnel.h"
#include <QMainWindow>
#include <QPrinter>
#include <QPainter>
#include <QFileDialog>
#include <QPageSize>
#include <QMessageBox>
#include <QDebug>
#include <QIntValidator>
#include <QRegularExpressionValidator>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QChart>
#include <QTableWidgetItem>
#include <QSqlQuery>
#include <QSqlError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "video.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class smart_creation;
}
QT_END_NAMESPACE

class smart_creation : public QMainWindow
{
    Q_OBJECT

public:
    smart_creation(QWidget *parent = nullptr);
    ~smart_creation();

private:
    gpersonnel GP;

private slots:
    // Navigation
    void on_valider_clicked();
    void on_employe_clicked();
    void on_materiel_clicked();
    void on_influenceur_clicked();
    void on_sponsors_clicked();
    void on_vd_clicked();
    void on_quitter_clicked();
    void on_mdp_oublie_clicked();
    void on_mdp_valider_clicked();

    // CRUD Influenceur (leurs fonctionnalités)
    void on_enregistrer_clicked();
    void on_sup_clicked();
    void on_modif_clicked();
    void on_trie_clicked();
    void on_recherche_clicked();
    void on_pdf_clicked();
    void on_tableView_influenceur_clicked(const QModelIndex &index);
    void on_quit_clicked();
    void on_stat_clicked();
    void on_retourne_clicked();
    void on_chatBotB_clicked();
    void on_retourne_2_clicked();
    void on_refresh_tabInf_clicked();
    //chat
    void onSendButtonClicked();
    void onResponseReceived(const QString &response);
    void onErrorOccurred(const QString &error);
    void onChatbotProcessingFinished();
    //twilio
    void onTwilioReplyFinished(QNetworkReply *reply);

    // CRUD Materiel (leurs fonctionnalités)
    void on_pushButton_ajouter_clicked();
    void on_pushButton_supprimer_clicked();
    void on_pushButton_modifier_clicked();
    void on_tableView_materiel_activated(const QModelIndex &index);
    void on_pushButton_exporter_clicked();
    void on_pushButton_recherche_clicked();
    void on_pushButton_ref_liste_clicked();
    void on_comboBox_Tri_currentIndexChanged(int index);
    void on_pushButton_statistique_clicked();
    void on_lineEdit_id_reservation_editingFinished();
    void mettreAJourMaintenance();
    void recommandationMaterielIA();

    // CRUD Personnel
    void on_ajouter_pers_clicked();
    void on_supprimer_pers_clicked();
    void on_modifier_pers_clicked();
    void on_afficher_pers_clicked();
    void on_tableView_2_clicked(const QModelIndex &index);
    void on_annuler_pers_clicked();
    void on_btn_rech_clicked();
    void on_export_pers_clicked();
    void on_tri_pers_clicked();
    void on_static_pers_clicked();
    void afficherStatistiquesDomaine();
    void on_btn_verifier_clicked();
    void on_btnrecherche_clicked();

    // CRUD Video (vos fonctionnalités)
    void on_btnajout_clicked();
    void on_btnmod_clicked();
    void on_btnsupp_clicked();
    void on_btnannuler_clicked();

    void on_lineedit_recherche_textChanged(const QString &arg1);
    void on_btntri_clicked();
    void on_pushButton_73_clicked();
    void on_pushButton_75_clicked();
    void on_btn_roi_clicked();
    void on_btn_predicteur_clicked();
private:
    Ui::smart_creation *ui;
    influenceur inf;
    Materiel Mat;
    Video videoTemp;



    // Méthodes pour influenceur
    bool saisie();
    void refreshTable();
    //chat
    class Chatbot *m_chatbot;
    void setupChatbot();
    void appendUserMessage(const QString &message);
    void appendBotMessage(const QString &message);
    void appendErrorMessage(const QString &error);
    void removeLastMessageIfLoading();
    void scrollToBottom();
    void setChatInputEnabled(bool enabled);
    //twillio
    QNetworkAccessManager *networkManager;
    QString twilio_account_sid = "ACa8587dc430e794ca7e06906e6751d980";
    QString twilio_auth_token = "d58ea72823fa5676b14ba261d1b200c2";
    QString twilio_from_number = "+15863154356";
    void sendWelcomeMessage(int phone_int, const QString& name = "");
    bool isTunisianMobileNumber(int phone_int);
    QString convertIntToInternational(int phone_int);

    //personnel
    QString emailRecup;

    // Méthodes pour vidéo
    void viderChamps();
    bool validerChamps();
    void actualiserAffichage();
    void rechercher(QString critere);
    void configurerConnexions();
    void chargerVideoDuTableau(int row);




};

#endif // SMART_CREATION_H

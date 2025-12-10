#include "smart_creation.h"
#include "connexion.h"
#include "chatbot.h"
#include "ui_smart_creation.h"
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QDebug>
#include <QString>
#include <QIntValidator>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QPainter>
#include <QAbstractItemModel>
#include <QTextDocument>
#include <QTextTable>
#include <QTextCursor>
#include <QPrinter>
#include <QVariant>
#include <QtCharts>
#include <QChartView>
#include <QPieSeries>
#include <QPieSlice>
#include <QPageSize>
#include <QNetworkReply>
#include <QScrollBar>
#include <QScrollArea>
#include <QSslError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QNetworkRequest>


smart_creation::smart_creation(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::smart_creation),
    m_chatbot(new Chatbot(this))
{
    ui->setupUi(this);

    // Test de connexion au démarrage
    connexion& c = connexion::creatInstance();
    bool test = c.createconnect();


    if(!test) {
        QMessageBox::critical(this, "Erreur", "Impossible de se connecter à la base de données");
    }

    // Initialisation pour influenceur
    ui->tableView_influenceur->setModel(inf.afficher());
    QIntValidator *validator = new QIntValidator(00000000,99999999,this);
    ui->id_inf->setValidator(validator);
    ui->rech->setValidator(validator);
    ui->suivre->setValidator(validator);
    ui->nom->setMaxLength(10);
    ui->prenom->setMaxLength(10);
    QRegularExpression regex("^[A-Za-zÀ-ÿ\\s'-]+$");
    QRegularExpressionValidator *validatorr = new QRegularExpressionValidator(regex, this);
    ui->nom->setValidator(validatorr);
    ui->prenom->setValidator(validatorr);
    /*CHTATTT*/
    setupChatbot();
    connect(m_chatbot, &Chatbot::processingFinished, this, &smart_creation::onChatbotProcessingFinished);
    //twillio
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &smart_creation::onTwilioReplyFinished);

    // Arduino Gas Sensor System
    setupArduinoConnection();

    // Initialisation pour matériel
    refreshTable();
    mettreAJourMaintenance();
    QIntValidator *idValidator = new QIntValidator(1, 999999, this);
    ui->lineEdit_ID->setValidator(idValidator);
    ui->lineEdit_Supprimer->setValidator(idValidator);
    QRegularExpression rx("^[A-Za-z\\s\\-]{1,13}$");
    QValidator *nomValidator = new QRegularExpressionValidator(rx, this);

    ui->lineEdit_Nom->setValidator(nomValidator);

    connect(ui->bouton_recommand, &QPushButton::clicked,
            this, &smart_creation::recommandationMaterielIA);

    actualiserAffichage();
    ui->tab1->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tab1->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tab1->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tab1->horizontalHeader()->setStretchLastSection(true);
    ui->date->setDate(QDate::currentDate());
    ui->nbvus->setText("0");
    ui->nblikes->setText("0");
    ui->categ->setText("0");
    ui->cout->setText("0.0");
    ui->idvid->setPlaceholderText("Tapez l'id");
    ui->nomvid->setPlaceholderText("Tapez le nom de vidéo");
    ui->idinflu->setPlaceholderText("Tapez l'id influenceur");
    ui->nbvus->setPlaceholderText("nombre de vues");
    ui->nblikes->setPlaceholderText("nombre de likes");
    ui->categ->setPlaceholderText("Tapez la catégorie");
    ui->cout->setPlaceholderText("Tapez le cout");


     ui->tableView_2->setModel(GP.afficher());
}

smart_creation::~smart_creation()
{
    delete ui;
}


void smart_creation::on_valider_clicked()
{
    // Récupération des champs saisis
    QString email = ui->enter_mail->text().trimmed();
    QString mdp   = ui->enter_mdp->text();

    // Vérification que les champs ne sont pas vides
    if (email.isEmpty() || mdp.isEmpty()) {
        QMessageBox::warning(this, "Champs vides",
                             "Veuillez saisir votre email et votre mot de passe.");
        return;
    }

    // Création de l'objet gpersonnel pour utiliser la fonction de connexion
    gpersonnel pers;

    int    id_perso = 0;
    QString nom     = "";
    QString prenom  = "";

    bool connexionOk = pers.verifierConnexion(email, mdp, id_perso, nom, prenom);

    if (connexionOk) {
        // Connexion réussie → on passe à l'interface principale
        qDebug() << "Connexion réussie ! Bienvenue" << prenom << nom << "(ID:" << id_perso << ")";

        // Optionnel : tu peux stocker les infos de l'utilisateur connecté quelque part
        // Exemple : dans des variables membres de smart_creation, ou dans une classe singleton, etc.
        // this->userId = id_perso;
        // this->userNomComplet = prenom + " " + nom;

        ui->stackedWidget->setCurrentIndex(1);  // Interface principale
    }
    else {
        // Échec de connexion
        QMessageBox::critical(this, "Erreur de connexion",
                              "Email ou mot de passe incorrect.");

        // Optionnel : vider le champ mot de passe
        ui->enter_mdp->clear();
        ui->enter_mail->setFocus();
    }
}

void smart_creation::on_mdp_oublie_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    qDebug() << "mdp oblier";
}


void smart_creation::on_mdp_valider_clicked()
{
    QString email = ui->mdp_mail->text();
    QString color = ui->mdp_color->text();
    QString animal = ui->mdp_animal->text();
    QString pere = ui->mdp_pere->text();

    gpersonnel gp;

    if (gp.verifierQuestions(email, color, animal, pere)) {
        emailRecup = ui->mdp_mail->text();
        ui->stackedWidget->setCurrentIndex(3);
        qDebug() << " Accès autorisé.";
    }
    else {
        QMessageBox::warning(this, "Erreur",
                             "Les informations saisies sont incorrectes !");
        qDebug() << " Réponses incorrectes.";
    }
}

void smart_creation::on_btn_verifier_clicked()
{
    QString mdp1 = ui->saisire_mdp->text();
    QString mdp2 = ui->verifier_mdp->text();

    // Vérifier si les champs sont remplis
    if (mdp1.isEmpty() || mdp2.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs.");
        return;
    }

    // Vérifier si les deux mots de passe correspondent
    if (mdp1 != mdp2) {
        QMessageBox::warning(this, "Erreur", "Les mots de passe ne correspondent pas.");
        return;
    }

    // Modifier dans la base
    gpersonnel gp;
    if (gp.changerMdp(emailRecup, mdp1)) {
        QMessageBox::information(this, "Succès", "Mot de passe changé avec succès !");
        ui->stackedWidget->setCurrentIndex(0);  // Retour login
    } else {
        QMessageBox::critical(this, "Erreur", "Impossible de changer le mot de passe.");
    }
}


void smart_creation::on_employe_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(2);
    qDebug() << "Navigation vers la gestion employé";
}

void smart_creation::on_materiel_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(3);
    qDebug() << "Navigation vers la gestion matériel";
}

void smart_creation::on_influenceur_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(4);
    qDebug() << "Navigation vers la gestion influenceur";
}

void smart_creation::on_sponsors_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(5);
    qDebug() << "Navigation vers la gestion sponsors";
}

void smart_creation::on_vd_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(1);
    //actualiserAffichage();
    qDebug() << "Navigation vers la gestion vidéos";
}

void smart_creation::on_quitter_clicked()
{
    // Fermer l'application
    qDebug() << "Fermeture de l'application";
    QApplication::quit();
}



// ============= CRUD INFLUENCEUR =============
bool smart_creation::saisie()
{
    if(ui->id_inf->text().isEmpty() || ui->id_inf->text().toInt() <= 0)
    {
        QMessageBox::warning(this, "Erreur", "L'ID doit être un nombre supérieur à 0");
        return false;
    }
    if(ui->suivre->text().isEmpty() || ui->suivre->text().toInt() <= 0)
    {
        QMessageBox::warning(this, "Erreur", "Le Nombre de suivre doit être un nombre supérieur à 0");
        return false;
    }
    if(ui->nom->text().isEmpty())
    {
        QMessageBox::warning(this, "Erreur", "Le nom est obligatoire");
        return false;
    }
    if(ui->prenom->text().isEmpty())
    {
        QMessageBox::warning(this, "Erreur", "Le prénom est obligatoire");
        return false;
    }
    if(ui->tel->text().isEmpty() || ui->tel->text().toInt() <= 0)
    {
        QMessageBox::warning(this, "Erreur", "Le téléphone doit être un nombre supérieur à 0");
        return false;
    }
    /*
    if(ui->tel->text().length() != 8)
    {
        QMessageBox::warning(this, "Erreur", "Le téléphone doit contenir exactement 8 chiffres");
        return false;
    }*/
    if(ui->suivre->text().isEmpty() || ui->suivre->text().toInt() < 0)
    {
        QMessageBox::warning(this, "Erreur", "Le nombre de suiveurs doit être >= 0");
        return false;
    }
    if(!ui->fm->isChecked() && !ui->hm->isChecked())
    {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un genre");
        return false;
    }
    return true;
}

void smart_creation::on_enregistrer_clicked()
{
    if(!saisie())
        return;
    int id=ui->id_inf->text().toInt();
    QString nom=ui->nom->text();
    QString prenom=ui->prenom->text();
    QString username=ui->username->text();
    QString email=ui->email->text();
    int num_tel=ui->tel->text().toInt();
    int nb_suiv=ui->suivre->text().toInt();
    QString genre="";
    if(ui->fm->isChecked())
        genre="Femme";
    else if(ui->hm->isChecked())
        genre="Homme";
    //twillio
    if(!isTunisianMobileNumber(num_tel)) {
        QMessageBox::warning(this, "Erreur", "Numéro de téléphone invalide. Doit être un numéro tunisien mobile (commençant par 55)");
        return;
    }
    influenceur nouvelInf;
    nouvelInf.setNom(nom);
    nouvelInf.setNum_tel(num_tel);

    influenceur inf(id,nom,prenom,email,username,num_tel,nb_suiv,genre);
    if(inf.ajouter())
    {
        ui->tableView_influenceur->setModel(inf.afficher());
        QMessageBox::information(nullptr, QObject::tr("database is open"),
                                 QObject::tr("addition successful.\n" "Click Cancel to exit."),
                                 QMessageBox::Cancel);
        // Envoyer le message de bienvenue
        qDebug() << "Envoi du SMS de bienvenue à:" << nom << "(" << num_tel << ")";
        sendWelcomeMessage(num_tel, nom);

        ui->id_inf->clear();
        ui->nom->clear();
        ui->prenom->clear();
        ui->email->clear();
        ui->username->clear();
        ui->tel->clear();
        ui->suivre->clear();
        ui->fm->setChecked(false);
        ui->hm->setChecked(false);
    }
    else{
        QMessageBox::information(nullptr, QObject::tr("database is open"),
                                 QObject::tr("addition failed.\n" "Click Cancel to exit."),
                                 QMessageBox::Cancel);
    }


}

void smart_creation::on_sup_clicked()
{
    int id=0;
    QModelIndex index=ui->tableView_influenceur->selectionModel()->currentIndex();

    if(index.isValid()){
        id=ui->tableView_influenceur->model()->data(ui->tableView_influenceur->model()->index(index.row(), 0)).toInt();
    }
    else if(!ui->id_inf->text().isEmpty()){
        id=ui->id_inf->text().toInt();
    }
    if(id==0) return;
    QMessageBox::StandardButton reply;
    reply=QMessageBox::question(
        nullptr,
        QObject::tr("Confirmer"),
        QObject::tr("Êtes-vous sûr(e) de vouloir supprimer l'ID %1 ?").arg(id),
        QMessageBox::Yes|QMessageBox::No
        );
    if(reply==QMessageBox::Yes){
        if(inf.supprimer(id)){
            ui->tableView_influenceur->setModel(inf.afficher());
            QMessageBox::information(nullptr, QObject::tr("Succès"), QObject::tr("Suppression réussie."));
        }
        else{
            QMessageBox::critical(nullptr, QObject::tr("Erreur"), QObject::tr("Échec de la suppression."));
        }
    }
}

void smart_creation::on_tableView_influenceur_clicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    QAbstractItemModel* model = ui->tableView_influenceur->model();
    int id=model->data(model->index(index.row(), 0)).toInt();
    QString nom=model->data(model->index(index.row(), 1)).toString();
    QString prenom= model->data(model->index(index.row(), 2)).toString();
    QString eml = model->data(model->index(index.row(), 3)).toString();
    QString usr = model->data(model->index(index.row(), 4)).toString();
    QString tel = model->data(model->index(index.row(), 5)).toString();
    int suivre = model->data(model->index(index.row(), 6)).toInt();
    QString gnr = model->data(model->index(index.row(), 8)).toString();
    ui->id_inf->setText(QString::number(id));
    ui->nom->setText(nom);
    ui->prenom->setText(prenom);
    ui->email->setText(eml);
    ui->username->setText(usr);
    ui->tel->setText(tel);
    ui->suivre->setText(QString::number(suivre));
    if (gnr=="fm") {
        ui->fm->setChecked(true);
        ui->hm->setChecked(false);
    } else if (gnr=="hm") {
        ui->hm->setChecked(true);
        ui->fm->setChecked(false);
    }
}

void smart_creation::on_modif_clicked()
{
    if (ui->id_inf->text().isEmpty()) {
        QMessageBox::warning(this, QObject::tr("Error"),
                             QObject::tr("Veuillez sélectionner un enregistrement."),
                             QMessageBox::Ok);
        return;
    }
    int x=ui->id_inf->text().toInt();
    QString n=ui->nom->text();
    QString p=ui->prenom->text();
    QString eml=ui->email->text();
    QString usr=ui->username->text();
    int tel=ui->tel->text().toInt();
    int suivre=ui->suivre->text().toInt();
    QString gnr;
    if (ui->fm->isChecked()) {
        gnr="fm";
    } else if (ui->hm->isChecked()) {
        gnr="hm";
    } else {
        QMessageBox::warning(this, QObject::tr("Error"),
                             QObject::tr("Veuillez sélectionner un genre."),
                             QMessageBox::Ok);
        return;
    }
    if (inf.modifier(x, n, p, eml, usr, tel, suivre, gnr))
    {
        ui->tableView_influenceur->setModel(inf.afficher());
        ui->id_inf->clear();
        ui->nom->clear();
        ui->prenom->clear();
        ui->email->clear();
        ui->username->clear();
        ui->tel->clear();
        ui->suivre->clear();
        ui->fm->setChecked(false);
        ui->hm->setChecked(false);
        QMessageBox::information(this, QObject::tr("Succès"),
                                 QObject::tr("Modification réussie."),
                                 QMessageBox::Ok);
    } else {
        QMessageBox::critical(this, QObject::tr("Erreur"),
                              QObject::tr("Échec de la modification."),
                              QMessageBox::Ok);
    }
}

void smart_creation::on_trie_clicked()
{
    ui->tableView_influenceur->setModel(inf.trier());
}
void smart_creation::on_recherche_clicked()
{
    if (ui->rech->text().isEmpty()) {
        QMessageBox::warning(this, "Attention", "Veuillez entrer l'ID de l'influenceur à rechercher.");
        return;
    }
    int idRecherche = ui->rech->text().toInt();
    QSqlQueryModel* filteredModel = inf.rechercher(idRecherche);

    if (filteredModel)
    {
        if (filteredModel->rowCount() > 0)
        {
            ui->tableView_influenceur->setModel(filteredModel);
            QMessageBox::information(this, "Recherche Influenceur", "Influenceur trouvé avec l'ID : " + QString::number(idRecherche));
        }
        else
        {
            QMessageBox::warning(this, "Erreur", "Aucun influenceur trouvé avec cet ID.");
            delete filteredModel;
        }
    }
    else
    {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la recherche.");
    }

    ui->rech->clear();
}
void smart_creation::on_pdf_clicked()
{
    QAbstractItemModel *model = ui->tableView_influenceur->model();
    if (!model) {
        QMessageBox::critical(this, "Erreur", "Le modèle du tableau n'est pas disponible.");
        return;
    }
    if(model->rowCount() == 0) {
        QMessageBox::warning(this, "Attention", "Le tableau est vide !");
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF", "", "*.pdf");
    if(fileName.isEmpty()) return;
    if(!fileName.endsWith(".pdf")) fileName += ".pdf";
    QTextDocument doc;
    QTextCursor cursor(&doc);
    cursor.insertHtml("<h2 align='center'>Liste des Influenceurs</h2><br>");
    int rows = model->rowCount();
    int cols = model->columnCount();

    QTextTableFormat tableFormat;
    tableFormat.setBorder(1);
    tableFormat.setCellPadding(3);
    tableFormat.setCellSpacing(0);
    QTextTable *table = cursor.insertTable(rows + 1, cols, tableFormat);

    for(int col = 0; col < cols; col++) {
        QVariant headerData = model->headerData(col, Qt::Horizontal, Qt::DisplayRole);
        QString text = headerData.toString();
        if (table->cellAt(0, col).isValid()) {
            table->cellAt(0, col).firstCursorPosition().insertText(text);
        }
    }
    for(int row = 0; row < rows; row++) {
        for(int col = 0; col < cols; col++) {
            QVariant data = model->data(model->index(row, col), Qt::DisplayRole);
            QString text = data.toString();
            if (table->cellAt(row + 1, col).isValid()) {
                table->cellAt(row + 1, col).firstCursorPosition().insertText(text);
            }
        }
    }
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    doc.print(&printer);
    QMessageBox::information(this, "Succès", "Export PDF terminé !");
}

void smart_creation::on_quit_clicked()
{
    this->close();
}

void smart_creation::on_stat_clicked()
{
    ui->tabWidget->setCurrentIndex(1);
    QList<QChartView*> oldCharts = ui->tab_2->findChildren<QChartView*>();
    foreach(QChartView* chart, oldCharts) {
        delete chart;
    }
    QBarSet *set = new QBarSet("Suiveurs");
    set->setColor(QColor("#4d94cc"));
    QStringList noms;
    QSqlQuery query("SELECT NOM_INF, NB_SUIV_INF FROM INFLUENCEUR ORDER BY NB_SUIV_INF DESC");
    while (query.next())
    {
        noms << query.value(0).toString();
        *set << query.value(1).toInt();
    }
    if(noms.isEmpty())
    {
        QMessageBox::information(this, "Information", "Aucun influenceur trouvé!");
        return;
    }
    QBarSeries *series = new QBarSeries();
    series->append(set);
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Statistiques des Influenceurs");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setMargins(QMargins(10, 10, 10, 10));
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(noms);
    axisX->setTitleText("Influenceurs");
    QFont fontX = axisX->labelsFont();
    fontX.setPointSize(8);
    axisX->setLabelsFont(fontX);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    QLogValueAxis *axisY = new QLogValueAxis();
    axisY->setTitleText("Suiveurs");
    axisY->setBase(10);
    axisY->setLabelFormat("%g");
    QFont fontY = axisY->labelsFont();
    fontY.setPointSize(8);
    axisY->setLabelsFont(fontY);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    QFont fontLegend = chart->legend()->font();
    fontLegend.setPointSize(8);
    chart->legend()->setFont(fontLegend);
    QFont fontTitle = chart->titleFont();
    fontTitle.setPointSize(10);
    chart->setTitleFont(fontTitle);
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setParent(ui->tab_2);
    chartView->setGeometry(60, 60, 790,450);
    chartView->show();
}
void smart_creation::on_chatBotB_clicked()
{
    ui->tabWidget->setCurrentIndex(2);

}
void smart_creation::on_retourne_clicked()
{
    ui->tabWidget->setCurrentIndex(0);
}

void smart_creation::on_retourne_2_clicked()
{
    ui->tabWidget->setCurrentIndex(0);

}

void smart_creation::on_refresh_tabInf_clicked()
{
    QSqlQueryModel* model = inf.afficher();

    if (model)
    {
        model->setHeaderData(0, Qt::Horizontal, "ID");
        model->setHeaderData(1, Qt::Horizontal, "Nom");
        model->setHeaderData(2, Qt::Horizontal, "Prenom");
        model->setHeaderData(3, Qt::Horizontal, "Email");
        model->setHeaderData(4, Qt::Horizontal, "Username");
        model->setHeaderData(5, Qt::Horizontal, "Telephone");
        model->setHeaderData(6, Qt::Horizontal, QObject::tr("Nb_Suivre"));
        model->setHeaderData(7, Qt::Horizontal, QObject::tr("Date_RDV"));
        model->setHeaderData(8, Qt::Horizontal, QObject::tr("Genre"));
        ui->tableView_influenceur->setModel(model);
    }
    ui->rech->clear();
}
void smart_creation::scrollToBottom()
{
    QTimer::singleShot(50, [this]() {
        ui->chatContainer->verticalScrollBar()->setValue(
            ui->chatContainer->verticalScrollBar()->maximum()
            );
    });
}

void smart_creation::setChatInputEnabled(bool enabled)
{
    ui->chatINF->setEnabled(enabled);
    ui->chatbotINF->setEnabled(enabled);
    if (enabled) {
        ui->chatbotINF->setFocus();
    }
}
void smart_creation::setupChatbot()
{
    // Connecter les signaux du chatbot
    connect(m_chatbot, &Chatbot::responseReceived,
            this, &smart_creation::onResponseReceived);
    connect(m_chatbot, &Chatbot::errorOccurred,
            this, &smart_creation::onErrorOccurred);

    // Connecter le bouton d'envoi
    connect(ui->chatINF, &QPushButton::clicked,
            this, &smart_creation::onSendButtonClicked);

    // Permettre d'envoyer avec la touche Enter
    connect(ui->chatbotINF, &QLineEdit::returnPressed,
            this, &smart_creation::onSendButtonClicked);

    // Configuration initiale de l'interface
    ui->chatContainer->setReadOnly(true);
    ui->chatbotINF->setPlaceholderText("Posez n'importe quelle question...");

    // Message de bienvenue
    appendBotMessage("🤖 **Assistant IA  Activé**\n\n"
                     "Je suis un modèle d'IA entraîné sur des milliards de données. "
                     "Je peux répondre à n'importe quelle question : géographie, science, "
                     "histoire, programmation, conseils, etc.\n\n"
                     "Posez-moi vos questions !");
}
void smart_creation::onChatbotProcessingFinished()
{
    setChatInputEnabled(true); // RÉACTIVER LA SAISIE
    ui->chatbotINF->setFocus(); // REMETTRE LE FOCUS
    qDebug() << "=== CHATBOT PROCESSING FINISHED - INTERFACE RÉACTIVÉE ===";
}

void smart_creation::onSendButtonClicked()
{
    QString userMessage = ui->chatbotINF->text().trimmed();
    if (userMessage.isEmpty()) return;

    appendUserMessage(userMessage);
    m_chatbot->sendMessage(userMessage);
    ui->chatbotINF->clear();
    setChatInputEnabled(false);

}

void smart_creation::onResponseReceived(const QString &response)
{
    removeLastMessageIfLoading();
    appendBotMessage(response);
    //setChatInputEnabled(true);
}

void smart_creation::onErrorOccurred(const QString &error)
{
    removeLastMessageIfLoading();
    appendErrorMessage(error);
    //setChatInputEnabled(true);
}

void smart_creation::appendUserMessage(const QString &message)
{
    ui->chatContainer->append(QString("<div style='margin: 10px 0;'><b style='color:#2196F3;'>Vous:</b> %1</div>")
                                  .arg(message.toHtmlEscaped()));
    scrollToBottom();
}

void smart_creation::appendBotMessage(const QString &message)
{
    QString formattedResponse = message.toHtmlEscaped().replace("\n", "<br>");
    ui->chatContainer->append(QString("<div style='margin: 10px 0;'><b style='color:#4CAF50;'>Chatbot:</b> %1</div>")
                                  .arg(formattedResponse));
    scrollToBottom();
}

void smart_creation::appendErrorMessage(const QString &error)
{
    ui->chatContainer->append(QString("<div style='margin: 10px 0; color:red;'><b>Erreur:</b> %1</div>")
                                  .arg(error.toHtmlEscaped()));
    scrollToBottom();
}

void smart_creation::removeLastMessageIfLoading()
{
    QString lastText = ui->chatContainer->toPlainText();
    if (lastText.endsWith("Le chatbot réfléchit...")) {
        QTextCursor cursor = ui->chatContainer->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.select(QTextCursor::BlockUnderCursor);
        cursor.removeSelectedText();
    }
}

//twilio

// Vérifier si c'est un numéro tunisien mobile
bool smart_creation::isTunisianMobileNumber(int phone_int) {
    return (phone_int >= 20000000 && phone_int <= 99999999);
}

// Convertir le INT en format international
QString smart_creation::convertIntToInternational(int phone_int) {
    return QString("+216%1").arg(phone_int, 8, 10, QChar('0'));
}

// Envoyer le message de bienvenue
void smart_creation::sendWelcomeMessage(int phone_int, const QString& name) {
    qDebug() << "=== DÉBUT ENVOI SMS TWILIO ===";

    if (!isTunisianMobileNumber(phone_int)) {
        qDebug() << "❌ Numéro non tunisien mobile ignoré:" << phone_int;
        return;
    }

    QString patient_phone = convertIntToInternational(phone_int);
    QString message = QString("Bonjour %1! Soyez le bienvenue, vous avez un rendez-vous avec nous aujourd'hui.").arg(name);

    qDebug() << "📱 De:" << twilio_from_number;
    qDebug() << "📱 À:" << patient_phone;
    qDebug() << "💬 Message:" << message;

    // Préparer l'URL Twilio
    QUrl url(QString("https://api.twilio.com/2010-04-01/Accounts/%1/Messages.json").arg(twilio_account_sid));

    // Préparer les données POST
    QUrlQuery postData;
    postData.addQueryItem("To", patient_phone);
    postData.addQueryItem("From", twilio_from_number);
    postData.addQueryItem("Body", message);

    // Préparer la requête
    QNetworkRequest request(url);

    // ⚠️ CHANGEMENT ICI : Utiliser AUTH TOKEN au lieu de API Key
    // Authentification Basic avec Account SID et Auth Token
    QString auth = QString("%1:%2").arg(twilio_account_sid).arg(twilio_auth_token);
    QByteArray authData = auth.toUtf8().toBase64();
    request.setRawHeader("Authorization", "Basic " + authData);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    qDebug() << "🔗 URL:" << url.toString();

    // Envoyer la requête
    networkManager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());

    qDebug() << "📤 Requête Twilio envoyée";
    qDebug() << "=== FIN ENVOI SMS TWILIO ===";
}

// Gestion de la réponse Twilio
void smart_creation::onTwilioReplyFinished(QNetworkReply *reply) {
    qDebug() << "=== RÉPONSE TWILIO ===";

    if (reply->error() == QNetworkReply::NoError) {
        QString response = QString(reply->readAll());
        qDebug() << "✅ SUCCÈS - SMS Twilio envoyé!";

        // Afficher un message à l'utilisateur
        QMessageBox::information(this, "SMS envoyé", "Message de bienvenue envoyé avec succès!");

    } else {
        qDebug() << "❌ ERREUR Twilio:" << reply->errorString();
        QString errorResponse = QString(reply->readAll());
        qDebug() << "📄 Réponse d'erreur:" << errorResponse;

        QMessageBox::warning(this, "Erreur SMS",
                             "Erreur lors de l'envoi du SMS: " + reply->errorString());
    }
    qDebug() << "=== FIN RÉPONSE TWILIO ===";
    reply->deleteLater();
}


// ============= CRUD MATERIEL =============
void smart_creation::refreshTable()
{

    ui->tableView_materiel->setModel(Mat.afficher());
}


void smart_creation::on_pushButton_ajouter_clicked()
{

    QString idText = ui->lineEdit_ID->text();
    QString nom = ui->lineEdit_Nom->text();
    bool isIdValid;
    int id = idText.toInt(&isIdValid);
    QRegularExpression rx("^[A-Za-z\\s\\-]+$");
    bool isNomValid = rx.match(nom).hasMatch();

    if (idText.isEmpty() || !isIdValid || id <= 0)
    {
        QMessageBox::critical(nullptr, QObject::tr("Erreur de Saisie"),
                              QObject::tr("Veuillez saisir un ID Matériel valide (nombre entier positif)."),
                              QMessageBox::Ok);
        return;
    }

    if (nom.isEmpty() || !isNomValid)
    {
        QMessageBox::critical(nullptr, QObject::tr("Erreur de Saisie"),
                              QObject::tr("Veuillez saisir un Nom valide (lettres, espaces et tirets uniquement)."),
                              QMessageBox::Ok);
        return;
    }

    QString type = ui->comboBox_Type->currentText();
    QString statut = ui->comboBox_Statut->currentText();
    int qte_dispo = ui->spinBox_Quantite->value();
    QDate date_achat = ui->dateEdit_Achat->date();
    double prix = ui->doubleSpinBox_Prix->value();

    Materiel M(id, nom, type, statut, qte_dispo, date_achat, prix);

    bool test = M.ajouter();

    if(test)
    {
        refreshTable();
        ui->lineEdit_ID->clear();
        ui->lineEdit_Nom->clear();
        ui->spinBox_Quantite->setValue(0);
        ui->doubleSpinBox_Prix->setValue(0.00);
        ui->comboBox_Type->setCurrentIndex(0);
        ui->comboBox_Statut->setCurrentIndex(0);

        QMessageBox::information(nullptr, QObject::tr("OK"), QObject::tr("Ajout effectué.\n"), QMessageBox::Ok);
    }
    else
    {
        QMessageBox::critical(nullptr, QObject::tr("Not OK"), QObject::tr("Ajout non effectué. (ID existant, clé primaire violée ou autre erreur DB)"), QMessageBox::Ok);
    }
}
void smart_creation::on_pushButton_supprimer_clicked()
{
    QString idText = ui->lineEdit_Supprimer->text();

    if (idText.isEmpty())
    {
        QMessageBox::warning(nullptr, QObject::tr("Attention"), QObject::tr("Veuillez saisir l'ID du matériel à supprimer."));
        return;
    }

    int id = idText.toInt();
    bool test = Mat.supprimer(id);

    if(test)
    {
        refreshTable();
        QMessageBox::information(nullptr, QObject::tr("OK"), QObject::tr("Suppression effectuée."), QMessageBox::Ok);
        ui->lineEdit_Supprimer->clear();
    }
    else
    {
        QMessageBox::critical(nullptr, QObject::tr("Not OK"), QObject::tr("Suppression non effectuée. (ID inconnu ou erreur base de données)"), QMessageBox::Ok);
    }
}

void smart_creation::on_pushButton_modifier_clicked()
{
    QString idText = ui->lineEdit_ID->text();
    QString nom = ui->lineEdit_Nom->text();

    if (idText.isEmpty() || nom.isEmpty()) {
        QMessageBox::warning(this, QObject::tr("Saisie Manquante"), QObject::tr("Veuillez sélectionner un matériel et remplir les champs."));
        return;
    }

    int id = idText.toInt();
    QString type = ui->comboBox_Type->currentText();
    QString statut = ui->comboBox_Statut->currentText();
    int qte_dispo = ui->spinBox_Quantite->value();
    QDate date_achat = ui->dateEdit_Achat->date();
    double prix = ui->doubleSpinBox_Prix->value();

    Materiel M(id, nom, type, statut, qte_dispo, date_achat, prix);

    bool test = M.modifier();

    if(test)
    {
        refreshTable();
        ui->lineEdit_ID->clear();
        ui->lineEdit_Nom->clear();
        ui->spinBox_Quantite->setValue(0);
        ui->doubleSpinBox_Prix->setValue(0.00);
        ui->comboBox_Type->setCurrentIndex(0);
        ui->comboBox_Statut->setCurrentIndex(0);

        QMessageBox::information(nullptr, QObject::tr("OK"), QObject::tr("Modification effectuée."), QMessageBox::Ok);
    }
    else
    {
        QMessageBox::critical(nullptr, QObject::tr("Not OK"), QObject::tr("Modification non effectuée.\nVérifiez l'ID ou l'erreur de base de données."), QMessageBox::Ok);
    }
}

void smart_creation::on_tableView_materiel_activated(const QModelIndex &index)
{
    QAbstractItemModel *model = ui->tableView_materiel->model();
    if (!model) return;

    int row = index.row();

    ui->lineEdit_ID->setText(model->data(model->index(row, 0)).toString());
    ui->lineEdit_Nom->setText(model->data(model->index(row, 1)).toString());
    ui->comboBox_Type->setCurrentText(model->data(model->index(row, 2)).toString());
    ui->comboBox_Statut->setCurrentText(model->data(model->index(row, 3)).toString());
    ui->spinBox_Quantite->setValue(model->data(model->index(row, 4)).toInt());
    ui->dateEdit_Achat->setDate(model->data(model->index(row, 5)).toDate());
    ui->doubleSpinBox_Prix->setValue(model->data(model->index(row, 6)).toDouble());
    ui->lineEdit_Supprimer->setText(model->data(model->index(row, 0)).toString());
}




void smart_creation::on_pushButton_exporter_clicked()
{

    QString fileName = QFileDialog::getSaveFileName(this, "Exporter PDF", "", "*.pdf");
    if (fileName.isEmpty()) {
        return;
    }
    bool test = Mat.exporterPDF(ui->tableView_materiel, fileName);

    if (test) {
        QMessageBox::information(this, QObject::tr("Exportation réussie"),
                                 QObject::tr("Le fichier PDF a été exporté avec succès."),
                                 QMessageBox::Ok);
    } else {
        QMessageBox::critical(this, QObject::tr("Erreur d'Exportation"),
                              QObject::tr("L'exportation du PDF a échoué."),
                              QMessageBox::Ok);
    }
}

void smart_creation::on_pushButton_recherche_clicked()
{
    QString idRecherche = ui->lineEdit_Supprimer->text();

    if (idRecherche.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Veuillez entrer l'ID du matériel à rechercher dans le champ de suppression.");
        return;
    }

    QSqlQueryModel* filteredModel = Mat.rechercher(idRecherche);

    if (filteredModel)
    {
        if (filteredModel->rowCount() > 0)
        {

            ui->tableView_materiel->setModel(filteredModel);
            QMessageBox::information(this, "Recherche Matériel", "Matériel trouvé avec l'ID : " + idRecherche );
        }
        else
        {
            delete filteredModel;
            refreshTable();
            QMessageBox::warning(this, "Erreur", "Aucun matériel trouvé avec cet ID.");
        }
    }
    else
    {
        QMessageBox::critical(this, "Erreur", "Erreur lors de l'exécution de la recherche dans la base de données.");
    }
}
void smart_creation::on_pushButton_ref_liste_clicked()
{
    refreshTable();
    ui->lineEdit_Supprimer->clear();

    QMessageBox::information(this, "Affichage", "Affichage de la liste complète de matériel.");
}

void smart_creation::on_comboBox_Tri_currentIndexChanged(int index)
{
    qDebug() << "Slot de Tri appelé. Index sélectionné :" << index;

    QString colonne = "TYPE_MAT";
    QString ordre;
    QString ordre_texte;

    if (index == 0) {
        ordre = "ASC";
        ordre_texte = "croissant";
    } else if (index == 1) {
        ordre = "DESC";
        ordre_texte = "décroissant";
    } else {
        refreshTable();
        return;
    }
    QSqlQueryModel* sortedModel = Mat.trier(colonne, ordre);

    if (sortedModel && sortedModel->rowCount() > 0)
    {
        ui->tableView_materiel->setModel(sortedModel);
        QMessageBox::information(this, "Tri", "Tri effectué par type en ordre " + ordre_texte);
    }
    else
    {
        QMessageBox::critical(this, "Erreur de Tri", "Le tri n'a retourné aucun résultat ou a échoué. Vérifiez la console.");
        refreshTable();
        if(sortedModel) delete sortedModel;
    }
}
void smart_creation::on_pushButton_statistique_clicked()
{
    QMap<QString, int> stats = Mat.compter_statut();

    if (stats.isEmpty()) {
        QMessageBox::warning(this, "Statistiques", "Aucune donnée de matériel trouvée pour les statistiques.");
        return;
    }
    QPieSeries *series = new QPieSeries();
    int total = 0;

    for (auto it = stats.begin(); it != stats.end(); ++it) {
        total += it.value();
    }

    for (auto it = stats.begin(); it != stats.end(); ++it) {
        QString label = QString("%1 (%2 - %3%)")
        .arg(it.key())
            .arg(it.value())
            .arg((double)it.value() * 100 / total, 0, 'f', 1);

        QPieSlice *slice = series->append(label, it.value());
        if (it.key().toLower() == "disponible") {
            slice->setExploded(true);
            slice->setLabelVisible(true);
        }
    }
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Répartition du Matériel par Statut de Disponibilité");
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    ui->tabWidget_3->setCurrentIndex(1);
    QWidget *stat_page = ui->tabWidget_3->widget(1);
    QLayout *oldLayout = stat_page->layout();
    if (oldLayout) {
        QLayoutItem *item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete oldLayout;
    }
    QVBoxLayout *layout = new QVBoxLayout(stat_page);
    layout->addWidget(chartView);
    stat_page->setLayout(layout);
}


void smart_creation::on_lineEdit_id_reservation_editingFinished()
{
    QString id_reserve = ui->lineEdit_id_reservation->text();
    ui->lineEdit_suggestion->clear();

    if (id_reserve.isEmpty()) {
        return;
    }
    QSqlQuery check_status;
    check_status.prepare("SELECT STATUT_MAT FROM MATERIEL WHERE ID_MAT = :id");
    check_status.bindValue(":id", id_reserve);

    if (!check_status.exec() || !check_status.next()) {
        ui->lineEdit_suggestion->setText("Erreur: ID de matériel non valide ou inexistant.");
        ui->lineEdit_suggestion->setStyleSheet("color: red;");
        return;
    }

    QString statut = check_status.value(0).toString();

    if (statut.compare("disponible", Qt::CaseInsensitive) == 0) {
        ui->lineEdit_suggestion->setText("Matériel disponible. Vous pouvez réserver cet ID.");
        ui->lineEdit_suggestion->setStyleSheet("color: green;");
        return;
    }
    bool estReserve = statut.compare("Reservé", Qt::CaseInsensitive) == 0;
    bool estEnPanne = statut.compare("en panne", Qt::CaseInsensitive) == 0;

    QString message_indisponible_base = estEnPanne ? "Matériel en panne" : (estReserve ? "Déjà réservé" : "Matériel indisponible");

    if (!estReserve && !estEnPanne) {
        ui->lineEdit_suggestion->setText(QString("%1. Statut matériel inconnu ou invalide.").arg(message_indisponible_base));
        ui->lineEdit_suggestion->setStyleSheet("color: red;");
        return;
    }

    QSqlQuery suggestion = Mat.suggerer_materiel_remplacement(id_reserve);
    if (suggestion.next()) {
        QString id_suggere = suggestion.value("ID_MAT").toString();
        QString nom_suggere = suggestion.value("NOM_MAT").toString();
        QString type_suggere = suggestion.value("TYPE_MAT").toString();

        QString message = QString("%1. Remplacement suggéré : ID %2, %3 (%4).")
                              .arg(message_indisponible_base).arg(id_suggere).arg(nom_suggere).arg(type_suggere);

        ui->lineEdit_suggestion->setText(message);
        ui->lineEdit_suggestion->setStyleSheet("color: orange;");

    } else {
        ui->lineEdit_suggestion->setText(QString("%1. Aucun remplacement disponible du même type.").arg(message_indisponible_base));
        ui->lineEdit_suggestion->setStyleSheet("color: red;");
    }
}
void smart_creation::mettreAJourMaintenance()
{
    QString notifications = Mat.genererNotificationsMaintenance();
    ui->label_notifications->setText(notifications);

    if (notifications.contains("Alerte")) {
        ui->label_notifications->setStyleSheet("color: red; font-weight: bold;");
    } else {
        ui->label_notifications->setStyleSheet("color: green;");
    }
}
const QString GROQ_API_KEY = "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";

void smart_creation::recommandationMaterielIA()
{

    Materiel mat;
    QString liste_materiel = mat.getListeMaterielPourIA();

    if (liste_materiel.contains("Erreur")) {
        QMessageBox::critical(this, "Erreur BD", liste_materiel);
        return;
    }

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl("https://api.groq.com/openai/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + GROQ_API_KEY).toUtf8());
    QString prompt =
        "Voici la liste complète du matériel actuellement disponible dans mon inventaire (ID, Nom, Type, Statut, Quantité disponible):\n\n"
        "--- INVENTAIRE ---\n"
        + liste_materiel +
        "\n------------------\n\n"
        "En tant qu'assistant expert en gestion d'inventaire, veuillez analyser cet inventaire et donner trois (3) recommandations claires pour la gestion ou l'acquisition de matériel. "
        "Concentrez-vous sur les points suivants :"
        "\n1. Quel matériel est en rupture de stock (`Qte: 0`) ou doit être commandé en priorité ?"
        "\n2. Quel matériel a un stock faible et pourrait nécessiter une alerte ?"
        "\n3. Quel matériel a un statut 'Réservé' mais pas de remplaçant clair ?"
        "\nRetournez les 3 recommandations sous forme de liste numérotée et en français, de manière concise et professionnelle.";

    QJsonObject body;
    body["model"] = "llama-3.1-8b-instant";
    QJsonArray messages;
    QJsonObject msg;
    msg["role"] = "user";
    msg["content"] = prompt;
    messages.append(msg);
    body["messages"] = messages;

    QNetworkReply *reply = manager->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [=]() {
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonObject obj = doc.object();

        if (!obj.contains("choices")) {
            QMessageBox::critical(this, "Erreur IA", "Erreur Groq: " + response);
            reply->deleteLater();
            return;
        }

        QString iaText = obj["choices"].toArray()[0].toObject()["message"].toObject()["content"].toString();

        QMessageBox::information(this, "Assistant Gestion IA", iaText);
        reply->deleteLater();
    });
}










void smart_creation::on_supprimer_pers_clicked()
{
    int id = ui->rech_supp->text().toInt();

    if (GP.supprimer(id)) {
        QMessageBox::information(this, "Succès", "Personnel supprimé !");
        ui->tableView_2->setModel(GP.afficher());
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la suppression !");
    }
}

void smart_creation::on_afficher_pers_clicked()
{
    ui->tableView_2->setModel(GP.afficher());
}

void smart_creation::on_ajouter_pers_clicked()
{
    int id = ui->id_pers->text().toInt();
    QString nom = ui->nom_pers->text();
    QString prenom = ui->prenom_pers->text();
    QString email = ui->mail_pers->text();
    QString mdp = ui->mdp_pers->text();
    float salaire = ui->salaire_pers->text().toFloat();
    QString telStr = ui->tel_pers->text();
    QDate date_naiss = ui->date_naissance_pers->date();
    QString domaine = ui->domaine_pers->text();
    QString genre = ui->radio_homme->isChecked() ? "Homme" : "Femme";
    QString nom_pere = ui->nom_pere_pers->text();
    QString color_pref = ui->color_pref_pers->text();
    QString animal_pref = ui->animal_pref_pers->text();


    if (id <= 0) {
        QMessageBox::warning(this, "Erreur", "ID invalide !");
        return;
    }

    if (nom.isEmpty() || prenom.isEmpty() || email.isEmpty() || mdp.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Tous les champs obligatoires doivent être remplis.");
        return;
    }

    if (!email.contains("@")) {
        QMessageBox::warning(this, "Erreur", "L'adresse email doit contenir '@'.");
        return;
    }

    if (telStr.length() != 8) {
        QMessageBox::warning(this, "Erreur", "Le numéro doit contenir exactement 8 chiffres.");
        return;
    }

    if (!telStr.contains(QRegularExpression("^[0-9]+$"))) {
        QMessageBox::warning(this, "Erreur", "Le numéro doit contenir uniquement des chiffres.");
        return;
    }

    int tel = telStr.toInt();
    if (GP.existeID(id)) {
        QMessageBox::warning(this, "Erreur", "Cet ID existe déjà !");
        return;
    }

    if (GP.existeEmail(email)) {
        QMessageBox::warning(this, "Erreur", "Cet email est déjà utilisé !");
        return;
    }

    if (GP.existeNum(tel)) {
        QMessageBox::warning(this, "Erreur", "Ce numéro est déjà utilisé !");
        return;
    }

    GP = gpersonnel(id, email, mdp, nom, prenom,
                    salaire, tel, date_naiss, genre, domaine,
                    nom_pere, color_pref, animal_pref);


    if (GP.ajouter()) {
        QMessageBox::information(this, "Succès", "Personnel ajouté !");
        ui->tableView_2->setModel(GP.afficher());
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de l'ajout !");
    }
}

void smart_creation::on_modifier_pers_clicked()
{
    int id = ui->id_pers->text().toInt();
    QString nom = ui->nom_pers->text();
    QString prenom = ui->prenom_pers->text();
    QString email = ui->mail_pers->text();
    QString mdp = ui->mdp_pers->text();
    float salaire = ui->salaire_pers->text().toFloat();
    QDate date_naiss = ui->date_naissance_pers->date();
    QString genre = ui->radio_homme->isChecked() ? "Homme" : "Femme";
    QString domaine = ui->domaine_pers->text();
    QString telStr = ui->tel_pers->text();
    QString nom_pere = ui->nom_pere_pers->text();
    QString color_pref = ui->color_pref_pers->text();
    QString animal_pref = ui->animal_pref_pers->text();

    if (id <= 0) { QMessageBox::warning(this, "Erreur", "ID invalide !"); return; }
    if (!email.contains("@")) { QMessageBox::warning(this, "Erreur", "L'adresse email doit contenir '@'."); return; }
    if (telStr.length() != 8 || !telStr.contains(QRegularExpression("^[0-9]+$"))) {
        QMessageBox::warning(this, "Erreur", "Numéro invalide !"); return;
    }
    int tel = telStr.toInt();

    if (GP.modifier(id, email, mdp, nom, prenom, salaire, tel, date_naiss, genre, domaine,
                    nom_pere, color_pref, animal_pref)) {
        QMessageBox::information(this, "Succès", "Personnel modifié !");
        ui->tableView_2->setModel(GP.afficher());
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la modification !");
    }
}





void smart_creation::on_tableView_2_clicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    int row = index.row();
    QAbstractItemModel *model = ui->tableView_2->model();

    if (row < 0 || row >= model->rowCount())
        return;

    ui->id_pers->setText(model->index(row, 0).data().toString());
    ui->mail_pers->setText(model->index(row, 1).data().toString());
    ui->mdp_pers->setText(model->index(row, 2).data().toString());
    ui->nom_pers->setText(model->index(row, 3).data().toString());
    ui->prenom_pers->setText(model->index(row, 4).data().toString());
    ui->salaire_pers->setText(model->index(row, 5).data().toString());
    ui->tel_pers->setText(model->index(row, 6).data().toString());
    ui->date_naissance_pers->setDate(model->index(row, 7).data().toDate());
    QString genre = model->index(row, 8).data().toString();
    ui->domaine_pers->setText(model->index(row, 9).data().toString());
    ui->radio_homme->setChecked(genre == "Homme");
    ui->radio_femme->setChecked(genre == "Femme");

    ui->nom_pere_pers->setText(model->index(row, 10).data().toString());
    ui->color_pref_pers->setText(model->index(row, 11).data().toString());
    ui->animal_pref_pers->setText(model->index(row, 12).data().toString());

}



void smart_creation::on_annuler_pers_clicked()
{
    ui->id_pers->clear();
    ui->nom_pers->clear();
    ui->prenom_pers->clear();
    ui->mail_pers->clear();
    ui->mdp_pers->clear();
    ui->salaire_pers->clear();
    ui->tel_pers->clear();
    ui->domaine_pers->clear();

    ui->date_naissance_pers->setDate(QDate::currentDate());

    ui->radio_homme->setChecked(false);
    ui->radio_femme->setChecked(false);
    ui->nom_pere_pers->clear();
    ui->color_pref_pers->clear();
    ui->animal_pref_pers->clear();


    QMessageBox::information(this, "Annulation", "Champs vidés !");
}
void smart_creation::on_btn_rech_clicked()
{
    QString texte = ui->rech_pers->text().trimmed();


    if (texte.isEmpty()) {
        ui->tableView_2->setModel(GP.afficher());
        return;
    }


    ui->tableView_2->setModel(GP.rechercher(texte));
}

void smart_creation::on_export_pers_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Exporter en PDF",
        QDir::homePath() + "/export_pers.pdf",
        "PDF (*.pdf)"
        );

    if (filePath.isEmpty())
        return;

    QAbstractItemModel *model = ui->tableView_2->model();
    if (!model)
        return;

    QString html;
    html += "<h2 style='text-align:center;'>Liste du Personnel</h2><br>";
    html += "<table border='1' cellspacing='0' cellpadding='4' width='100%'>";


    html += "<tr>";
    for (int col = 0; col < model->columnCount(); col++) {
        html += "<th style='background:#EEE;'>" + model->headerData(col, Qt::Horizontal).toString() + "</th>";
    }
    html += "</tr>";


    for (int row = 0; row < model->rowCount(); row++) {
        html += "<tr>";
        for (int col = 0; col < model->columnCount(); col++) {
            html += "<td>" + model->index(row, col).data().toString() + "</td>";
        }
        html += "</tr>";
    }
    html += "</table>";


    QTextDocument document;
    document.setHtml(html);

    QPrinter printer(QPrinter::PrinterMode::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));

    document.print(&printer);

    QMessageBox::information(this, "PDF Exporté", "Le fichier PDF a été enregistré :\n" + filePath);
}
void smart_creation::on_tri_pers_clicked()
{
    QString critere = ui->tri_combo->currentText();

    if (critere.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un critère de tri.");
        return;
    }


    QSqlQueryModel *modelTri = GP.trier(critere);


    ui->tableView_2->setModel(modelTri);
}



void smart_creation::afficherStatistiquesDomaine()
{
    QSqlQueryModel *stat = GP.statistiquesDomaine();

    QPieSeries *series = new QPieSeries();

    for (int i = 0; i < stat->rowCount(); i++)
    {
        QString domaine = stat->index(i, 0).data().toString();
        int total = stat->index(i, 1).data().toInt();

        series->append(domaine + " (" + QString::number(total) + ")", total);
    }

    // Style
    for (auto slice : series->slices()) {
        slice->setLabelVisible(true);
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Répartition des personnels par domaine");
    chart->legend()->setAlignment(Qt::AlignBottom);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Statistiques – Domaine");
    dialog->resize(600, 450);


    QPushButton *btnQuitter = new QPushButton("Quitter");
    btnQuitter->setStyleSheet("padding: 6px; font-size: 14px;");

    connect(btnQuitter, &QPushButton::clicked, dialog, &QDialog::close);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(chartView);
    layout->addWidget(btnQuitter, 0, Qt::AlignCenter);
    dialog->setLayout(layout);

    dialog->exec();
}
void smart_creation::on_static_pers_clicked()
{
    afficherStatistiquesDomaine();
}































/*******************************/
// ============= CRUD VIDEO =============
void smart_creation::on_btnajout_clicked()
{
    if(!validerChamps())
        return;

    int id = ui->idvid->text().toInt();
    QString nom = ui->nomvid->text().trimmed();
    int id_infl = ui->idinflu->text().toInt(); // Pour la table PRODUIRE
    int vues = ui->nbvus->text().toInt();
    int likes = ui->nblikes->text().toInt();
    QDate date = ui->date->date();
    QString categorie = ui->categ->text().trimmed(); // Categorie est maintenant QString
    double cout_val = ui->cout->text().toDouble();

    if(videoTemp.existe(id))
    {
        QMessageBox::warning(this, "ID existant",
                             "Cet ID de vidéo existe déjà dans la base de données!\n"
                             "Veuillez choisir un autre ID.");
        ui->idvid->setFocus();
        ui->idvid->selectAll();
        return;
    }

    Video v(id, nom, vues, likes, date, categorie, cout_val);

    if(v.ajouter())
    {
        // Lier la vidéo à l'influenceur via la table PRODUIRE (si ID influenceur fourni)
        if(id_infl > 0)
        {
            QSqlQuery queryProduire;
            queryProduire.prepare("INSERT INTO PRODUIRE (ID_INFL, ID_VD) VALUES (:id_inf, :id_vid)");
            queryProduire.bindValue(":id_inf", id_infl);
            queryProduire.bindValue(":id_vid", id);

            if(!queryProduire.exec())
            {
                qDebug() << "Avertissement: Impossible de lier l'influenceur:" << queryProduire.lastError().text();
                QMessageBox::warning(this, "Avertissement",
                                     "Vidéo ajoutée mais impossible de la lier à l'influenceur.\n"
                                     "Vérifiez que l'ID influenceur existe.");
            }
        }

        actualiserAffichage();
        viderChamps();
        QMessageBox::information(this, "Succès", "Vidéo ajoutée avec succès!");
    }
    else
    {
        QMessageBox::critical(this, "Erreur",
                              "Erreur lors de l'ajout de la vidéo.\n"
                              "Vérifiez les contraintes de la base de données.");
    }
}

void smart_creation::on_btnmod_clicked()
{
    if(ui->idvid->text().isEmpty())
    {
        QMessageBox::warning(this, "ID manquant",
                             "Veuillez sélectionner une vidéo à modifier!\n"
                             "Cliquez sur une ligne du tableau.");
        return;
    }

    // Validation des champs
    if(ui->nomvid->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "Champ manquant",
                             "Veuillez saisir le nom de la vidéo!");
        ui->nomvid->setFocus();
        return;
    }

    if(ui->nomvid->text().trimmed().length() < 3)
    {
        QMessageBox::warning(this, "Nom trop court",
                             "Le nom de la vidéo doit contenir au moins 3 caractères!");
        ui->nomvid->setFocus();
        return;
    }

    int id = ui->idvid->text().toInt();
    if(!videoTemp.existe(id))
    {
        QMessageBox::warning(this, "Vidéo introuvable",
                             "Cette vidéo n'existe pas dans la base de données!");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation",
                                  "Voulez-vous vraiment modifier cette vidéo?",
                                  QMessageBox::Yes | QMessageBox::No);
    if(reply == QMessageBox::No)
        return;

    QString nom = ui->nomvid->text().trimmed();
    int id_infl = ui->idinflu->text().isEmpty() ? 0 : ui->idinflu->text().toInt(); // ID influenceur optionnel
    int vues = ui->nbvus->text().toInt();
    int likes = ui->nblikes->text().toInt();
    QDate date = ui->date->date();
    QString categorie = ui->categ->text().trimmed(); // Categorie est maintenant QString
    double cout_val = ui->cout->text().toDouble();

    Video v(id, nom, vues, likes, date, categorie, cout_val);

    if(v.modifier())
    {
        // Mettre à jour la relation avec l'influenceur dans PRODUIRE
        // D'abord supprimer l'ancienne relation
        QSqlQuery queryDelete;
        queryDelete.prepare("DELETE FROM PRODUIRE WHERE ID_VD = :id_vid");
        queryDelete.bindValue(":id_vid", id);
        queryDelete.exec();

        // Puis créer la nouvelle relation si ID influenceur fourni
        if(id_infl > 0)
        {
            QSqlQuery queryProduire;
            queryProduire.prepare("INSERT INTO PRODUIRE (ID_INFL, ID_VD) VALUES (:id_inf, :id_vid)");
            queryProduire.bindValue(":id_inf", id_infl);
            queryProduire.bindValue(":id_vid", id);

            if(!queryProduire.exec())
            {
                qDebug() << "Avertissement: Impossible de lier l'influenceur:" << queryProduire.lastError().text();
            }
        }

        actualiserAffichage();
        viderChamps();
        QMessageBox::information(this, "Succès", "Vidéo modifiée avec succès!");
    }
    else
    {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la modification.");
    }
}

void smart_creation::on_btnsupp_clicked()
{
    QList<QTableWidgetItem*> selection = ui->tab1->selectedItems();
    if(selection.isEmpty())
    {
        QMessageBox::warning(this, "Aucune sélection",
                             "Veuillez sélectionner une vidéo à supprimer dans le tableau!");
        return;
    }
    int row = ui->tab1->currentRow();
    if(row < 0 || !ui->tab1->item(row, 0))
    {
        QMessageBox::warning(this, "Erreur",
                             "Impossible de récupérer les informations de la vidéo.");
        return;
    }
    int id = ui->tab1->item(row, 0)->text().toInt();
    QString nom = ui->tab1->item(row, 1)->text();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation de suppression",
                                  QString("Voulez-vous vraiment supprimer la vidéo:\n\n"
                                          "ID: %1\n"
                                          "Nom: \"%2\"\n\n"
                                          "Cette action est irréversible!")
                                      .arg(id).arg(nom),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No);

    if(reply == QMessageBox::Yes)
    {
        if(videoTemp.supprimer(id))
        {
            actualiserAffichage();
            viderChamps();
            QMessageBox::information(this, "Succès", "Vidéo supprimée avec succès!");
        }
        else
        {
            QMessageBox::critical(this, "Erreur", "Erreur lors de la suppression.");
        }
    }
}

void smart_creation::on_btnannuler_clicked()
{
    viderChamps();
}

void smart_creation::on_btnrecherche_clicked()
{
    // Recherche multi-critères intelligente
    QString critere = ui->lineedit_recherche->text().trimmed();

    if(critere.isEmpty())
    {
        QMessageBox::information(this, "Recherche Multi-Critères",
                                 "Veuillez saisir un critère de recherche.\n\n"
                                 "Vous pouvez rechercher par :\n"
                                 "• ID Vidéo (ex: 1)\n"
                                 "• Nom de vidéo (ex: Samsung)\n"
                                 "• Catégorie (ex: Technologie)\n"
                                 "• Date (ex: 22/11/25)\n"
                                 "• Coût (ex: 12000)\n"
                                 "• ID Influenceur (ex: 1)");
        actualiserAffichage(); // Afficher toutes les vidéos
        return;
    }

    rechercher(critere);

    // Afficher un message informatif avec les résultats
    int nbResultats = ui->tab1->rowCount();
    if(nbResultats == 0)
    {
        QMessageBox::information(this, "Aucun résultat",
                                 QString("Aucune vidéo trouvée pour le critère : \"%1\"").arg(critere));
    }
    else
    {
        qDebug() << "Recherche effectuée : " << nbResultats << " résultat(s) pour \"" << critere << "\"";
    }
}

void smart_creation::on_lineedit_recherche_textChanged(const QString &arg1)
{
    // Recherche en temps réel lors de la saisie (optionnel)
    if(arg1.trimmed().isEmpty())
    {
        actualiserAffichage();
    }
    else
    {
        rechercher(arg1.trimmed());
    }
}

void smart_creation::on_btntri_clicked()
{
    // Créer une boîte de dialogue pour choisir le critère de tri
    QStringList criteres;
    criteres << "Catégorie" << "Coût (Croissant)" << "Coût (Décroissant)"
             << "Vues (Croissant)" << "Vues (Décroissant)" << "Date";

    bool ok;
    QString critere = QInputDialog::getItem(this, "Tri des vidéos",
                                            "Choisir un critère de tri:",
                                            criteres, 0, false, &ok);

    if(!ok || critere.isEmpty())
        return;

    QSqlQuery query;
    QString orderBy;

    if(critere == "Catégorie")
        orderBy = "V.CATEGORIE_VID ASC";
    else if(critere == "Coût (Croissant)")
        orderBy = "V.COUT_VID ASC";
    else if(critere == "Coût (Décroissant)")
        orderBy = "V.COUT_VID DESC";
    else if(critere == "Vues (Croissant)")
        orderBy = "V.NB_VUE_VID ASC";
    else if(critere == "Vues (Décroissant)")
        orderBy = "V.NB_VUE_VID DESC";
    else if(critere == "Date")
        orderBy = "V.DATE_VD DESC";

    query.prepare("SELECT V.ID_VID, V.NOM_VID, V.CATEGORIE_VID, V.NB_VUE_VID, V.NB_LIKE_VID, "
                  "TO_CHAR(V.DATE_VD, 'DD/MM/YYYY'), V.COUT_VID, P.ID_INFL "
                  "FROM VIDEO V "
                  "LEFT JOIN PRODUIRE P ON V.ID_VID = P.ID_VD "
                  "ORDER BY " + orderBy);

    if(!query.exec())
    {
        QMessageBox::critical(this, "Erreur", "Erreur lors du tri: " + query.lastError().text());
        return;
    }

    ui->tab1->clearContents();
    ui->tab1->setRowCount(0);

    int row = 0;
    while(query.next())
    {
        ui->tab1->insertRow(row);
        for(int col = 0; col < 8; col++)
        {
            QTableWidgetItem *item = new QTableWidgetItem(query.value(col).toString());
            item->setTextAlignment(Qt::AlignCenter);
            ui->tab1->setItem(row, col, item);
        }
        row++;
    }

    ui->tab1->resizeColumnsToContents();
    QMessageBox::information(this, "Tri", QString("Vidéos triées par %1").arg(critere));
}

void smart_creation::on_pushButton_73_clicked()
{
    // Export PDF des vidéos
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF",
                                                    "videos_export.pdf",
                                                    "PDF Files (*.pdf)");

    if(fileName.isEmpty())
        return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize::A4);
    printer.setPageOrientation(QPageLayout::Landscape);

    QTextDocument doc;
    QString html = "<html><head><style>"
                   "table { border-collapse: collapse; width: 100%; }"
                   "th, td { border: 1px solid black; padding: 8px; text-align: center; }"
                   "th { background-color: #4CAF50; color: white; }"
                   "h1 { text-align: center; color: #4CAF50; }"
                   "</style></head><body>";

    html += "<h1>Liste des Vidéos</h1>";
    html += "<table>";
    html += "<tr><th>ID</th><th>Nom</th><th>Catégorie</th><th>Vues</th><th>Likes</th><th>Date</th><th>Coût</th><th>ID Influenceur</th></tr>";

    int rowCount = ui->tab1->rowCount();
    for(int row = 0; row < rowCount; row++)
    {
        html += "<tr>";
        for(int col = 0; col < 8; col++)
        {
            QTableWidgetItem* item = ui->tab1->item(row, col);
            QString text = item ? item->text() : "";
            html += "<td>" + text + "</td>";
        }
        html += "</tr>";
    }

    html += "</table>";
    html += "<br><p style='text-align: center;'>Total: " + QString::number(rowCount) + " vidéo(s)</p>";
    html += "</body></html>";

    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, "Export PDF", "Export réussi!\nFichier: " + fileName);
}

void smart_creation::on_pushButton_75_clicked()
{
    // Statistiques des vidéos par catégorie
    QSqlQuery query;
    query.prepare("SELECT V.CATEGORIE_VID, COUNT(*) as NOMBRE, "
                  "SUM(TO_NUMBER(V.NB_VUE_VID)) as TOTAL_VUES, "
                  "SUM(TO_NUMBER(V.NB_LIKE_VID)) as TOTAL_LIKES, "
                  "SUM(V.COUT_VID) as TOTAL_COUT "
                  "FROM VIDEO V "
                  "GROUP BY V.CATEGORIE_VID "
                  "ORDER BY NOMBRE DESC");

    if(!query.exec())
    {
        QMessageBox::critical(this, "Erreur", "Erreur lors du calcul des statistiques:\n" + query.lastError().text());
        return;
    }

    // Créer le graphique circulaire
    QPieSeries *series = new QPieSeries();
    series->setHoleSize(0.35); // Donut chart

    QString statsText = "📊 STATISTIQUES DES VIDÉOS PAR CATÉGORIE\n";
    statsText += "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    int totalVideos = 0;
    while(query.next())
    {
        QString categorie = query.value(0).toString();
        int nombre = query.value(1).toInt();
        int totalVues = query.value(2).toInt();
        int totalLikes = query.value(3).toInt();
        double totalCout = query.value(4).toDouble();

        totalVideos += nombre;

        // Ajouter au graphique
        QPieSlice *slice = series->append(categorie, nombre);
        slice->setLabelVisible(true);
        slice->setLabel(QString("%1: %2 (%3%)")
                            .arg(categorie)
                            .arg(nombre)
                            .arg(100 * slice->percentage(), 0, 'f', 1));

        // Ajouter aux statistiques texte
        statsText += QString("📹 %1:\n").arg(categorie);
        statsText += QString("   • Nombre de vidéos: %1\n").arg(nombre);
        statsText += QString("   • Total vues: %1\n").arg(totalVues);
        statsText += QString("   • Total likes: %1\n").arg(totalLikes);
        statsText += QString("   • Coût total: %1 TND\n\n").arg(totalCout, 0, 'f', 2);
    }

    statsText += "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    statsText += QString("📊 TOTAL: %1 vidéo(s)").arg(totalVideos);

    // Créer le graphique
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Répartition des vidéos par catégorie");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setAlignment(Qt::AlignRight);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Créer une nouvelle fenêtre pour afficher les statistiques
    QDialog *statsDialog = new QDialog(this);
    statsDialog->setWindowTitle("Statistiques des Vidéos");
    statsDialog->resize(900, 600);

    QVBoxLayout *layout = new QVBoxLayout(statsDialog);

    // Ajouter le graphique
    layout->addWidget(chartView);

    // Ajouter les statistiques texte
    QTextEdit *textStats = new QTextEdit(statsDialog);
    textStats->setPlainText(statsText);
    textStats->setReadOnly(true);
    textStats->setMaximumHeight(200);
    layout->addWidget(textStats);

    QPushButton *closeBtn = new QPushButton("Fermer", statsDialog);
    connect(closeBtn, &QPushButton::clicked, statsDialog, &QDialog::accept);
    layout->addWidget(closeBtn);

    statsDialog->exec();
}

void smart_creation::chargerVideoDuTableau(int row)
{
    if(row >= 0 && row < ui->tab1->rowCount())
    {
        QTableWidgetItem* item = ui->tab1->item(row, 0);
        if(!item) return;

        // Mapping des colonnes (8 colonnes)
        // 0: ID Video, 1: Nom Video, 2: Categorie, 3: Vues, 4: Likes, 5: Date, 6: Cout, 7: ID Influenceur
        ui->idvid->setText(ui->tab1->item(row, 0)->text());      // ID
        ui->nomvid->setText(ui->tab1->item(row, 1)->text());     // Nom
        ui->categ->setText(ui->tab1->item(row, 2)->text());      // Categorie
        ui->nbvus->setText(ui->tab1->item(row, 3)->text());      // Vues
        ui->nblikes->setText(ui->tab1->item(row, 4)->text());    // Likes

        QString dateStr = ui->tab1->item(row, 5)->text();        // Date
        QDate date = QDate::fromString(dateStr, "dd/MM/yyyy");
        if(date.isValid())
            ui->date->setDate(date);
        else
            ui->date->setDate(QDate::currentDate());

        ui->cout->setText(ui->tab1->item(row, 6)->text());       // Cout

        // Charger l'ID influenceur depuis la colonne 7
        if(ui->tab1->item(row, 7))
            ui->idinflu->setText(ui->tab1->item(row, 7)->text());
        else
            ui->idinflu->clear();

        qDebug() << "Vidéo chargée - ID:" << ui->idvid->text();
    }
}

void smart_creation::viderChamps()
{
    ui->idvid->clear();
    ui->nomvid->clear();
    ui->idinflu->clear();
    ui->nbvus->setText("0");
    ui->nblikes->setText("0");
    ui->date->setDate(QDate::currentDate());
    ui->categ->clear(); // Categorie est maintenant QString, pas int
    ui->cout->setText("0.0");

    ui->idvid->setFocus();
    ui->tab1->clearSelection();

    qDebug() << "Champs vidés";
}

bool smart_creation::validerChamps()
{
    if(ui->idvid->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "Champ manquant",
                             "Veuillez saisir l'ID de la vidéo!");
        ui->idvid->setFocus();
        return false;
    }
    bool ok;
    int id = ui->idvid->text().toInt(&ok);
    if(!ok || id <= 0)
    {
        QMessageBox::warning(this, "ID invalide",
                             "L'ID doit être un nombre entier positif!");
        ui->idvid->setFocus();
        ui->idvid->selectAll();
        return false;
    }
    if(ui->nomvid->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "Champ manquant",
                             "Veuillez saisir le nom de la vidéo!");
        ui->nomvid->setFocus();
        return false;
    }
    if(ui->nomvid->text().trimmed().length() < 3)
    {
        QMessageBox::warning(this, "Nom trop court",
                             "Le nom de la vidéo doit contenir au moins 3 caractères!");
        ui->nomvid->setFocus();
        ui->nomvid->selectAll();
        return false;
    }
    if(ui->idinflu->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "Champ manquant",
                             "Veuillez saisir l'ID de l'influenceur!");
        ui->idinflu->setFocus();
        return false;
    }
    int id_infl = ui->idinflu->text().toInt(&ok);
    if(!ok || id_infl <= 0)
    {
        QMessageBox::warning(this, "ID influenceur invalide",
                             "L'ID de l'influenceur doit être un nombre entier positif!");
        ui->idinflu->setFocus();
        ui->idinflu->selectAll();
        return false;
    }
    if(ui->cout->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "Champ manquant",
                             "Veuillez saisir le coût de la vidéo!");
        ui->cout->setFocus();
        return false;
    }
    double cout_val = ui->cout->text().toDouble(&ok);
    if(!ok || cout_val < 0)
    {
        QMessageBox::warning(this, "Coût invalide",
                             "Le coût doit être un nombre positif!\n"
                             "Exemple: 100.50");
        ui->cout->setFocus();
        ui->cout->selectAll();
        return false;
    }
    return true;
}

void smart_creation::actualiserAffichage()
{
    QSqlQuery query;
    // Requête avec LEFT JOIN pour inclure l'ID influenceur via PRODUIRE
    query.prepare("SELECT V.ID_VID, V.NOM_VID, V.CATEGORIE_VID, V.NB_VUE_VID, V.NB_LIKE_VID, "
                  "TO_CHAR(V.DATE_VD, 'DD/MM/YYYY'), V.COUT_VID, P.ID_INFL "
                  "FROM VIDEO V "
                  "LEFT JOIN PRODUIRE P ON V.ID_VID = P.ID_VD "
                  "ORDER BY V.ID_VID");

    if(!query.exec())
    {
        qDebug() << "Erreur requête:" << query.lastError().text();
        return;
    }

    ui->tab1->clearContents();
    ui->tab1->setRowCount(0);
    ui->tab1->setColumnCount(8); // 8 colonnes maintenant (avec ID_INF)
    QStringList headers;
    headers << "ID Video" << "Nom Video" << "Categorie" << "Vues" << "Likes" << "Date" << "Cout" << "ID Influenceur";
    ui->tab1->setHorizontalHeaderLabels(headers);

    int row = 0;
    while(query.next())
    {
        ui->tab1->insertRow(row);
        for(int col = 0; col < 8; col++)
        {
            QTableWidgetItem *item = new QTableWidgetItem(query.value(col).toString());
            item->setTextAlignment(Qt::AlignCenter);
            ui->tab1->setItem(row, col, item);
        }
        row++;
    }

    ui->tab1->resizeColumnsToContents();
    ui->tab1->horizontalHeader()->setStretchLastSection(true);
    qDebug() << "Affichage actualisé -" << row << "vidéo(s)";
}

void smart_creation::rechercher(QString critere)
{
    QSqlQuery query;
    // Recherche multi-critères intelligente : cherche dans tous les champs
    // ID vidéo, Nom vidéo, Catégorie, Date, Coût, ID influenceur
    query.prepare("SELECT V.ID_VID, V.NOM_VID, V.CATEGORIE_VID, V.NB_VUE_VID, V.NB_LIKE_VID, "
                  "TO_CHAR(V.DATE_VD, 'DD/MM/YYYY'), V.COUT_VID, P.ID_INFL "
                  "FROM VIDEO V "
                  "LEFT JOIN PRODUIRE P ON V.ID_VID = P.ID_VD "
                  "WHERE UPPER(V.NOM_VID) LIKE :critere "           // Recherche par nom
                  "OR TO_CHAR(V.ID_VID) LIKE :critere "             // Recherche par ID vidéo
                  "OR UPPER(V.CATEGORIE_VID) LIKE :critere "        // Recherche par catégorie
                  "OR TO_CHAR(V.DATE_VD, 'DD/MM/YYYY') LIKE :critere " // Recherche par date
                  "OR TO_CHAR(V.COUT_VID) LIKE :critere "           // Recherche par coût
                  "OR TO_CHAR(P.ID_INFL) LIKE :critere "            // Recherche par ID influenceur
                  "ORDER BY V.ID_VID");

    query.bindValue(":critere", "%" + critere.toUpper() + "%");

    if(!query.exec())
    {
        qDebug() << "Erreur recherche:" << query.lastError().text();
        return;
    }

    ui->tab1->clearContents();
    ui->tab1->setRowCount(0);

    int row = 0;
    while(query.next())
    {
        ui->tab1->insertRow(row);
        for(int col = 0; col < 8; col++) // 8 colonnes avec ID_INF
        {
            QTableWidgetItem *item = new QTableWidgetItem(query.value(col).toString());
            item->setTextAlignment(Qt::AlignCenter);
            ui->tab1->setItem(row, col, item);
        }
        row++;
    }

    ui->tab1->resizeColumnsToContents();
    qDebug() << "Recherche: " << row << "résultat(s) trouvé(s)";
}


// ============= ANALYSEUR DE RENTABILITÉ (ROI) =============
void smart_creation::on_btn_roi_clicked()
{
    ROIAnalyzer *analyzer = new ROIAnalyzer(this);
    analyzer->afficher();
    delete analyzer;
}

// ============= PRÉDICTEUR DE TENDANCES ML =============
void smart_creation::on_btn_predicteur_clicked()
{
    MLPredictor *predictor = new MLPredictor(this);
    predictor->afficher();
    delete predictor;
}

// ============= ARDUINO GAS SENSOR ALERT SYSTEM =============

/**
 * Configuration de la connexion Arduino
 * Recherche et se connecte au port série Arduino
 */
void smart_creation::setupArduinoConnection()
{
    qDebug() << "=== INITIALISATION CONNEXION ARDUINO ===";

    arduinoPort = new QSerialPort(this);

    // Rechercher le port Arduino (généralement COM3, COM4 sur Windows ou /dev/ttyUSB0, /dev/ttyACM0 sur Linux)
    const auto ports = QSerialPortInfo::availablePorts();

    qDebug() << "Ports série disponibles:";
    for (const QSerialPortInfo &portInfo : ports) {
        qDebug() << "  - Port:" << portInfo.portName()
                 << "Description:" << portInfo.description()
                 << "Manufacturer:" << portInfo.manufacturer();

        // Détecter Arduino (généralement contient "Arduino" dans la description)
        if (portInfo.description().contains("Arduino", Qt::CaseInsensitive) ||
            portInfo.manufacturer().contains("Arduino", Qt::CaseInsensitive) ||
            portInfo.portName().contains("USB", Qt::CaseInsensitive) ||
            portInfo.portName().contains("ACM", Qt::CaseInsensitive))
        {
            qDebug() << "✅ Arduino détecté sur:" << portInfo.portName();

            arduinoPort->setPortName(portInfo.portName());
            arduinoPort->setBaudRate(QSerialPort::Baud9600);
            arduinoPort->setDataBits(QSerialPort::Data8);
            arduinoPort->setParity(QSerialPort::NoParity);
            arduinoPort->setStopBits(QSerialPort::OneStop);
            arduinoPort->setFlowControl(QSerialPort::NoFlowControl);

            if (arduinoPort->open(QIODevice::ReadWrite)) {
                qDebug() << "✅ Port Arduino ouvert avec succès!";
                connect(arduinoPort, &QSerialPort::readyRead, this, &smart_creation::onSerialDataReceived);
                return;
            } else {
                qDebug() << "❌ Erreur d'ouverture du port:" << arduinoPort->errorString();
            }
        }
    }

    qDebug() << "⚠️ Aucun Arduino détecté. Connexion automatique désactivée.";
    qDebug() << "   Vous pouvez toujours tester avec le bouton manuel.";
}

/**
 * Gestionnaire de réception de données série depuis Arduino
 * Lit les messages envoyés par l'Arduino et déclenche les alertes
 */
void smart_creation::onSerialDataReceived()
{
    if (!arduinoPort)
        return;

    QByteArray data = arduinoPort->readAll();
    QString message = QString::fromUtf8(data).trimmed();

    qDebug() << "📨 Message Arduino reçu:" << message;

    // Vérifier si c'est une alerte de gaz
    if (message.contains("GAS_ALERT", Qt::CaseInsensitive) ||
        message.contains("GAZ_DETECTE", Qt::CaseInsensitive) ||
        message.contains("DANGER", Qt::CaseInsensitive))
    {
        qDebug() << "🚨 ALERTE GAZ DÉTECTÉE!";
        sendGasAlertToAllEmployees();
    }
}

/**
 * Fonction d'envoi de SMS (version générique)
 * @param phoneNumber - Numéro de téléphone au format international (+216...)
 * @param message - Message à envoyer
 */
void smart_creation::sendSMS(const QString &phoneNumber, const QString &message)
{
    qDebug() << "📱 Envoi SMS à:" << phoneNumber;

    // Préparer l'URL Twilio
    QUrl url(QString("https://api.twilio.com/2010-04-01/Accounts/%1/Messages.json").arg(twilio_account_sid));

    // Préparer les données POST
    QUrlQuery postData;
    postData.addQueryItem("To", phoneNumber);
    postData.addQueryItem("From", twilio_from_number);
    postData.addQueryItem("Body", message);

    // Préparer la requête
    QNetworkRequest request(url);
    QString auth = QString("%1:%2").arg(twilio_account_sid).arg(twilio_auth_token);
    QByteArray authData = auth.toUtf8().toBase64();
    request.setRawHeader("Authorization", "Basic " + authData);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    // Envoyer la requête
    networkManager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
}

/**
 * FONCTION PRINCIPALE: Envoie une alerte SMS à tous les employés
 * en cas de détection de gaz par l'Arduino
 */
void smart_creation::sendGasAlertToAllEmployees()
{
    qDebug() << "=== 🚨 ALERTE GAZ - ENVOI SMS À TOUS LES EMPLOYÉS ===";

    // Requête pour récupérer tous les numéros de téléphone des employés
    QSqlQuery query;
    query.prepare("SELECT NOM_PERSO, PRENOM_PERSO, NUM_TEL_PERSO FROM PERSONNEL");

    if (!query.exec()) {
        qDebug() << "❌ Erreur de récupération des employés:" << query.lastError().text();
        QMessageBox::critical(this, "Erreur",
                              "Impossible de récupérer la liste des employés pour l'alerte gaz!");
        return;
    }

    // Message d'alerte de gaz
    QString alertMessage = "🚨 ALERTE URGENTE! 🚨\n"
                          "Un capteur de gaz a détecté un danger dans les locaux de Smart Creation.\n"
                          "Veuillez évacuer immédiatement les lieux et contacter les secours!\n"
                          "⚠️ NE PAS RETOURNER AVANT L'AUTORISATION ⚠️";

    int totalEmployees = 0;
    int smsSent = 0;

    // Parcourir tous les employés
    while (query.next()) {
        QString nom = query.value(0).toString();
        QString prenom = query.value(1).toString();
        int phoneInt = query.value(2).toInt();

        totalEmployees++;

        qDebug() << "Employé:" << nom << prenom << "- Tél:" << phoneInt;

        // Vérifier si c'est un numéro tunisien valide
        if (isTunisianMobileNumber(phoneInt)) {
            QString phoneInternational = convertIntToInternational(phoneInt);
            sendSMS(phoneInternational, alertMessage);
            smsSent++;
            qDebug() << "  ✅ SMS envoyé à" << nom << prenom;
        } else {
            qDebug() << "  ⚠️ Numéro invalide pour" << nom << prenom;
        }
    }

    qDebug() << "=== FIN ALERTE GAZ ===";
    qDebug() << "Total employés:" << totalEmployees;
    qDebug() << "SMS envoyés:" << smsSent;

    // Afficher un message à l'utilisateur
    QMessageBox::warning(this, "🚨 Alerte Gaz Envoyée",
                        QString("Alerte de gaz détectée!\n\n"
                                "SMS d'urgence envoyés à %1 employé(s) sur %2.\n\n"
                                "⚠️ Veuillez évacuer les locaux immédiatement!")
                        .arg(smsSent).arg(totalEmployees));
}

/**
 * Bouton de test pour simuler une alerte gaz
 * (sans avoir besoin de l'Arduino connecté)
 */
void smart_creation::on_btn_test_gas_alert_clicked()
{
    qDebug() << "🧪 TEST MANUEL - Simulation d'alerte gaz";

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Test Alerte Gaz",
                                  "⚠️ ATTENTION ⚠️\n\n"
                                  "Vous êtes sur le point d'envoyer une alerte de gaz "
                                  "à TOUS les employés de la base de données.\n\n"
                                  "Voulez-vous continuer?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        sendGasAlertToAllEmployees();
    } else {
        qDebug() << "Test annulé par l'utilisateur";
    }
}

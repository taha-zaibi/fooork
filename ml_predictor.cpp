#include "ml_predictor.h"
#include <QHeaderView>

MLPredictor::MLPredictor(QWidget *parent)
    : QDialog(parent)
    , tablePred(nullptr)
    , statsLabel(nullptr)
    , btnFermer(nullptr)
{
    setWindowTitle("Prédicteur de Tendances ML");
    resize(1200, 700);
    setStyleSheet("QDialog { background-color: rgb(255, 255, 255); }");
}

MLPredictor::~MLPredictor()
{
}

void MLPredictor::afficher()
{
    // Calculer les prédictions
    QList<VideoPrediction> predictions = calculerPredictions();

    if(predictions.isEmpty())
    {
        QMessageBox::information(this, "Prédicteur", "Aucune vidéo disponible pour l'analyse prédictive.");
        return;
    }

    // Créer l'interface
    creerInterface(predictions);

    // Afficher le dialogue
    exec();
}

QList<VideoPrediction> MLPredictor::calculerPredictions()
{
    QList<VideoPrediction> videosPredictions;

    // Requête pour récupérer toutes les vidéos avec leurs métriques
    QSqlQuery query;
    query.prepare("SELECT V.ID_VID, V.NOM_VID, V.NB_VUE_VID, V.NB_LIKE_VID, V.COUT_VID, "
                  "V.CATEGORIE_VID, V.DATE_VD "
                  "FROM VIDEO V "
                  "ORDER BY V.DATE_VD DESC");

    if(!query.exec())
    {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la récupération des données:\n" + query.lastError().text());
        return videosPredictions;
    }

    // Calculer les métriques et faire les prédictions
    int totalVues = 0;
    int totalLikes = 0;
    double totalCout = 0;

    while(query.next())
    {
        VideoPrediction vPred;
        vPred.id = query.value(0).toInt();
        vPred.nom = query.value(1).toString();
        vPred.vues = query.value(2).toInt();
        vPred.likes = query.value(3).toInt();
        vPred.cout = query.value(4).toDouble();
        vPred.categorie = query.value(5).toString();
        vPred.date = query.value(6).toDate();

        totalVues += vPred.vues;
        totalLikes += vPred.likes;
        totalCout += vPred.cout;

        // Calcul du taux d'engagement
        if(vPred.cout > 0) {
            vPred.tauxEngagement = (vPred.vues + vPred.likes) / vPred.cout;
            vPred.scoreViral = (vPred.vues * vPred.likes) / vPred.cout;
        } else {
            vPred.tauxEngagement = 0;
            vPred.scoreViral = 0;
        }

        videosPredictions.append(vPred);
    }

    if(videosPredictions.isEmpty())
        return videosPredictions;

    // Calculer les moyennes pour les prédictions
    double moyenneVues = totalVues / (double)videosPredictions.size();
    double moyenneLikes = totalLikes / (double)videosPredictions.size();
    double moyenneCout = totalCout / videosPredictions.size();
    double tauxCroissanceMoyen = moyenneVues / (moyenneCout > 0 ? moyenneCout : 1);

    // Appliquer le modèle prédictif
    appliquerModelePredictif(videosPredictions, moyenneVues, moyenneLikes, tauxCroissanceMoyen);

    return videosPredictions;
}

void MLPredictor::appliquerModelePredictif(QList<VideoPrediction> &predictions,
                                           double moyenneVues,
                                           double moyenneLikes,
                                           double tauxCroissanceMoyen)
{
    for(VideoPrediction &vPred : predictions)
    {
        // Prédiction basée sur le modèle de régression simple
        // Formule: Prédiction = Valeur actuelle * (1 + facteur de croissance)
        double facteurCroissance = vPred.tauxEngagement / tauxCroissanceMoyen;

        if(facteurCroissance > 1.5) {
            // Forte croissance prédite
            vPred.vuesPredites = vPred.vues * 1.3; // +30% prédit
            vPred.likesPredits = vPred.likes * 1.35; // +35% prédit
        } else if(facteurCroissance > 1.0) {
            // Croissance modérée
            vPred.vuesPredites = vPred.vues * 1.15; // +15% prédit
            vPred.likesPredits = vPred.likes * 1.20; // +20% prédit
        } else if(facteurCroissance > 0.7) {
            // Stable
            vPred.vuesPredites = vPred.vues * 1.05; // +5% prédit
            vPred.likesPredits = vPred.likes * 1.05; // +5% prédit
        } else {
            // En déclin
            vPred.vuesPredites = vPred.vues * 0.95; // -5% prédit
            vPred.likesPredits = vPred.likes * 0.90; // -10% prédit
        }

        // Déterminer la tendance et l'alerte
        if(vPred.scoreViral > moyenneVues * moyenneLikes * 2) {
            vPred.tendance = "🔥 VIRAL";
            vPred.alerte = "VIRAL";
            vPred.couleurTendance = QColor("#e74c3c"); // Rouge vif
        } else if(vPred.tauxEngagement > tauxCroissanceMoyen * 1.3) {
            vPred.tendance = "📈 En forte hausse";
            vPred.alerte = "TENDANCE";
            vPred.couleurTendance = QColor("#27ae60"); // Vert
        } else if(vPred.tauxEngagement > tauxCroissanceMoyen * 0.9) {
            vPred.tendance = "➡️ Stable";
            vPred.alerte = "NORMAL";
            vPred.couleurTendance = QColor("#3498db"); // Bleu
        } else {
            vPred.tendance = "📉 En déclin";
            vPred.alerte = "ATTENTION";
            vPred.couleurTendance = QColor("#e67e22"); // Orange
        }
    }
}

void MLPredictor::creerInterface(const QList<VideoPrediction> &predictions)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Titre - adapté au thème global
    QLabel *titre = new QLabel("<h2 style='text-align:center; background-color: rgb(148, 179, 107); "
                               "padding: 15px; border-radius: 30px; color: black; border: 2px solid #bd22ff; font-weight: bold;'>🤖 PRÉDICTEUR DE TENDANCES ML</h2>");
    titre->setTextFormat(Qt::RichText);
    mainLayout->addWidget(titre);

    // Tableau des prédictions
    tablePred = new QTableWidget(this);
    tablePred->setColumnCount(9);
    QStringList headers;
    headers << "ID" << "Nom Vidéo" << "Vues Actuelles" << "Vues Prédites"
            << "Likes Actuels" << "Likes Prédits" << "Score Viral" << "Tendance" << "Alerte";
    tablePred->setHorizontalHeaderLabels(headers);
    tablePred->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablePred->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablePred->horizontalHeader()->setStretchLastSection(true);

    // Style du tableau - adapté au thème global
    tablePred->setStyleSheet(
        "QTableWidget { "
        "   background-color: white; "
        "   border: 2px solid #bd22ff; "
        "   border-radius: 8px; "
        "   gridline-color: rgb(203, 203, 203); "
        "} "
        "QTableWidget::item { "
        "   padding: 8px; "
        "   color: black; "
        "} "
        "QHeaderView::section { "
        "   background-color: rgb(148, 179, 107); "
        "   color: black; "
        "   font-weight: bold; "
        "   padding: 10px; "
        "   border: none; "
        "   border-right: 1px solid rgb(118, 118, 118); "
        "} "
        );

    // Remplir le tableau
    remplirTableau(tablePred, predictions);
    mainLayout->addWidget(tablePred);

    // Calculer les statistiques
    int totalVues = 0;
    int videosVirales = 0;
    int videosTendance = 0;
    int videosAttention = 0;

    for(const VideoPrediction &vPred : predictions)
    {
        totalVues += vPred.vues;
        if(vPred.alerte == "VIRAL") videosVirales++;
        else if(vPred.alerte == "TENDANCE") videosTendance++;
        else if(vPred.alerte == "ATTENTION") videosAttention++;
    }

    // Statistiques de prédiction
    QString statsHTML = genererStatistiquesHTML(predictions, totalVues, videosVirales, videosTendance, videosAttention);
    statsLabel = new QLabel(statsHTML);
    statsLabel->setTextFormat(Qt::RichText);
    statsLabel->setWordWrap(true);
    mainLayout->addWidget(statsLabel);

    // Bouton Fermer - adapté au thème global
    btnFermer = new QPushButton("✖ Fermer", this);
    btnFermer->setStyleSheet(
        "QPushButton { "
        "   background-color: rgb(148, 179, 107); "
        "   color: black; "
        "   padding: 12px 30px; "
        "   font-size: 14px; "
        "   font-weight: bold; "
        "   border: 2px solid #bd22ff; "
        "   border-radius: 8px; "
        "   min-width: 150px; "
        "} "
        "QPushButton:hover { "
        "   background-color: rgb(168, 199, 127); "
        "   border: 2px solid #bd22ff; "
        "} "
        "QPushButton:pressed { "
        "   background-color: rgb(128, 159, 87); "
        "} "
        );
    connect(btnFermer, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(btnFermer, 0, Qt::AlignCenter);
}

void MLPredictor::remplirTableau(QTableWidget *table, const QList<VideoPrediction> &predictions)
{
    table->setRowCount(predictions.size());

    for(int i = 0; i < predictions.size(); i++)
    {
        const VideoPrediction &vPred = predictions[i];

        // ID
        table->setItem(i, 0, new QTableWidgetItem(QString::number(vPred.id)));

        // Nom
        table->setItem(i, 1, new QTableWidgetItem(vPred.nom));

        // Vues actuelles
        table->setItem(i, 2, new QTableWidgetItem(QString::number(vPred.vues)));

        // Vues prédites
        QTableWidgetItem *itemVuesPred = new QTableWidgetItem(QString::number(vPred.vuesPredites));
        int diffVues = vPred.vuesPredites - vPred.vues;
        if(diffVues > 0) {
            itemVuesPred->setBackground(QColor("#d4edda"));
            itemVuesPred->setText(QString::number(vPred.vuesPredites) + " (+" + QString::number(diffVues) + ")");
        } else {
            itemVuesPred->setBackground(QColor("#f8d7da"));
            itemVuesPred->setText(QString::number(vPred.vuesPredites) + " (" + QString::number(diffVues) + ")");
        }
        table->setItem(i, 3, itemVuesPred);

        // Likes actuels
        table->setItem(i, 4, new QTableWidgetItem(QString::number(vPred.likes)));

        // Likes prédits
        QTableWidgetItem *itemLikesPred = new QTableWidgetItem(QString::number(vPred.likesPredits));
        int diffLikes = vPred.likesPredits - vPred.likes;
        if(diffLikes > 0) {
            itemLikesPred->setBackground(QColor("#d4edda"));
            itemLikesPred->setText(QString::number(vPred.likesPredits) + " (+" + QString::number(diffLikes) + ")");
        } else {
            itemLikesPred->setBackground(QColor("#f8d7da"));
            itemLikesPred->setText(QString::number(vPred.likesPredits) + " (" + QString::number(diffLikes) + ")");
        }
        table->setItem(i, 5, itemLikesPred);

        // Score viral
        QTableWidgetItem *itemScore = new QTableWidgetItem(QString::number(vPred.scoreViral, 'f', 0));
        itemScore->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 6, itemScore);

        // Tendance
        QTableWidgetItem *itemTendance = new QTableWidgetItem(vPred.tendance);
        itemTendance->setBackground(vPred.couleurTendance);
        itemTendance->setForeground(QColor("#ffffff"));
        QFont fontBold;
        fontBold.setBold(true);
        itemTendance->setFont(fontBold);
        itemTendance->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 7, itemTendance);

        // Alerte
        QTableWidgetItem *itemAlerte = new QTableWidgetItem(vPred.alerte);
        itemAlerte->setTextAlignment(Qt::AlignCenter);
        if(vPred.alerte == "VIRAL") {
            itemAlerte->setBackground(QColor("#e74c3c"));
            itemAlerte->setForeground(QColor("#ffffff"));
        } else if(vPred.alerte == "TENDANCE") {
            itemAlerte->setBackground(QColor("#f39c12"));
            itemAlerte->setForeground(QColor("#ffffff"));
        } else if(vPred.alerte == "NORMAL") {
            itemAlerte->setBackground(QColor("#95a5a6"));
            itemAlerte->setForeground(QColor("#ffffff"));
        } else {
            itemAlerte->setBackground(QColor("#e67e22"));
            itemAlerte->setForeground(QColor("#ffffff"));
        }
        itemAlerte->setFont(fontBold);
        table->setItem(i, 8, itemAlerte);
    }

    table->resizeColumnsToContents();
}

QString MLPredictor::genererStatistiquesHTML(const QList<VideoPrediction> &predictions,
                                             int totalVues,
                                             int videosVirales,
                                             int videosTendance,
                                             int videosAttention)
{
    QString statsHTML = QString(
                            "<div style='background-color: rgb(148, 179, 107); "
                            "padding: 20px; border-radius: 30px; border: 2px solid #bd22ff;'>"
                            "<h3 style='color: black; text-align:center; margin-bottom:15px; font-weight: bold;'>📊 Analyse Prédictive Globale</h3>"
                            "<div style='display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 15px;'>"

                            "<div style='background-color: #ffe6e6; padding: 15px; border-radius: 8px; border: 1px solid #bd22ff;'>"
                            "   <h4 style='color:#e74c3c; margin:5px 0;'>🔥 Vidéos Virales</h4>"
                            "   <p style='font-size: 24px; font-weight: bold; color:#c0392b; margin:10px 0;'>%1</p>"
                            "   <p style='color:#666; font-size: 12px;'>Potentiel viral élevé</p>"
                            "</div>"

                            "<div style='background-color: #fff4e6; padding: 15px; border-radius: 8px; border: 1px solid #bd22ff;'>"
                            "   <h4 style='color:#f39c12; margin:5px 0;'>📈 Tendances émergentes</h4>"
                            "   <p style='font-size: 24px; font-weight: bold; color:#e67e22; margin:10px 0;'>%2</p>"
                            "   <p style='color:#666; font-size: 12px;'>Croissance forte prédite</p>"
                            "</div>"

                            "<div style='background-color: #ffe8cc; padding: 15px; border-radius: 8px; border: 1px solid #bd22ff;'>"
                            "   <h4 style='color:#e67e22; margin:5px 0;'>⚠️ Attention requise</h4>"
                            "   <p style='font-size: 24px; font-weight: bold; color:#d35400; margin:10px 0;'>%3</p>"
                            "   <p style='color:#666; font-size: 12px;'>Déclin prédit</p>"
                            "</div>"

                            "</div>"

                            "<div style='background-color: white; padding: 15px; border-radius: 8px; margin-top: 15px; border: 1px solid #bd22ff;'>"
                            "   <h4 style='color:#bd22ff; font-weight: bold;'>🎯 Recommandations ML</h4>"
                            "   <ul style='color: black; line-height: 1.8;'>"
                            "       <li><b>Focus Marketing:</b> Investir davantage sur les vidéos VIRALES et TENDANCE</li>"
                            "       <li><b>Optimisation:</b> Analyser et améliorer les vidéos en ATTENTION</li>"
                            "       <li><b>Stratégie:</b> Reproduire le format des vidéos à fort score viral</li>"
                            "       <li><b>Prévision:</b> Vues moyennes prédites: <b>+%.0f%%</b> pour le prochain mois</li>"
                            "   </ul>"
                            "</div>"
                            "</div>"
                            ).arg(videosVirales)
                            .arg(videosTendance)
                            .arg(videosAttention)
                            .arg(((double)totalVues / predictions.size() * 1.15 - (double)totalVues / predictions.size())
                                 / ((double)totalVues / predictions.size()) * 100);

    return statsHTML;
}

#include "roi_analyzer.h"
#include <algorithm>
#include <QHeaderView>

ROIAnalyzer::ROIAnalyzer(QWidget *parent)
    : QDialog(parent)
    , tableROI(nullptr)
    , statsLabel(nullptr)
    , btnFermer(nullptr)
{
    setWindowTitle("Analyseur de Rentabilité (ROI)");
    resize(1000, 600);
    setStyleSheet("QDialog { background-color: rgb(255, 255, 255); }");
}

ROIAnalyzer::~ROIAnalyzer()
{
}

void ROIAnalyzer::afficher()
{
    // Calculer les métriques
    QList<VideoROI> videosROI = calculerMetriques();

    if(videosROI.isEmpty())
    {
        QMessageBox::information(this, "Analyseur ROI", "Aucune vidéo disponible pour l'analyse.");
        return;
    }

    // Créer l'interface
    creerInterface(videosROI);

    // Afficher le dialogue
    exec();
}

QList<VideoROI> ROIAnalyzer::calculerMetriques()
{
    QList<VideoROI> videosROI;

    // Requête pour récupérer toutes les vidéos avec leurs métriques
    QSqlQuery query;
    query.prepare("SELECT V.ID_VID, V.NOM_VID, V.NB_VUE_VID, V.NB_LIKE_VID, V.COUT_VID, V.CATEGORIE_VID "
                  "FROM VIDEO V "
                  "ORDER BY V.ID_VID");

    if(!query.exec())
    {
        QMessageBox::critical(this, "Erreur", "Erreur lors de la récupération des données:\n" + query.lastError().text());
        return videosROI;
    }

    // Calculer les métriques pour chaque vidéo
    while(query.next())
    {
        VideoROI vROI;
        vROI.id = query.value(0).toInt();
        vROI.nom = query.value(1).toString();
        vROI.vues = query.value(2).toInt();
        vROI.likes = query.value(3).toInt();
        vROI.cout = query.value(4).toDouble();
        vROI.categorie = query.value(5).toString();

        // Calcul de l'engagement (Vues + Likes)
        vROI.engagement = vROI.vues + vROI.likes;

        // Calcul du ROI: (Engagement - Coût) / Coût × 100%
        if(vROI.cout > 0)
            vROI.roi = ((vROI.engagement - vROI.cout) / vROI.cout) * 100.0;
        else
            vROI.roi = 0.0;

        // Calcul du CPV (Coût Par Vue): Coût / Nombre de vues
        if(vROI.vues > 0)
            vROI.cpv = vROI.cout / vROI.vues;
        else
            vROI.cpv = 0.0;

        // Calcul du CPL (Coût Par Like): Coût / Nombre de likes
        if(vROI.likes > 0)
            vROI.cpl = vROI.cout / vROI.likes;
        else
            vROI.cpl = 0.0;

        videosROI.append(vROI);
    }

    // Trier par ROI décroissant (les plus rentables en premier)
    std::sort(videosROI.begin(), videosROI.end(), [](const VideoROI &a, const VideoROI &b) {
        return a.roi > b.roi;
    });

    return videosROI;
}

void ROIAnalyzer::creerInterface(const QList<VideoROI> &videosROI)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Titre - adapté au thème global (vert olive et violet)
    QLabel *titre = new QLabel("<h2 style='text-align:center; background-color: rgb(148, 179, 107); "
                               "padding: 15px; border-radius: 30px; color: black; border: 2px solid #bd22ff; font-weight: bold;'>📊 ANALYSEUR DE RENTABILITÉ DES VIDÉOS</h2>");
    titre->setTextFormat(Qt::RichText);
    mainLayout->addWidget(titre);

    // Créer le tableau pour afficher les résultats
    tableROI = new QTableWidget(this);
    tableROI->setColumnCount(9);
    QStringList headers;
    headers << "ID" << "Nom Vidéo" << "Vues" << "Likes" << "Coût (TND)"
            << "Engagement" << "ROI (%)" << "CPV" << "CPL";
    tableROI->setHorizontalHeaderLabels(headers);
    tableROI->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableROI->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableROI->horizontalHeader()->setStretchLastSection(true);

    // Style du tableau - adapté au thème global
    tableROI->setStyleSheet(
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
    remplirTableau(tableROI, videosROI);
    mainLayout->addWidget(tableROI);

    // Statistiques récapitulatives
    QString statsHTML = genererStatistiquesHTML(videosROI);
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
        "}"
        );
    connect(btnFermer, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(btnFermer, 0, Qt::AlignCenter);
}

void ROIAnalyzer::remplirTableau(QTableWidget *table, const QList<VideoROI> &videosROI)
{
    table->setRowCount(videosROI.size());

    for(int i = 0; i < videosROI.size(); i++)
    {
        const VideoROI &vROI = videosROI[i];

        // ID
        QTableWidgetItem *itemID = new QTableWidgetItem(QString::number(vROI.id));
        itemID->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 0, itemID);

        // Nom
        QTableWidgetItem *itemNom = new QTableWidgetItem(vROI.nom);
        itemNom->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 1, itemNom);

        // Vues
        QTableWidgetItem *itemVues = new QTableWidgetItem(QString::number(vROI.vues));
        itemVues->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 2, itemVues);

        // Likes
        QTableWidgetItem *itemLikes = new QTableWidgetItem(QString::number(vROI.likes));
        itemLikes->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 3, itemLikes);

        // Coût
        QTableWidgetItem *itemCout = new QTableWidgetItem(QString::number(vROI.cout, 'f', 2));
        itemCout->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 4, itemCout);

        // Engagement
        QTableWidgetItem *itemEngagement = new QTableWidgetItem(QString::number(vROI.engagement));
        itemEngagement->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 5, itemEngagement);

        // ROI (avec indicateur de couleur moderne)
        QTableWidgetItem *itemROI = new QTableWidgetItem(QString::number(vROI.roi, 'f', 2) + "%");
        itemROI->setTextAlignment(Qt::AlignCenter);

        // Système de couleurs moderne par niveau de ROI
        if(vROI.roi >= 500) {
            // Excellent ROI (>= 500%) - Vert foncé
            itemROI->setBackground(QColor("#27ae60")); // Vert émeraude
            itemROI->setForeground(QColor("#ffffff")); // Texte blanc
        }
        else if(vROI.roi >= 200) {
            // Très bon ROI (200-499%) - Vert moyen
            itemROI->setBackground(QColor("#2ecc71")); // Vert clair
            itemROI->setForeground(QColor("#ffffff")); // Texte blanc
        }
        else if(vROI.roi >= 50) {
            // Bon ROI (50-199%) - Vert pastel
            itemROI->setBackground(QColor("#a8e6cf")); // Vert pastel
            itemROI->setForeground(QColor("#2c3e50")); // Texte foncé
        }
        else if(vROI.roi > 0) {
            // ROI positif faible (0-49%) - Bleu
            itemROI->setBackground(QColor("#74b9ff")); // Bleu clair
            itemROI->setForeground(QColor("#2c3e50")); // Texte foncé
        }
        else if(vROI.roi > -50) {
            // ROI légèrement négatif (-49 à 0%) - Orange
            itemROI->setBackground(QColor("#fdcb6e")); // Orange
            itemROI->setForeground(QColor("#2c3e50")); // Texte foncé
        }
        else {
            // ROI très négatif (< -50%) - Rouge
            itemROI->setBackground(QColor("#e74c3c")); // Rouge
            itemROI->setForeground(QColor("#ffffff")); // Texte blanc
        }

        // Ajouter une police en gras pour le ROI
        QFont fontROI = itemROI->font();
        fontROI.setBold(true);
        itemROI->setFont(fontROI);

        table->setItem(i, 6, itemROI);

        // CPV
        QTableWidgetItem *itemCPV = new QTableWidgetItem(QString::number(vROI.cpv, 'f', 4));
        itemCPV->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 7, itemCPV);

        // CPL
        QTableWidgetItem *itemCPL = new QTableWidgetItem(QString::number(vROI.cpl, 'f', 4));
        itemCPL->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 8, itemCPL);
    }

    table->resizeColumnsToContents();
}

QString ROIAnalyzer::genererStatistiquesHTML(const QList<VideoROI> &videosROI)
{
    if(videosROI.isEmpty())
        return QString();

    double roiMoyen = 0.0;
    double cpvMoyen = 0.0;
    double cplMoyen = 0.0;
    int videosRentables = 0;

    for(const VideoROI &vROI : videosROI)
    {
        roiMoyen += vROI.roi;
        cpvMoyen += vROI.cpv;
        cplMoyen += vROI.cpl;
        if(vROI.roi > 0)
            videosRentables++;
    }

    roiMoyen /= videosROI.size();
    cpvMoyen /= videosROI.size();
    cplMoyen /= videosROI.size();

    QString statsText = QString(
                            "<div style='background-color: rgb(148, 179, 107); "
                            "padding: 20px; border-radius: 30px; border: 2px solid #bd22ff;'>"
                            "<h3 style='color: black; margin:10px 0; text-align:center; font-size: 18px; font-weight: bold;'>📈 Statistiques Globales</h3>"
                            "<div style='display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin: 15px 0;'>"
                            "<div style='background-color: white; padding: 10px; border-radius: 8px; border: 1px solid #bd22ff;'>"
                            "   <p style='margin: 5px 0;'><b style='color:#bd22ff;'>📊 Total vidéos:</b> <span style='font-size: 16px; color: black;'>%1</span></p>"
                            "</div>"
                            "<div style='background-color: white; padding: 10px; border-radius: 8px; border: 1px solid #bd22ff;'>"
                            "   <p style='margin: 5px 0;'><b style='color:#27ae60;'>✅ Rentables:</b> <span style='font-size: 16px; color:#27ae60; font-weight: bold;'>%2</span></p>"
                            "</div>"
                            "<div style='background-color: white; padding: 10px; border-radius: 8px; border: 1px solid #bd22ff;'>"
                            "   <p style='margin: 5px 0;'><b style='color:#e74c3c;'>❌ Non rentables:</b> <span style='font-size: 16px; color:#e74c3c; font-weight: bold;'>%3</span></p>"
                            "</div>"
                            "<div style='background-color: white; padding: 10px; border-radius: 8px; border: 1px solid #bd22ff;'>"
                            "   <p style='margin: 5px 0;'><b style='color:#bd22ff;'>📈 ROI moyen:</b> <span style='font-size: 16px; color: black; font-weight: bold;'>%4%%</span></p>"
                            "</div>"
                            "</div>"
                            "<div style='background-color: white; padding: 15px; border-radius: 8px; margin-top: 10px; border: 1px solid #bd22ff;'>"
                            "   <p style='margin: 8px 0;'><b style='color:#bd22ff;'>💰 CPV moyen:</b> <span style='color: black;'>%5 TND</span></p>"
                            "   <p style='margin: 8px 0;'><b style='color:#bd22ff;'>❤️ CPL moyen:</b> <span style='color: black;'>%6 TND</span></p>"
                            "</div>"
                            "<hr style='border: none; border-top: 2px solid #bd22ff; margin: 15px 0;'>"
                            "<div style='background-color: #d4edda; padding: 12px; border-radius: 8px; margin: 10px 0; border-left: 4px solid #27ae60;'>"
                            "   <p style='margin: 5px 0;'><b>🏆 Plus rentable:</b> <span style='color:#27ae60; font-weight: bold;'>%7</span> <br>"
                            "   <span style='color:#666; font-size: 14px;'>ROI: <b style='color:#27ae60;'>%8%%</b></span></p>"
                            "</div>"
                            "<div style='background-color: #f8d7da; padding: 12px; border-radius: 8px; margin: 10px 0; border-left: 4px solid #e74c3c;'>"
                            "   <p style='margin: 5px 0;'><b>📉 Moins rentable:</b> <span style='color:#e74c3c; font-weight: bold;'>%9</span> <br>"
                            "   <span style='color:#666; font-size: 14px;'>ROI: <b style='color:#e74c3c;'>%10%%</b></span></p>"
                            "</div>"
                            "</div>"
                            ).arg(videosROI.size())
                            .arg(videosRentables)
                            .arg(videosROI.size() - videosRentables)
                            .arg(roiMoyen, 0, 'f', 2)
                            .arg(cpvMoyen, 0, 'f', 4)
                            .arg(cplMoyen, 0, 'f', 4)
                            .arg(videosROI.first().nom)
                            .arg(videosROI.first().roi, 0, 'f', 2)
                            .arg(videosROI.last().nom)
                            .arg(videosROI.last().roi, 0, 'f', 2);

    return statsText;
}
